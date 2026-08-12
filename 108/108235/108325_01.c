/*
 * A108235 -- Hercher-Niedermeyer recursive algorithm (Theorems 1--3).
 *
 * Reference: C. Hercher and F. Niedermeyer, "Efficient Calculation the
 * Number of Partitions of the Set {1,2,...,3n} into Subsets {x,y,z}
 * Satisfying x+y=z", arXiv:2307.00303 (2023).
 *
 * Count partitions of {1,...,3*n} into unordered triples {x,y,z} with
 * x<y<z and x+y=z.  At every node choose the smallest remaining element
 * b_1.  It cannot be z in its triple, so it is x; choosing its other
 * addend y uniquely fixes z=x+y.  Conversely every valid partition has
 * exactly this branch.  Induction on the number of triples therefore shows
 * that the recursion counts every unordered partition exactly once.
 *
 * For S={b_1<...<b_(3m)} define
 *
 *   S_1=sum_(i=1)^(2m) b_i,    S_2=sum_(i=2m+1)^(3m) b_i.
 *
 * Theorem 1 of Hercher and Niedermeyer proves S_1<=S_2 for every completable
 * state.  Theorem 2 supplies the following exact pruning rules:
 *
 *   (a) b_1+b_(2m)<=b_(3m);
 *   (b) equality in (a) forces {b_1,b_(2m),b_(3m)} and S_1=S_2;
 *   (c) once S_1=S_2, every triple has two elements among the first 2m
 *       and its sum among the last m; equality persists in all descendants.
 *
 * Algorithm 2's three update formulas maintain S_1,S_2 exactly after a
 * triple is removed, according to whether y and z lie below or above the
 * old 2m boundary.  Theorem 3 proves that, for the original integer interval
 * and this smallest-first recursion, any completable state with m=2 must
 * have S_1=S_2.  This implementation uses all these rules: it is the fastest
 * algorithm compared in Tables 1 and 2 of arXiv:2307.00303.
 *
 * A uint64_t bit mask stores S.  Root branches are distributed dynamically
 * among pthread workers; each worker then runs the same exact recursive
 * algorithm independently.  The congruence obstruction is also exact:
 * twice the sum of all z's is 1+...+3n=3n(3n+1)/2, hence 4 divides
 * 3n(3n+1), which is possible only for n==0 or 1 (mod 4).
 *
 * Safety bounds at MAX_N=17: all values fit in 51 mask bits and S_1,S_2
 * are at most 1+...+51=1326, so int arithmetic is ample.  Even the number
 * of unrestricted partitions into triples,
 *
 *       51!/(6^17 17!) = 257635246075526315926272895552000000000,
 *
 * is below 2^128, and every U128 accumulation is checked nevertheless.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       108325_01.c -o 108325_01
 *
 * Usage:
 *   ./108325_01 13 --threads 8
 *   ./108325_01 --term 17 --threads 8
 *   ./108325_01 --check --threads 8 --no-bfile
 * Results are atomically recorded in b108235_01.txt by default.  Use
 * --output FILE to select another b-file or --no-bfile to disable writing.
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "108325_01.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 17
#define DEFAULT_N 9
#define DEFAULT_CHECK_N 9
#define MAX_THREADS 64
#define MAX_VALUES (3 * MAX_N)
#define MAX_ROOT_TASKS MAX_VALUES

_Static_assert(MAX_VALUES < 64,
               "remaining-set bit mask requires fewer than 64 values");

static const char *output_path = "b108235_01.txt";
static bool write_bfile = true;
static int requested_threads = 4;

static const char *const known[] = {
    "1", "1", "0", "0", "8", "21", "0", "0", "3040", "20505",
    "0", "0", "10567748", "103372655", "0", "0",
    "142664107305", "1836652173363"
};

static void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static int parse_n(const char *text)
{
    char *end = NULL;
    errno = 0;
    const long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < 0 || value > MAX_N) {
        fprintf(stderr, "error: N must be in 0..%d: %s\n", MAX_N, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static int parse_threads(const char *text)
{
    char *end = NULL;
    errno = 0;
    const long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < 1 || value > MAX_THREADS) {
        fprintf(stderr, "error: threads must be in 1..%d: %s\n",
                MAX_THREADS, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static double now_seconds(void)
{
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) {
        die("clock_gettime failed");
    }
    return (double)time.tv_sec + (double)time.tv_nsec / 1e9;
}

static int print_u128(FILE *stream, U128 value)
{
    char digits[40];
    size_t length = 0;
    do {
        digits[length++] = (char)('0' + (unsigned)(value % 10));
        value /= 10;
    } while (value != 0);
    while (length != 0) {
        if (fputc(digits[--length], stream) == EOF) {
            return -1;
        }
    }
    return 0;
}

static void print_term(int n, U128 value)
{
    if (fprintf(stdout, "%d ", n) < 0 ||
        print_u128(stdout, value) != 0 ||
        fputc('\n', stdout) == EOF || fflush(stdout) != 0) {
        die("cannot write result to standard output");
    }
}

static bool parse_u128(const char *text, U128 *result)
{
    const U128 maximum = ~(U128)0;
    U128 value = 0;
    if (*text == '\0') {
        return false;
    }
    for (; *text != '\0'; ++text) {
        if (*text < '0' || *text > '9') {
            return false;
        }
        const unsigned digit = (unsigned)(*text - '0');
        if (value > (maximum - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
    }
    *result = value;
    return true;
}

static bool add_u128(U128 *destination, U128 addend)
{
    const U128 old = *destination;
    *destination += addend;
    return *destination >= old;
}

static int acquire_bfile_lock(void)
{
    const char suffix[] = ".lock";
    const size_t length = strlen(output_path);
    if (length > SIZE_MAX - sizeof(suffix)) {
        die("b-file lock path is too long");
    }
    char *path = malloc(length + sizeof(suffix));
    if (path == NULL) {
        die("cannot allocate b-file lock path");
    }
    memcpy(path, output_path, length);
    memcpy(path + length, suffix, sizeof(suffix));
    const int fd = open(path, O_RDWR | O_CREAT, 0666);
    free(path);
    if (fd < 0) {
        die("cannot open b-file lock");
    }
    struct flock lock = {
        .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0
    };
    int result;
    do {
        result = fcntl(fd, F_SETLKW, &lock);
    } while (result != 0 && errno == EINTR);
    if (result != 0) {
        close(fd);
        die("cannot lock b-file");
    }
    return fd;
}

static void release_bfile_lock(int fd)
{
    struct flock lock = {
        .l_type = F_UNLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0
    };
    const bool unlock_failed = fcntl(fd, F_SETLK, &lock) != 0;
    const bool close_failed = close(fd) != 0;
    if (unlock_failed || close_failed) {
        die("cannot release b-file lock");
    }
}

static void sync_parent_directory(const char *path)
{
    const char *slash = strrchr(path, '/');
    char *allocated = NULL;
    const char *directory;
    if (slash == NULL) {
        directory = ".";
    } else if (slash == path) {
        directory = "/";
    } else {
        const size_t length = (size_t)(slash - path);
        allocated = malloc(length + 1);
        if (allocated == NULL) {
            die("cannot allocate b-file directory path");
        }
        memcpy(allocated, path, length);
        allocated[length] = '\0';
        directory = allocated;
    }
    const int fd = open(directory, O_RDONLY);
    free(allocated);
    if (fd < 0) {
        die("cannot open b-file parent directory");
    }
    const int result = fsync(fd);
    const int saved_errno = errno;
    const bool close_failed = close(fd) != 0;
    if ((result != 0 && saved_errno != EINVAL && saved_errno != ENOTSUP) ||
        close_failed) {
        die("cannot synchronize b-file parent directory");
    }
}

static void store_bfile_term(int n, U128 value)
{
    const int lock_fd = acquire_bfile_lock();
    U128 values[MAX_N + 1] = {0};
    bool present[MAX_N + 1] = {false};
    int previous = -1;
    mode_t output_mode = 0644;
    struct stat metadata;
    if (stat(output_path, &metadata) == 0) {
        output_mode = metadata.st_mode & 0777;
    } else if (errno != ENOENT) {
        die("cannot inspect b-file");
    }

    FILE *input = fopen(output_path, "r");
    if (input == NULL && errno != ENOENT) {
        die("cannot open existing b-file");
    }
    if (input != NULL) {
        char line[128];
        while (fgets(line, sizeof(line), input) != NULL) {
            int index;
            char number[64];
            char extra;
            if (sscanf(line, "%d %63s %c", &index, number, &extra) != 2 ||
                index < 0 || index > MAX_N || index <= previous ||
                !parse_u128(number, &values[index])) {
                fclose(input);
                die("existing b-file is malformed or not strictly ordered");
            }
            present[index] = true;
            previous = index;
        }
        if (ferror(input) || fclose(input) != 0) {
            die("cannot read existing b-file");
        }
    }

    if (present[n]) {
        if (values[n] != value) {
            die("computed term disagrees with existing b-file");
        }
        release_bfile_lock(lock_fd);
        return;
    }
    values[n] = value;
    present[n] = true;

    const char suffix[] = ".tmp.XXXXXX";
    const size_t path_length = strlen(output_path);
    if (path_length > SIZE_MAX - sizeof(suffix)) {
        die("b-file path is too long");
    }
    char *temporary = malloc(path_length + sizeof(suffix));
    if (temporary == NULL) {
        die("cannot allocate b-file temporary path");
    }
    memcpy(temporary, output_path, path_length);
    memcpy(temporary + path_length, suffix, sizeof(suffix));
    const int fd = mkstemp(temporary);
    if (fd < 0) {
        free(temporary);
        die("cannot create temporary b-file");
    }
    if (fchmod(fd, output_mode) != 0) {
        close(fd);
        unlink(temporary);
        free(temporary);
        die("cannot set temporary b-file permissions");
    }
    FILE *output = fdopen(fd, "w");
    if (output == NULL) {
        close(fd);
        unlink(temporary);
        free(temporary);
        die("cannot open temporary b-file stream");
    }
    bool failed = false;
    for (int index = 0; index <= MAX_N; ++index) {
        if (!present[index]) {
            continue;
        }
        if (fprintf(output, "%d ", index) < 0 ||
            print_u128(output, values[index]) != 0 ||
            fputc('\n', output) == EOF) {
            failed = true;
            break;
        }
    }
    if (!failed && fflush(output) != 0) {
        failed = true;
    }
    if (!failed && fsync(fd) != 0) {
        failed = true;
    }
    if (fclose(output) != 0) {
        failed = true;
    }
    if (failed) {
        unlink(temporary);
        free(temporary);
        die("cannot write temporary b-file");
    }
    if (rename(temporary, output_path) != 0) {
        unlink(temporary);
        free(temporary);
        die("cannot atomically replace b-file");
    }
    sync_parent_directory(output_path);
    free(temporary);
    release_bfile_lock(lock_fd);
    fprintf(stderr, "108325_01: recorded computed A108235 term n=%d in %s\n",
            n, output_path);
}

static unsigned smallest_value(uint64_t mask)
{
    if (mask == 0) {
        die("internal search mask is empty");
    }
    return (unsigned)__builtin_ctzll(mask) + 1U;
}

static unsigned largest_value(uint64_t mask)
{
    if (mask == 0) {
        die("internal search mask is empty");
    }
    return 64U - (unsigned)__builtin_clzll(mask);
}

static unsigned value_at_rank(uint64_t mask, unsigned zero_based_rank)
{
    while (zero_based_rank != 0) {
        if (mask == 0) {
            die("rank exceeds remaining search set");
        }
        mask &= mask - 1;
        --zero_based_rank;
    }
    return smallest_value(mask);
}

static uint64_t value_bit(unsigned value)
{
    if (value == 0 || value > MAX_VALUES) {
        die("search value is outside bit-mask range");
    }
    return UINT64_C(1) << (value - 1U);
}

static bool is_one_triple(uint64_t mask)
{
    if (__builtin_popcountll(mask) != 3) {
        die("terminal search state does not contain three values");
    }
    const unsigned x = smallest_value(mask);
    mask ^= value_bit(x);
    const unsigned y = smallest_value(mask);
    mask ^= value_bit(y);
    const unsigned z = smallest_value(mask);
    return x + y == z;
}

static U128 count_reduced(uint64_t mask, unsigned m);

static U128 count_general(uint64_t mask, unsigned m, int sum1, int sum2)
{
    if ((unsigned)__builtin_popcountll(mask) != 3U * m) {
        die("general recursion mask cardinality invariant failed");
    }
    if (m == 1) {
        return is_one_triple(mask) ? 1 : 0;
    }
    if (sum1 > sum2) {                  /* Theorem 1. */
        return 0;
    }
    if (sum1 == sum2) {                 /* Theorem 2(1). */
        return count_reduced(mask, m);
    }
    if (m == 2) {                       /* Theorem 3. */
        return 0;
    }

    const unsigned x = smallest_value(mask);
    const unsigned lower_last = value_at_rank(mask, 2U * m - 1U);
    const unsigned upper_first = value_at_rank(mask, 2U * m);
    const unsigned maximum = largest_value(mask);

    if (x + lower_last > maximum) {     /* Theorem 2(2). */
        return 0;
    }
    if (x + lower_last == maximum) {
        /* Theorem 2(3) would force sum1==sum2, contrary to this branch. */
        return 0;
    }
    if (x + upper_first > maximum) {
        /* Theorem 2(4) likewise forces equality of the two sums. */
        return 0;
    }

    const uint64_t x_bit = value_bit(x);
    U128 answer = 0;
    uint64_t candidates = mask ^ x_bit;
    while (candidates != 0) {
        const unsigned y = smallest_value(candidates);
        candidates &= candidates - 1;
        const unsigned z = x + y;
        if (z > maximum) {
            break;                      /* All later y are larger. */
        }
        const uint64_t z_bit = value_bit(z);
        if ((mask & z_bit) == 0) {
            continue;
        }
        const uint64_t child_mask = mask ^ x_bit ^ value_bit(y) ^ z_bit;
        int child_sum1;
        int child_sum2;

        /* Algorithm 2, lines 13--21, converted from ranks to boundaries. */
        if (y >= upper_first) {
            child_sum1 = sum1 - (int)x - (int)lower_last;
            child_sum2 = sum2 - (int)y - (int)z + (int)lower_last;
        } else if (z >= upper_first) {
            child_sum1 = sum1 - (int)x - (int)y;
            child_sum2 = sum2 - (int)z;
        } else {
            child_sum1 = sum1 - (int)x - (int)y - (int)z +
                         (int)upper_first;
            child_sum2 = sum2 - (int)upper_first;
        }

        const U128 addend = child_sum1 == child_sum2 ?
            count_reduced(child_mask, m - 1U) :
            count_general(child_mask, m - 1U, child_sum1, child_sum2);
        if (!add_u128(&answer, addend)) {
            die("A108235 answer overflowed unsigned __int128");
        }
    }
    return answer;
}

