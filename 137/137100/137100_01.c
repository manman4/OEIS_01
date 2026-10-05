/*
 * A137100: positive integers k for which both k and k*k have decimal
 * digits only in {2,4,6,7}.  https://oeis.org/A137100
 *
 * Build (no GMP or OpenMP needed):
 *   cc -O3 -march=native -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *      137100_01.c -o 137100_01
 * Run from the directory containing this file:
 *   ./137100_01                         # next term with 27..40 digits
 *   ./137100_01 --max-digits 60 --threads 8
 *   ./137100_01 --min-digits 1 --max-digits 26 --all
 *   ./137100_01 --check 4762
 *   ./137100_01 --self-test
 *
 * Bounds are inclusive digit lengths, not bounds on k*k.  The default
 * interval contains every admissible k with 10^26 < k < 10^40.  Neither
 * endpoint itself has allowed digits.  The published absence of terms
 * through 10^26 is NOT used for pruning: use --min-digits 1 to recheck it.
 * Default mode completes each length, prints its smallest match, and stops
 * after the first successful length.  --all prints all matches in increasing
 * order through --max-digits.  stdout contains k only; stderr has progress,
 * newly discovered candidates, and completion/interruption reports.
 * Exit codes: 0 = completed (possibly no match), 1 = error, 130 = interrupted.
 * No checkpoint files are written.  After interruption, rerun starting at
 * the first digit length not reported complete.  Increasing the bound can
 * be expensive; the program does not claim a next term must exist.
 *
 * Exact pruning proof:
 * 1. At depth n, a[0..n-1] are the low n digits of k, all allowed.
 *    c[j] = sum_{u+v=j} a[u]*a[v] are uncarried square coefficients.
 *    Appending digit d at position n adds 2*d*a[u] to c[n+u] (u<n)
 *    and d*d to c[2*n].  Undoing these additions restores the parent.
 * 2. carry is the carry into position n after processing c[0..n-1].
 *    Thus (c[n]+carry)%10 is the next square digit.  Any later digit of
 *    k contributes only at positions >=n+1, so a forbidden digit here
 *    cannot be repaired by an extension.  Rejection loses no solution.
 * 3. At target depth N, the low N square digits already passed.  Processing
 *    c[N..2*N-2] and the remaining carry checks every other square digit.
 *    The leading square digit is nonzero because k has no leading zero.
 *    There is no floating point, machine-sized k, or heuristic pruning.
 * 4. Tasks partition the surviving low min(N,8) digits.  Every word of
 *    length N over the alphabet occurs in exactly one task unless rejected
 *    by (2).  Exhausting a length and sorting its matches proves minimality
 *    within that length; lengths are exhausted in increasing order.
 *
 * Arithmetic limits: N<=256, each coefficient<=49*N, and every carried
 * column<=55*N.  uint32_t is ample.  Recursion and arrays are bounded.
 * Diagnostic node/survivor counters saturate at UINT64_MAX.
 * The independent self-test enumerates all alphabet words through 8 digits
 * as uint64_t integers, squares them directly, and compares both the suffix
 * survivor counts and the complete sorted match lists with this search.
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if defined(__APPLE__)
extern int sysctlbyname(const char *, void *, size_t *, void *, size_t);
#endif

#define MAX_DIGITS 256U
#define SPLIT_DIGITS 8U
#define MAX_THREADS 256U
static const unsigned alphabet[4] = {2, 4, 6, 7};
static volatile sig_atomic_t interrupted;

typedef struct { unsigned char a[SPLIT_DIGITS]; } Task;
typedef struct {
    unsigned length, split;
    Task *tasks;
    size_t task_count, task_capacity, next_task, done;
    char **matches;
    size_t count, capacity;
    uint64_t leaves, nodes;
    bool quiet;
    pthread_mutex_t lock;
} Search;
typedef struct {
    Search *search;
    unsigned char a[MAX_DIGITS];
    uint32_t c[2 * MAX_DIGITS];
    uint64_t leaves, nodes;
} Worker;

static void die(const char *s)
{
    fprintf(stderr, "error: %s\n", s);
    exit(1);
}
static void *resize(void *p, size_t n)
{
    void *q = realloc(p, n);
    if (!q) die("out of memory");
    return q;
}
static bool good(unsigned d) { return d == 2 || d == 4 || d == 6 || d == 7; }
static uint64_t add_count(uint64_t a, uint64_t b)
{
    return UINT64_MAX - a < b ? UINT64_MAX : a + b;
}
static void on_signal(int sig) { (void)sig; interrupted = 1; }
static double seconds(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t)) die("clock_gettime failed");
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}
static unsigned number(const char *s, unsigned max)
{
    if (!*s) die("empty numeric argument");
    for (const char *p = s; *p; ++p)
        if (*p < '0' || *p > '9') die("expected a positive decimal integer");
    char *end;
    errno = 0;
    unsigned long v = strtoul(s, &end, 10);
    if (errno || *end || v < 1 || v > max) die("numeric argument out of range");
    return (unsigned)v;
}
static void change(Worker *w, unsigned n, unsigned d, bool add)
{
    for (unsigned i = 0; i < n; ++i) {
        uint32_t v = 2U * d * w->a[i];
        if (add) w->c[n+i] += v;
        else w->c[n+i] -= v;
    }
    if (add) w->c[2*n] += d*d;
    else w->c[2*n] -= d*d;
}
static bool upper_good(const Worker *w, unsigned n, uint32_t carry)
{
    for (unsigned i = n; i < 2*n-1; ++i) {
        uint32_t t = w->c[i] + carry;
        if (!good(t % 10)) return false;
        carry = t / 10;
    }
    while (carry) {
        if (!good(carry % 10)) return false;
        carry /= 10;
    }
    return true;
}
static void record(Worker *w)
{
    Search *s = w->search;
    char *value = resize(NULL, s->length + 1U);
    for (unsigned i = 0; i < s->length; ++i)
        value[i] = (char)('0' + w->a[s->length - 1U - i]);
    value[s->length] = '\0';
    pthread_mutex_lock(&s->lock);
    if (s->count == s->capacity) {
        if (s->capacity > SIZE_MAX / (2 * sizeof(*s->matches)))
            die("too many matches to store");
        s->capacity = s->capacity ? 2*s->capacity : 16;
        s->matches = resize(s->matches, s->capacity * sizeof(*s->matches));
    }
    s->matches[s->count++] = value;
    if (!s->quiet) fprintf(stderr, "candidate: %s (length still incomplete)\n", value);
    pthread_mutex_unlock(&s->lock);
}
static void dfs(Worker *w, unsigned n, uint32_t carry, bool tasks)
{
    if (interrupted) return;
    Search *s = w->search;
    w->nodes = add_count(w->nodes, 1);
    if (tasks && n == s->split) {
        if (s->task_count == s->task_capacity) {
            s->task_capacity = s->task_capacity ? 2*s->task_capacity : 128;
            s->tasks = resize(s->tasks, s->task_capacity * sizeof(*s->tasks));
        }
        memcpy(s->tasks[s->task_count++].a, w->a, n);
        return;
    }
    if (n == s->length) {
        w->leaves = add_count(w->leaves, 1);
        if (upper_good(w, n, carry)) record(w);
        return;
    }
    for (unsigned j = 0; j < 4; ++j) {
        unsigned d = alphabet[j];
        w->a[n] = (unsigned char)d;
        change(w, n, d, true);
        uint32_t t = w->c[n] + carry;
        if (good(t % 10)) dfs(w, n+1, t/10, tasks);
        change(w, n, d, false);
    }
}
static void *work(void *arg)
{
    Search *s = arg;
    Worker w = {.search = s};
    for (;;) {
        pthread_mutex_lock(&s->lock);
        size_t task = s->next_task++;
        pthread_mutex_unlock(&s->lock);
        if (interrupted || task >= s->task_count) break;
        memset(w.c, 0, sizeof(w.c));
        uint32_t carry = 0;
        for (unsigned n = 0; n < s->split; ++n) {
            unsigned d = s->tasks[task].a[n];
            w.a[n] = (unsigned char)d;
            change(&w, n, d, true);
            carry = (w.c[n] + carry) / 10;
        }
        dfs(&w, s->split, carry, false);
        pthread_mutex_lock(&s->lock);
        if (!interrupted) ++s->done;
        if (!s->quiet && (s->done % 16 == 0 || s->done == s->task_count))
            fprintf(stderr, "%u digits: %zu/%zu suffix tasks complete\n",
                    s->length, s->done, s->task_count);
        pthread_mutex_unlock(&s->lock);
    }
    pthread_mutex_lock(&s->lock);
    s->leaves = add_count(s->leaves, w.leaves);
    s->nodes = add_count(s->nodes, w.nodes);
    pthread_mutex_unlock(&s->lock);
    return NULL;
}
static int compare(const void *a, const void *b)
{
    return strcmp(*(char *const *)a, *(char *const *)b);
}
static void run(Search *s, unsigned n, unsigned threads, bool quiet)
{
    memset(s, 0, sizeof(*s));
    s->length = n;
    s->split = n < SPLIT_DIGITS ? n : SPLIT_DIGITS;
    s->quiet = quiet;
    if (pthread_mutex_init(&s->lock, NULL)) die("mutex initialization failed");
    Worker root = {.search = s};
    dfs(&root, 0, 0, true);
    s->nodes = root.nodes;
    pthread_t ids[MAX_THREADS];
    if (s->task_count < threads) threads = (unsigned)s->task_count;
    for (unsigned i = 0; i < threads; ++i)
        if (pthread_create(&ids[i], NULL, work, s)) die("pthread_create failed");
    for (unsigned i = 0; i < threads; ++i)
        if (pthread_join(ids[i], NULL)) die("pthread_join failed");
    if (s->count) qsort(s->matches, s->count, sizeof(*s->matches), compare);
}
static void release(Search *s)
{
    for (size_t i = 0; i < s->count; ++i) free(s->matches[i]);
    free(s->matches);
    free(s->tasks);
    pthread_mutex_destroy(&s->lock);
}
static bool integer_good(uint64_t k)
{
    do {
        if (!good((unsigned)(k % 10))) return false;
        k /= 10;
    } while (k);
    return true;
}
static void oracle(unsigned left, uint64_t k, uint64_t modulus,
                   uint64_t *leaves, char values[32][32], size_t *count)
{
    if (left) {
        for (unsigned j = 0; j < 4; ++j)
            oracle(left-1, 10*k + alphabet[j], modulus, leaves, values, count);
        return;
    }
    uint64_t square = k*k;
    uint64_t suffix = square % modulus;
    bool ok = true;
    for (uint64_t p = modulus; p > 1; p /= 10) {
        if (!good((unsigned)(suffix % 10))) ok = false;
        suffix /= 10;
    }
    if (ok) ++*leaves;
    if (integer_good(square)) {
        if (*count >= 32) die("self-test oracle capacity exceeded");
        snprintf(values[(*count)++], 32, "%" PRIu64, k);
    }
}
static void self_test(void)
{
    uint64_t modulus = 1;
    for (unsigned n = 1; n <= 8; ++n) {
        modulus *= 10;
        char values[32][32];
        size_t count = 0;
        uint64_t leaves = 0;
        oracle(n, 0, modulus, &leaves, values, &count);
        for (unsigned threads = 1; threads <= 4; threads *= 4) {
            Search s;
            run(&s, n, threads, true);
            if (s.leaves != leaves || s.count != count) die("self-test count mismatch");
            for (size_t i = 0; i < count; ++i)
                if (strcmp(s.matches[i], values[i])) die("self-test match mismatch");
            release(&s);
        }
    }
    fprintf(stderr, "self-test passed: exhaustive 1..8 digits, 1 and 4 threads\n");
}
static int check(const char *k)
{
    size_t n = strlen(k);
    if (!n || n > MAX_DIGITS || k[0] == '0') die("check expects 1..256 digits, no leading zero");
    uint32_t c[2*MAX_DIGITS] = {0};
    bool ok = true;
    for (size_t i = 0; i < n; ++i) {
        if (k[i] < '0' || k[i] > '9') die("check expects decimal digits");
        if (!good((unsigned)(k[i] - '0'))) ok = false;
    }
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j)
            c[i+j] += (uint32_t)(k[n-1-i]-'0') * (uint32_t)(k[n-1-j]-'0');
    for (size_t i = 0; i < 2*n-1; ++i) {
        c[i+1] += c[i]/10;
        c[i] %= 10;
    }
    size_t len = 2*n;
    while (len > 1 && c[len-1] == 0) --len;
    fprintf(stderr, "%s^2 = ", k);
    for (size_t i = len; i > 0; --i) {
        fprintf(stderr, "%u", c[i-1]);
        if (!good(c[i-1])) ok = false;
    }
    fprintf(stderr, "\n%s\n", ok ? "match" : "not a match");
    if (ok && (puts(k) == EOF || fflush(stdout))) die("stdout write failed");
    return 0;
}
static void usage(void)
{
    fprintf(stderr, "Usage: ./137100_01 [--min-digits N] [--max-digits N]\n"
                    "                    [--threads N] [--all]\n"
                    "       ./137100_01 --check K | --self-test | --help\n"
                    "Defaults: min=27, max=40; limits: digits<=256, threads<=256.\n");
}
int main(int argc, char **argv)
{
    unsigned first = 27, last = 40;
    long cpus = 1;
#if defined(__APPLE__)
    int cpu_count = 1;
    size_t cpu_size = sizeof(cpu_count);
    if (sysctlbyname("hw.logicalcpu", &cpu_count, &cpu_size, NULL, 0) == 0)
        cpus = cpu_count;
#elif defined(_SC_NPROCESSORS_ONLN)
    cpus = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    unsigned threads = cpus > 0 && cpus <= MAX_THREADS ? (unsigned)cpus : 1;
    bool all = false;
    if (argc == 2 && !strcmp(argv[1], "--help")) { usage(); return 0; }
    if (argc == 2 && !strcmp(argv[1], "--self-test")) { self_test(); return 0; }
    if (argc == 3 && !strcmp(argv[1], "--check")) return check(argv[2]);
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--all")) { all = true; continue; }
        if (i+1 >= argc) { usage(); die("missing argument"); }
        if (!strcmp(argv[i], "--min-digits")) first = number(argv[++i], MAX_DIGITS);
        else if (!strcmp(argv[i], "--max-digits")) last = number(argv[++i], MAX_DIGITS);
        else if (!strcmp(argv[i], "--threads")) threads = number(argv[++i], MAX_THREADS);
        else { usage(); die("unknown option"); }
    }
    if (first > last) die("min-digits exceeds max-digits");
    if (signal(SIGINT, on_signal) == SIG_ERR || signal(SIGTERM, on_signal) == SIG_ERR)
        die("signal handler installation failed");
    fprintf(stderr, "search: %u..%u digits inclusive, %u threads\n", first, last, threads);
    for (unsigned n = first; n <= last; ++n) {
        double start = seconds();
        Search s;
        run(&s, n, threads, false);
        if (interrupted) {
            fprintf(stderr, "interrupted: %u digits INCOMPLETE; restart at this length\n", n);
            release(&s);
            return 130;
        }
        fprintf(stderr, "%u digits COMPLETE: nodes=%" PRIu64 ", suffix survivors=%" PRIu64
                        ", matches=%zu, %.3f s\n", n, s.nodes, s.leaves, s.count, seconds()-start);
        size_t emit = all ? s.count : (s.count ? 1 : 0);
        for (size_t i = 0; i < emit; ++i) {
            if (puts(s.matches[i]) == EOF || fflush(stdout)) die("stdout write failed");
        }
        bool found = s.count != 0;
        release(&s);
        if (found && !all) {
            fprintf(stderr, "smallest match in the requested interval found; search stopped\n");
            return 0;
        }
    }
    fprintf(stderr, "requested %u..%u digit interval COMPLETE\n", first, last);
    return 0;
}
