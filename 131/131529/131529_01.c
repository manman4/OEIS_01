/*
 * A131529 -- exact low-memory search for permutations with distinct signed
 * adjacent differences.
 *
 * A search state consists of the set V of values already used, the last
 * value x, and the set D of signed differences already used.
 * Differences -(n-1)..-1 and 1..n-1 occupy the two halves of a uint64_t.
 *
 * Prefix tasks partition all permutations (length five for n=17..20,
 * length four otherwise, and shorter when n<4),
 * and are independent parallel tasks.  Complementing every value,
 * y -> n-1-y in the internal zero-based representation, reverses all signed
 * differences.  For n>=2 this is a fixed-point-free involution.  Exactly the
 * lexicographically smaller prefix in each complementary pair is searched,
 * and the resulting sum is multiplied by two.  No memo table is populated,
 * so memory is bounded by the small task array and explicitly sized worker
 * stacks rather than by the number of search states.
 *
 * Counts use unsigned __int128.  Since a(n)<=n! and 32!<2^128, arithmetic is
 * rigorously exact for the supported range n<=32.  For n<=10 a separate
 * lexicographic enumeration of all n! permutations checks the definition
 * directly.  Published OEIS terms through n=19 are post-computation checks.
 *
 * Completed terms are flushed and fsynced to b131529_01_part.txt beside the
 * executable.  On complete success it atomically replaces b131529_01.txt.
 * An advisory lock prevents two copies from writing these files concurrently.
 * Per-task results are atomically checkpointed once per minute and reused on
 * restart.  Progress is written to stderr approximately once per minute;
 * sequence rows go to stdout.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       131529_01.c -o 131529_01
 *
 * Usage:
 *   A131529_MEMORY_MIB=512 A131529_THREADS=8 ./131529_01 N [FROM]
 *
 * N is in 1..32.  FROM is at most 20; verified terms below FROM are copied
 * so stdout and the b-file remain complete from offset 1.
 */

#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <sys/sysctl.h>
#endif

#if !defined(__SIZEOF_INT128__)
#error "131529_01.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 32
#define VERIFIED_MAX_N 19
#define FIRST_UNVERIFIED_N 20
#define DIRECT_CHECK_MAX_N 10
#define DEFAULT_MEMORY_MIB UINT64_C(512)
#define MIN_MEMORY_MIB UINT64_C(64)
#define MAX_MEMORY_MIB UINT64_C(65536)
#define PROGRESS_INTERVAL_SECONDS 60.0
#define WORKER_STACK_KIB ((size_t)256)

static const char *const verified_terms[VERIFIED_MAX_N + 1] = {
    NULL,
    "1", "2", "4", "12", "44", "176", "788", "3936",
    "23264", "152112", "1104876", "8725320", "74715908",
    "687915040", "6782261964", "71294227456", "796138700016",
    "9409401651840", "117378774461812"
};

typedef struct {
    int n;
    uint32_t full_vertices;
    uint64_t difference_bit[MAX_N][MAX_N];
} Search;

typedef struct {
    uint32_t used_vertices;
    uint64_t used_differences;
    uint8_t last;
} PrefixTask;

typedef struct {
    uint64_t low;
    uint64_t high;
    uint8_t done;
    uint8_t reserved[7];
} TaskResult;

typedef struct {
    char magic[8];
    uint32_t version;
    uint32_t n;
    uint32_t prefix_length;
    uint32_t reserved;
    uint64_t task_count;
    uint64_t task_hash;
} CheckpointHeader;

typedef struct {
    Search prototype;
    PrefixTask *tasks;
    TaskResult *task_results;
    size_t task_count;
    size_t next_task;
    size_t completed_tasks;
    U128 representative_total;
    int prefix_length;
    uint64_t task_hash;
    const char *checkpoint_path;
    bool finished;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    double started;
    double last_progress;
} ParallelSearch;