static U128 count_reduced(uint64_t mask, unsigned m)
{
    if ((unsigned)__builtin_popcountll(mask) != 3U * m) {
        die("reduced recursion mask cardinality invariant failed");
    }
    if (m == 1) {
        return is_one_triple(mask) ? 1 : 0;
    }

    const unsigned x = smallest_value(mask);
    const unsigned lower_last = value_at_rank(mask, 2U * m - 1U);
    const unsigned upper_first = value_at_rank(mask, 2U * m);
    const unsigned maximum = largest_value(mask);
    if (x + lower_last > maximum) {     /* Theorem 2(2). */
        return 0;
    }
    if (x + lower_last == maximum) {    /* Forced by Theorem 2(3). */
        const uint64_t child = mask ^ value_bit(x) ^
                               value_bit(lower_last) ^ value_bit(maximum);
        return count_reduced(child, m - 1U);
    }

    const uint64_t x_bit = value_bit(x);
    U128 answer = 0;
    uint64_t candidates = mask ^ x_bit;
    while (candidates != 0) {
        const unsigned y = smallest_value(candidates);
        candidates &= candidates - 1;
        if (y > lower_last) {
            break;                      /* y must lie in the first 2m. */
        }
        const unsigned z = x + y;
        if (z > maximum) {
            break;
        }
        const uint64_t z_bit = value_bit(z);
        if (z < upper_first || (mask & z_bit) == 0) {
            continue;                   /* z must lie in the last m. */
        }
        const uint64_t child = mask ^ x_bit ^ value_bit(y) ^ z_bit;
        if (!add_u128(&answer, count_reduced(child, m - 1U))) {
            die("A108235 answer overflowed unsigned __int128");
        }
    }
    return answer;
}