typedef struct {
    ParallelSearch *parallel;
    int prefix_length;
    int prefix[MAX_N];
} TaskBuilder;

static _Noreturn void die(const char *message);
static void *checked_malloc(size_t count, size_t size);

static uint64_t hash_u64(uint64_t hash, uint64_t value)
{
    for (int byte = 0; byte < 8; ++byte) {
        hash ^= value & UINT64_C(255);
        hash *= UINT64_C(1099511628211);
        value >>= 8;
    }
    return hash;
}

static void save_checkpoint(const ParallelSearch *parallel)
{
    CheckpointHeader header = {{0}, 1, (uint32_t)parallel->prototype.n,
                               (uint32_t)parallel->prefix_length, 0,
                               (uint64_t)parallel->task_count,
                               parallel->task_hash};
    memcpy(header.magic, "A131529", 7);
    size_t path_length = strlen(parallel->checkpoint_path);
    if (path_length > SIZE_MAX - 6) {
        die("checkpoint path length overflow");
    }
    char *temporary = checked_malloc(path_length + 6, 1);
    memcpy(temporary, parallel->checkpoint_path, path_length);
    memcpy(temporary + path_length, ".part", 6);
    FILE *file = fopen(temporary, "wb");
    if (file == NULL ||
        fwrite(&header, sizeof(header), 1, file) != 1 ||
        fwrite(parallel->task_results, sizeof(*parallel->task_results),
               parallel->task_count, file) != parallel->task_count ||
        fflush(file) != 0 || fsync(fileno(file)) != 0 ||
        fclose(file) != 0 ||
        rename(temporary, parallel->checkpoint_path) != 0) {
        fprintf(stderr, "error: could not save checkpoint %s: %s\n",
                parallel->checkpoint_path, strerror(errno));
        free(temporary);
        exit(EXIT_FAILURE);
    }
    free(temporary);
}

static void load_checkpoint(ParallelSearch *parallel)
{
    FILE *file = fopen(parallel->checkpoint_path, "rb");
    if (file == NULL) {
        if (errno == ENOENT) {
            return;
        }
        fprintf(stderr, "error: could not open checkpoint %s: %s\n",
                parallel->checkpoint_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    CheckpointHeader header;
    if (fread(&header, sizeof(header), 1, file) != 1 ||
        memcmp(header.magic, "A131529", 7) != 0 ||
        header.version != 1 || header.n != (uint32_t)parallel->prototype.n ||
        header.prefix_length != (uint32_t)parallel->prefix_length ||
        header.task_count != (uint64_t)parallel->task_count ||
        header.task_hash != parallel->task_hash ||
        fread(parallel->task_results, sizeof(*parallel->task_results),
              parallel->task_count, file) != parallel->task_count ||
        fgetc(file) != EOF || ferror(file)) {
        fclose(file);
        die("checkpoint is truncated or incompatible");
    }
    if (fclose(file) != 0) {
        die("could not close checkpoint");
    }
    for (size_t i = 0; i < parallel->task_count; ++i) {
        const TaskResult *result = &parallel->task_results[i];
        if (result->done > 1) {
            die("checkpoint contains an invalid completion flag");
        }
        if (!result->done) {
            continue;
        }
        U128 value = ((U128)result->high << 64) | result->low;
        U128 maximum = ~(U128)0;
        if (parallel->representative_total > maximum - value) {
            die("checkpoint total overflowed unsigned __int128");
        }
        parallel->representative_total += value;
        ++parallel->completed_tasks;
    }
    fprintf(stderr, "A131529 n=%d resumed tasks=%zu/%zu from %s\n",
            parallel->prototype.n, parallel->completed_tasks,
            parallel->task_count, parallel->checkpoint_path);
}

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static double monotonic_seconds(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        die("clock_gettime failed");
    }
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static void *checked_malloc(size_t count, size_t size)
{
    if (size != 0 && count > SIZE_MAX / size) {
        die("allocation size overflow");
    }
    void *memory = malloc(count * size);
    if (memory == NULL && count != 0) {
        die("memory allocation failed");
    }
    return memory;
}

static int parse_int_range(const char *text, int minimum, int maximum,
                           const char *name)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < minimum || value > maximum) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                name, minimum, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static uint64_t parse_memory_limit(void)
{
    const char *text = getenv("A131529_MEMORY_MIB");
    uint64_t mib = DEFAULT_MEMORY_MIB;
    if (text != NULL && *text != '\0') {
        char *end = NULL;
        errno = 0;
        unsigned long long value = strtoull(text, &end, 10);
        if (errno != 0 || end == text || *end != '\0' ||
            value < MIN_MEMORY_MIB || value > MAX_MEMORY_MIB) {
            fprintf(stderr,
                    "error: A131529_MEMORY_MIB must be in %" PRIu64
                    "..%" PRIu64 ": %s\n",
                    MIN_MEMORY_MIB, MAX_MEMORY_MIB, text);
            exit(EXIT_FAILURE);
        }
        mib = (uint64_t)value;
    }
    return mib * UINT64_C(1024) * UINT64_C(1024);
}

static char *path_beside_executable(const char *argv0,
                                    const char *filename)
{
    char executable[PATH_MAX];
    char resolved[PATH_MAX];
    bool found = false;
#ifdef __APPLE__
    uint32_t size = (uint32_t)sizeof(executable);
    if (_NSGetExecutablePath(executable, &size) == 0) {
        found = true;
    }
#elif defined(__linux__)
    ssize_t length = readlink("/proc/self/exe", executable,
                              sizeof(executable) - 1);
    if (length >= 0) {
        executable[length] = '\0';
        found = true;
    }
#endif
    if (!found) {
        size_t length = strlen(argv0);
        if (length >= sizeof(executable)) {
            die("executable path is too long");
        }
        memcpy(executable, argv0, length + 1);
    }
    const char *base = realpath(executable, resolved);
    if (base == NULL) {
        base = executable;
    }
    const char *slash = strrchr(base, '/');
    size_t directory_length = slash == NULL ? 1 : (size_t)(slash - base);
    const char *directory = slash == NULL ? "." : base;
    size_t filename_length = strlen(filename);
    if (directory_length > SIZE_MAX - filename_length - 2) {
        die("output path length overflow");
    }
    char *path = checked_malloc(directory_length + filename_length + 2, 1);
    memcpy(path, directory, directory_length);
    path[directory_length] = '/';
    memcpy(path + directory_length + 1, filename, filename_length + 1);
    return path;
}

static int acquire_output_lock(const char *lock_path)
{
    int descriptor = open(lock_path, O_RDWR | O_CREAT, 0600);
    if (descriptor < 0) {
        fprintf(stderr, "error: could not open %s: %s\n",
                lock_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(descriptor, F_SETLK, &lock) != 0) {
        fprintf(stderr,
                "error: another A131529 writer is active (%s)\n",
                lock_path);
        close(descriptor);
        exit(EXIT_FAILURE);
    }
    return descriptor;
}

static U128 search_completions(Search *search, uint32_t used_vertices,
                               int last, uint64_t used_differences,
                               int remaining)
{
    if (remaining == 0) {
        return 1;
    }
    U128 total = 0;
    uint32_t candidates = search->full_vertices & ~used_vertices;
    while (candidates != 0) {
        int next = __builtin_ctz(candidates);
        uint32_t next_bit = UINT32_C(1) << next;
        candidates &= candidates - 1;
        uint64_t difference = search->difference_bit[last][next];
        if ((used_differences & difference) != 0) {
            continue;
        }
        U128 add = search_completions(search,
                                      used_vertices | next_bit, next,
                                      used_differences | difference,
                                      remaining - 1);
        U128 maximum = ~(U128)0;
        if (total > maximum - add) {
            die("exact count overflowed unsigned __int128");
        }
        total += add;
    }
    return total;
}

/* For n<=20 every subtree count is at most 20!, which fits in uint64_t.
 * Keeping the hot recursion in 64 bits avoids double-word arithmetic. */
static uint64_t search_completions_u64(const Search *search,
                                       uint32_t used_vertices, int last,
                                       uint64_t used_differences,
                                       int remaining)
{
    if (remaining == 0) {
        return 1;
    }
    uint64_t total = 0;
    uint32_t candidates = search->full_vertices & ~used_vertices;
    while (candidates != 0) {
        int next = __builtin_ctz(candidates);
        uint32_t next_bit = UINT32_C(1) << next;
        candidates &= candidates - 1;
        uint64_t difference = search->difference_bit[last][next];
        if ((used_differences & difference) != 0) {
            continue;
        }
        uint64_t add = search_completions_u64(
            search, used_vertices | next_bit, next,
            used_differences | difference, remaining - 1);
        total += add;
    }
    return total;
}

static int parse_thread_count(void)
{
    long detected = 1;
#ifdef __APPLE__
    int logical_cpus = 1;
    size_t logical_cpus_size = sizeof(logical_cpus);
    if (sysctlbyname("hw.logicalcpu", &logical_cpus,
                     &logical_cpus_size, NULL, 0) == 0 &&
        logical_cpus > 0) {
        detected = logical_cpus;
    }
#elif defined(_SC_NPROCESSORS_ONLN)
    detected = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    int count = detected > 0 && detected <= 64 ? (int)detected : 1;
    const char *text = getenv("A131529_THREADS");
    if (text != NULL && *text != '\0') {
        count = parse_int_range(text, 1, 64, "A131529_THREADS");
    }
    return count;
}

static bool prefix_is_complement_representative(const int *prefix,
                                                int length, int n)
{
    for (int i = 0; i < length; ++i) {
        int complement = n - 1 - prefix[i];
        if (prefix[i] != complement) {
            return prefix[i] < complement;
        }
    }
    die("distinct prefix was fixed by value complementation");
}

static void generate_prefix_tasks(TaskBuilder *builder, int depth,
                                  uint32_t used_vertices,
                                  uint64_t used_differences, int last)
{
    Search *search = &builder->parallel->prototype;
    if (depth == builder->prefix_length) {
        if (!prefix_is_complement_representative(
                builder->prefix, builder->prefix_length, search->n)) {
            return;
        }
        PrefixTask *task =
            &builder->parallel->tasks[builder->parallel->task_count++];
        task->used_vertices = used_vertices;
        task->used_differences = used_differences;
        task->last = (uint8_t)last;
        return;
    }
    uint32_t candidates = search->full_vertices & ~used_vertices;
    while (candidates != 0) {
        int next = __builtin_ctz(candidates);
        uint32_t next_bit = UINT32_C(1) << next;
        candidates &= candidates - 1;
        uint64_t next_differences = used_differences;
        if (depth != 0) {
            uint64_t difference = search->difference_bit[last][next];
            if ((used_differences & difference) != 0) {
                continue;
            }
            next_differences |= difference;
        }
        builder->prefix[depth] = next;
        generate_prefix_tasks(builder, depth + 1,
                              used_vertices | next_bit,
                              next_differences, next);
    }
}

static void *parallel_worker(void *argument)
{
    ParallelSearch *parallel = argument;
    Search search = parallel->prototype;
    for (;;) {
        if (pthread_mutex_lock(&parallel->mutex) != 0) {
            die("pthread_mutex_lock failed");
        }
        while (parallel->next_task < parallel->task_count &&
               parallel->task_results[parallel->next_task].done) {
            ++parallel->next_task;
        }
        if (parallel->next_task == parallel->task_count) {
            if (pthread_mutex_unlock(&parallel->mutex) != 0) {
                die("pthread_mutex_unlock failed");
            }
            break;
        }
        size_t task_index = parallel->next_task++;
        PrefixTask task = parallel->tasks[task_index];
        if (pthread_mutex_unlock(&parallel->mutex) != 0) {
            die("pthread_mutex_unlock failed");
        }

        int remaining = search.n -
                        __builtin_popcount(task.used_vertices);
        U128 value = search.n <= 20
                         ? (U128)search_completions_u64(
                               &search, task.used_vertices, task.last,
                               task.used_differences, remaining)
                         : search_completions(&search, task.used_vertices,
                                              task.last,
                                              task.used_differences,
                                              remaining);

        if (pthread_mutex_lock(&parallel->mutex) != 0) {
            die("pthread_mutex_lock failed");
        }
        U128 maximum = ~(U128)0;
        if (parallel->representative_total > maximum - value) {
            die("exact count overflowed unsigned __int128");
        }
        parallel->representative_total += value;
        parallel->task_results[task_index].low = (uint64_t)value;
        parallel->task_results[task_index].high = (uint64_t)(value >> 64);
        parallel->task_results[task_index].done = 1;
        ++parallel->completed_tasks;
        if (pthread_mutex_unlock(&parallel->mutex) != 0) {
            die("pthread_mutex_unlock failed");
        }
    }
    return NULL;
}

static void *progress_monitor(void *argument)
{
    ParallelSearch *parallel = argument;
    if (pthread_mutex_lock(&parallel->mutex) != 0) {
        die("pthread_mutex_lock failed");
    }
    for (;;) {
        if (parallel->finished) {
            break;
        }
        struct timespec deadline;
        if (clock_gettime(CLOCK_REALTIME, &deadline) != 0) {
            die("clock_gettime failed");
        }
        ++deadline.tv_sec;
        int wait_result = pthread_cond_timedwait(
            &parallel->condition, &parallel->mutex, &deadline);
        if (wait_result != 0 && wait_result != ETIMEDOUT) {
            die("pthread_cond_timedwait failed");
        }
        double now = monotonic_seconds();
        if (now - parallel->last_progress >= PROGRESS_INTERVAL_SECONDS) {
            save_checkpoint(parallel);
            fprintf(stderr,
                    "A131529 n=%d progress tasks=%zu/%zu (%.1f%%) "
                    "elapsed=%.0fs checkpoint=saved\n",
                    parallel->prototype.n, parallel->completed_tasks,
                    parallel->task_count,
                    100.0 * (double)parallel->completed_tasks /
                        (double)parallel->task_count,
                    now - parallel->started);
            fflush(stderr);
            parallel->last_progress = now;
        }
    }
    if (pthread_mutex_unlock(&parallel->mutex) != 0) {
        die("pthread_mutex_unlock failed");
    }
    return NULL;
}

static U128 calculate_term(int n, uint64_t memory_limit,
                           const char *checkpoint_path,
                           size_t *tasks_out,
                           uint64_t *peak_out, double *seconds_out,
                           int *threads_out)
{
    if (n == 1) {
        *tasks_out = 1;
        *peak_out = 0;
        *seconds_out = 0;
        *threads_out = 1;
        return 1;
    }

    ParallelSearch parallel;
    memset(&parallel, 0, sizeof(parallel));
    Search *prototype = &parallel.prototype;
    prototype->n = n;
    prototype->full_vertices = n == 32
                                   ? UINT32_MAX
                                   : (UINT32_C(1) << n) - 1;
    for (int from = 0; from < n; ++from) {
        for (int to = 0; to < n; ++to) {
            if (from == to) {
                continue;
            }
            int difference = to - from;
            int magnitude = difference < 0 ? -difference : difference;
            int index = difference < 0
                            ? magnitude - 1
                            : (n - 1) + magnitude - 1;
            prototype->difference_bit[from][to] =
                UINT64_C(1) << index;
        }
    }

    int prefix_length = n < 4 ? n :
                        (n >= 17 && n <= 20 ? 5 : 4);
    parallel.prefix_length = prefix_length;
    parallel.checkpoint_path = checkpoint_path;
    size_t maximum_tasks = 1;
    for (int i = 0; i < prefix_length; ++i) {
        if (maximum_tasks > SIZE_MAX / (size_t)(n - i)) {
            die("prefix-task count overflow");
        }
        maximum_tasks *= (size_t)(n - i);
    }
    parallel.tasks = checked_malloc(maximum_tasks,
                                    sizeof(*parallel.tasks));
    TaskBuilder builder = {&parallel, prefix_length, {0}};
    generate_prefix_tasks(&builder, 0, 0, 0, 0);
    if (parallel.task_count == 0) {
        die("prefix task generation produced no tasks");
    }
    parallel.task_hash = UINT64_C(1469598103934665603);
    for (size_t i = 0; i < parallel.task_count; ++i) {
        parallel.task_hash = hash_u64(
            parallel.task_hash, parallel.tasks[i].used_vertices);
        parallel.task_hash = hash_u64(
            parallel.task_hash, parallel.tasks[i].used_differences);
        parallel.task_hash = hash_u64(parallel.task_hash,
                                      parallel.tasks[i].last);
    }
    parallel.task_results = calloc(parallel.task_count,
                                   sizeof(*parallel.task_results));
    if (parallel.task_results == NULL) {
        die("memory allocation failed");
    }
    load_checkpoint(&parallel);

    int thread_count = parse_thread_count();
    if ((size_t)thread_count > parallel.task_count) {
        thread_count = (int)parallel.task_count;
    }
    pthread_t *threads = checked_malloc((size_t)thread_count,
                                        sizeof(*threads));
    size_t worker_stack_size = WORKER_STACK_KIB * (size_t)1024;
#ifdef PTHREAD_STACK_MIN
    if (worker_stack_size < (size_t)PTHREAD_STACK_MIN) {
        worker_stack_size = (size_t)PTHREAD_STACK_MIN;
    }
#endif
    uint64_t controlled_memory =
        (uint64_t)maximum_tasks * (uint64_t)sizeof(PrefixTask) +
        (uint64_t)parallel.task_count *
            (uint64_t)sizeof(TaskResult) +
        (uint64_t)(size_t)(thread_count + 1) *
            (uint64_t)sizeof(pthread_t) +
        (uint64_t)(size_t)(thread_count + 1) *
            (uint64_t)worker_stack_size;
    if (controlled_memory > memory_limit) {
        die("A131529_MEMORY_MIB is too small for tasks and worker stacks");
    }
    if (pthread_mutex_init(&parallel.mutex, NULL) != 0) {
        die("pthread_mutex_init failed");
    }
    if (pthread_cond_init(&parallel.condition, NULL) != 0) {
        die("pthread_cond_init failed");
    }
    pthread_attr_t attributes;
    if (pthread_attr_init(&attributes) != 0) {
        die("pthread_attr_init failed");
    }
    if (pthread_attr_setstacksize(&attributes, worker_stack_size) != 0) {
        die("pthread_attr_setstacksize failed");
    }
    parallel.started = monotonic_seconds();
    parallel.last_progress = parallel.started;
    pthread_t monitor_thread;
    if (pthread_create(&monitor_thread, &attributes, progress_monitor,
                       &parallel) != 0) {
        die("pthread_create failed for progress monitor");
    }
    for (int i = 0; i < thread_count; ++i) {
        if (pthread_create(&threads[i], &attributes, parallel_worker,
                           &parallel) != 0) {
            die("pthread_create failed");
        }
    }
    if (pthread_attr_destroy(&attributes) != 0) {
        die("pthread_attr_destroy failed");
    }
    for (int i = 0; i < thread_count; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            die("pthread_join failed");
        }
    }
    if (pthread_mutex_lock(&parallel.mutex) != 0) {
        die("pthread_mutex_lock failed");
    }
    parallel.finished = true;
    if (pthread_cond_signal(&parallel.condition) != 0) {
        die("pthread_cond_signal failed");
    }
    if (pthread_mutex_unlock(&parallel.mutex) != 0) {
        die("pthread_mutex_unlock failed");
    }
    if (pthread_join(monitor_thread, NULL) != 0) {
        die("pthread_join failed for progress monitor");
    }
    double seconds = monotonic_seconds() - parallel.started;
    if (pthread_cond_destroy(&parallel.condition) != 0) {
        die("pthread_cond_destroy failed");
    }
    if (pthread_mutex_destroy(&parallel.mutex) != 0) {
        die("pthread_mutex_destroy failed");
    }
    free(threads);
    save_checkpoint(&parallel);
    free(parallel.task_results);
    free(parallel.tasks);

    U128 maximum = ~(U128)0;
    if (parallel.representative_total > maximum / 2) {
        die("exact count overflowed unsigned __int128");
    }
    *tasks_out = parallel.task_count;
    *peak_out = controlled_memory;
    *seconds_out = seconds;
    *threads_out = thread_count;
    U128 total = 2 * parallel.representative_total;
    return total;
}

static bool next_permutation(int *values, int n)
{
    int pivot = n - 2;
    while (pivot >= 0 && values[pivot] >= values[pivot + 1]) {
        --pivot;
    }
    if (pivot < 0) {
        return false;
    }
    int successor = n - 1;
    while (values[successor] <= values[pivot]) {
        --successor;
    }
    int temporary = values[pivot];
    values[pivot] = values[successor];
    values[successor] = temporary;
    for (int left = pivot + 1, right = n - 1; left < right;
         ++left, --right) {
        temporary = values[left];
        values[left] = values[right];
        values[right] = temporary;
    }
    return true;
}

static uint64_t direct_count(int n)
{
    int values[DIRECT_CHECK_MAX_N];
    for (int i = 0; i < n; ++i) {
        values[i] = i;
    }
    uint64_t count = 0;
    do {
        uint64_t differences = 0;
        bool valid = true;
        for (int i = 1; i < n; ++i) {
            int difference = values[i] - values[i - 1];
            int magnitude = difference < 0 ? -difference : difference;
            int index = difference < 0
                            ? magnitude - 1
                            : (n - 1) + magnitude - 1;
            uint64_t bit = UINT64_C(1) << index;
            if ((differences & bit) != 0) {
                valid = false;
                break;
            }
            differences |= bit;
        }
        if (valid) {
            ++count;
        }
    } while (next_permutation(values, n));
    return count;
}

static U128 parse_u128(const char *text)
{
    U128 value = 0;
    U128 maximum = ~(U128)0;
    if (text == NULL || *text == '\0') {
        die("invalid empty decimal constant");
    }
    for (const char *p = text; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            die("invalid decimal constant");
        }
        unsigned digit = (unsigned)(*p - '0');
        if (value > (maximum - digit) / 10) {
            die("decimal constant exceeds unsigned __int128");
        }
        value = value * 10 + digit;
    }
    return value;
}