typedef struct {
    uint64_t mask;
    unsigned m;
    int sum1;
    int sum2;
    bool reduced;
} RootTask;

typedef struct {
    _Atomic unsigned next;
    unsigned count;
    RootTask tasks[MAX_ROOT_TASKS];
} RootSchedule;

typedef struct {
    RootSchedule *schedule;
    U128 answer;
} Worker;

static unsigned build_root_tasks(int n, RootTask tasks[MAX_ROOT_TASKS])
{
    const unsigned m = (unsigned)n;
    const unsigned positions = 3U * m;
    const uint64_t mask = (UINT64_C(1) << positions) - 1;
    const unsigned x = 1;
    const unsigned lower_last = 2U * m;
    const unsigned upper_first = 2U * m + 1U;
    const unsigned maximum = positions;
    const int sum1 = (int)(m * (2U * m + 1U));
    const int total = (int)(positions * (positions + 1U) / 2U);
    const int sum2 = total - sum1;
    unsigned count = 0;

    /* For the full interval every z=x+y<=3n is present. */
    for (unsigned y = 2; x + y <= maximum; ++y) {
        const unsigned z = x + y;
        int child_sum1;
        int child_sum2;
        if (y >= upper_first) {
            child_sum1 = sum1 - (int)x - (int)lower_last;
            child_sum2 = sum2 - (int)y - (int)z + (int)lower_last;
        } else if (z >= upper_first) {
            child_sum1 = sum1 - (int)x - (int)y;
            child_sum2 = sum2 - (int)z;
        } else {
            child_sum1 = sum1 - (int)x - (int)y - (int)z +
                         (int)upper_first;
            child_sum2 = sum2 - (int)upper_first;
        }
        if (count >= MAX_ROOT_TASKS) {
            die("root-task array is too small");
        }
        tasks[count++] = (RootTask) {
            .mask = mask ^ value_bit(x) ^ value_bit(y) ^ value_bit(z),
            .m = m - 1U,
            .sum1 = child_sum1,
            .sum2 = child_sum2,
            .reduced = child_sum1 == child_sum2
        };
    }
    return count;
}