static void u128_to_decimal(U128 value, char buffer[40])
{
    char reversed[40];
    size_t length = 0;
    do {
        reversed[length++] = (char)('0' + value % 10);
        value /= 10;
    } while (value != 0);
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = reversed[length - 1 - i];
    }
    buffer[length] = '\0';
}

static void verify_u128_range(void)
{
    U128 factorial = 1;
    U128 maximum = ~(U128)0;
    for (unsigned n = 2; n <= MAX_N; ++n) {
        if (factorial > maximum / n) {
            die("MAX_N factorial does not fit unsigned __int128");
        }
        factorial *= n;
    }
}

static void write_term(FILE *bfile, const char *part_path, int n,
                       U128 value)
{
    char decimal[40];
    u128_to_decimal(value, decimal);
    if (fprintf(bfile, "%d %s\n", n, decimal) < 0) {
        die("could not write a b-file term");
    }
    if (fflush(bfile) != 0 || fsync(fileno(bfile)) != 0) {
        fprintf(stderr, "error: could not flush %s: %s\n",
                part_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (printf("%d %s\n", n, decimal) < 0 || fflush(stdout) != 0) {
        die("could not write stdout");
    }
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s N [FROM]\n"
            "  write exact A131529 terms 1..N, with N <= %d\n"
            "  1 <= FROM <= min(N,%d)\n",
            program, MAX_N, FIRST_UNVERIFIED_N);
}

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    int maximum_n = parse_int_range(argv[1], 1, MAX_N, "N");
    int maximum_from = maximum_n < FIRST_UNVERIFIED_N
                           ? maximum_n
                           : FIRST_UNVERIFIED_N;
    int first_n = argc == 3
                      ? parse_int_range(argv[2], 1, maximum_from, "FROM")
                      : 1;
    uint64_t memory_limit = parse_memory_limit();
    verify_u128_range();

    char *part_path =
        path_beside_executable(argv[0], "b131529_01_part.txt");
    char *final_path =
        path_beside_executable(argv[0], "b131529_01.txt");
    char *lock_path =
        path_beside_executable(argv[0], "b131529_01.lock");
    int lock_descriptor = acquire_output_lock(lock_path);
    FILE *bfile = fopen(part_path, "w");
    if (bfile == NULL) {
        fprintf(stderr, "error: could not open %s: %s\n",
                part_path, strerror(errno));
        close(lock_descriptor);
        free(lock_path);
        free(final_path);
        free(part_path);
        return EXIT_FAILURE;
    }

    for (int n = 1; n < first_n; ++n) {
        U128 value = parse_u128(verified_terms[n]);
        write_term(bfile, part_path, n, value);
        fprintf(stderr, "A131529 n=%d loaded from verified prefix\n", n);
    }
    for (int n = first_n; n <= maximum_n; ++n) {
        size_t tasks = 0;
        uint64_t peak = 0;
        double seconds = 0;
        int threads = 0;
        char checkpoint_name[64];
        int checkpoint_length = snprintf(
            checkpoint_name, sizeof(checkpoint_name),
            "a131529_01_n%d.checkpoint", n);
        if (checkpoint_length < 0 ||
            (size_t)checkpoint_length >= sizeof(checkpoint_name)) {
            die("checkpoint filename overflow");
        }
        char *checkpoint_path =
            path_beside_executable(argv[0], checkpoint_name);
        U128 value = calculate_term(n, memory_limit, checkpoint_path,
                                    &tasks, &peak, &seconds, &threads);
        free(checkpoint_path);
        if (n <= DIRECT_CHECK_MAX_N && value != direct_count(n)) {
            die("independent full-permutation check disagrees");
        }
        if (n <= VERIFIED_MAX_N &&
            value != parse_u128(verified_terms[n])) {
            die("computed value disagrees with the OEIS check term");
        }
        fprintf(stderr,
                "A131529 n=%d tasks=%zu threads=%d "
                "time=%.3fs peak=%.3f MiB%s%s\n",
                n, tasks, threads, seconds,
                (double)peak / (1024.0 * 1024.0),
                n <= DIRECT_CHECK_MAX_N ? " direct-check=ok" : "",
                n <= VERIFIED_MAX_N ? " known-term=ok" : "");
        write_term(bfile, part_path, n, value);
    }

    if (fclose(bfile) != 0) {
        fprintf(stderr, "error: could not close %s: %s\n",
                part_path, strerror(errno));
        close(lock_descriptor);
        free(lock_path);
        free(final_path);
        free(part_path);
        return EXIT_FAILURE;
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: could not rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        close(lock_descriptor);
        free(lock_path);
        free(final_path);
        free(part_path);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "wrote %s (n=1..%d)\n", final_path, maximum_n);
    close(lock_descriptor);
    free(lock_path);
    free(final_path);
    free(part_path);
    return EXIT_SUCCESS;
}