static void *worker_main(void *argument)
{
    Worker *worker = argument;
    for (;;) {
        const unsigned index = atomic_fetch_add_explicit(
            &worker->schedule->next, 1U, memory_order_relaxed);
        if (index >= worker->schedule->count) {
            break;
        }
        const RootTask *const task = &worker->schedule->tasks[index];
        const U128 addend = task->reduced ?
            count_reduced(task->mask, task->m) :
            count_general(task->mask, task->m, task->sum1, task->sum2);
        if (!add_u128(&worker->answer, addend)) {
            die("worker answer overflowed unsigned __int128");
        }
    }
    return NULL;
}

static U128 a108235(int n)
{
    if (n == 0) {
        return 1;
    }
    if ((n & 3) != 0 && (n & 3) != 1) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }

    RootSchedule schedule = {0};
    schedule.count = build_root_tasks(n, schedule.tasks);
    int threads = requested_threads;
    if ((unsigned)threads > schedule.count) {
        threads = (int)schedule.count;
    }
    if (threads < 1) {
        die("valid A108235 search unexpectedly has no root tasks");
    }
    Worker *workers = calloc((size_t)threads, sizeof(*workers));
    pthread_t *ids = calloc((size_t)threads, sizeof(*ids));
    if (workers == NULL || ids == NULL) {
        free(workers);
        free(ids);
        die("cannot allocate search workers");
    }

    const double started = now_seconds();
    for (int id = 0; id < threads; ++id) {
        workers[id].schedule = &schedule;
        const int error = pthread_create(&ids[id], NULL,
                                         worker_main, &workers[id]);
        if (error != 0) {
            fprintf(stderr, "error: pthread_create: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }
    U128 answer = 0;
    for (int id = 0; id < threads; ++id) {
        const int error = pthread_join(ids[id], NULL);
        if (error != 0) {
            fprintf(stderr, "error: pthread_join: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
        if (!add_u128(&answer, workers[id].answer)) {
            die("A108235 answer overflowed unsigned __int128");
        }
    }
    free(workers);
    free(ids);
    fprintf(stderr,
            "108325_01: A108235 n=%d, Hercher-Niedermeyer Theorems 1--3, "
            "root-tasks=%u, threads=%d, %.3f s\n",
            n, schedule.count, threads, now_seconds() - started);
    return answer;
}

static void verify_known(int n, U128 value)
{
    const int count = (int)(sizeof(known) / sizeof(known[0]));
    if (n >= count) {
        return;
    }
    U128 expected;
    if (!parse_u128(known[n], &expected)) {
        die("invalid built-in known term");
    }
    if (value != expected) {
        fprintf(stderr, "error: A108235 mismatch at n=%d: got ", n);
        print_u128(stderr, value);
        fprintf(stderr, ", expected %s\n", known[n]);
        exit(EXIT_FAILURE);
    }
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--threads T] [--output FILE]\n"
            "       %s --term N [--threads T] [--output FILE]\n"
            "       %s --check [--threads T] [--no-bfile]\n"
            "N must be in 0..%d; T must be in 1..%d.\n",
            program, program, program, MAX_N, MAX_THREADS);
}

int main(int argc, char **argv)
{
    enum { MODE_RANGE, MODE_TERM, MODE_CHECK } mode = MODE_RANGE;
    int maximum = -1;
    bool have_mode = false;
    bool have_threads = false;
    bool have_output = false;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        if (!strcmp(argv[i], "--threads")) {
            if (have_threads || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            requested_threads = parse_threads(argv[i]);
            have_threads = true;
        } else if (!strcmp(argv[i], "--output")) {
            if (have_output || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            output_path = argv[i];
            write_bfile = true;
            have_output = true;
        } else if (!strcmp(argv[i], "--no-bfile")) {
            if (have_output) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            write_bfile = false;
            have_output = true;
        } else if (!strcmp(argv[i], "--term")) {
            if (have_mode || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = MODE_TERM;
            maximum = parse_n(argv[i]);
            have_mode = true;
        } else if (!strcmp(argv[i], "--check")) {
            if (have_mode) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = MODE_CHECK;
            have_mode = true;
        } else if (argv[i][0] != '-' && !have_mode) {
            maximum = parse_n(argv[i]);
            mode = MODE_RANGE;
            have_mode = true;
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (maximum < 0) {
        maximum = DEFAULT_N;
    }
    if (mode == MODE_CHECK) {
        for (int n = 0; n <= DEFAULT_CHECK_N; ++n) {
            verify_known(n, a108235(n));
        }
        printf("ok: A108235 terms n=0..%d verified with "
               "Hercher-Niedermeyer Theorems 1--3\n", DEFAULT_CHECK_N);
        return EXIT_SUCCESS;
    }
    if (mode == MODE_TERM) {
        const U128 value = a108235(maximum);
        verify_known(maximum, value);
        if (write_bfile) {
            store_bfile_term(maximum, value);
        }
        print_term(maximum, value);
        return EXIT_SUCCESS;
    }

    for (int n = 0; n <= maximum; ++n) {
        const U128 value = a108235(n);
        verify_known(n, value);
        if (write_bfile) {
            store_bfile_term(n, value);
        }
        print_term(n, value);
    }
    return EXIT_SUCCESS;
}
