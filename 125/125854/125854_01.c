/*
 * A125854
 *
 * Primes p such that p divides the numerator of H_((p+1)/2).
 * Eisenstein's congruence gives the much faster equivalent test
 *
 *     q_p(2) = (2^(p-1)-1)/p == 1 (mod p),
 *
 * or, without forming the Fermat quotient,
 *
 *     2^(p-1) == 1+p (mod p^2).
 *
 * This program enumerates primes in an inclusive uint64_t interval with an
 * odd-only segmented sieve, then applies one modular exponentiation per
 * prime.  Segments are distributed dynamically among POSIX threads.
 *
 * Build (macOS/Homebrew or another pkg-config setup):
 *
 *   cc -O3 -march=native -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *      125854_01.c -o 125854_01 $(pkg-config --cflags --libs gmp) -lm
 *
 * Usage:
 *
 *   ./125854_01 START END [THREADS [SEGMENT_ODDS]]
 *   ./125854_01 --self-test
 *
 * START and END are inclusive.  SEGMENT_ODDS is the number of odd integers
 * represented by one segment (default: 1048576).  Matches are written to
 * stdout immediately; diagnostics and progress go to stderr.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <gmp.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#if defined(__APPLE__)
/* Avoid pulling BSD-only typedefs through <sys/sysctl.h> in strict POSIX mode. */
extern int sysctlbyname(const char *, void *, size_t *, void *, size_t);
#endif

#define DEFAULT_SEGMENT_ODDS UINT64_C(1048576)
#define MAX_THREADS 1024u

_Static_assert(sizeof(unsigned long) >= sizeof(uint64_t),
               "this program needs a 64-bit unsigned long for GMP");

typedef struct {
    uint32_t *values;
    size_t count;
} PrimeTable;

typedef struct {
    mpz_t base;
    mpz_t p;
    mpz_t modulus;
    mpz_t residue;
    mpz_t target;
} PowContext;

typedef struct {
    uint64_t first_odd;
    uint64_t odd_count;
    uint64_t segment_odds;
    uint64_t segment_count;
    const PrimeTable *base_primes;

    atomic_uint_fast64_t next_segment;
    atomic_uint_fast64_t completed_segments;
    atomic_uint_fast64_t primes_tested;

    pthread_mutex_t output_lock;
    double started_at;
    double last_report_at;
    bool print_matches;
    bool show_progress;

    uint64_t *matches;
    size_t match_count;
    size_t match_capacity;
    bool failed;
} SearchJob;

static void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static void *xmalloc(size_t size)
{
    void *p = malloc(size == 0 ? 1 : size);
    if (p == NULL) {
        die("out of memory");
    }
    return p;
}

static void *xcalloc(size_t count, size_t size)
{
    void *p = calloc(count == 0 ? 1 : count, size);
    if (p == NULL) {
        die("out of memory");
    }
    return p;
}

static double monotonic_seconds(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        die("clock_gettime failed");
    }
    return (double)ts.tv_sec + 1.0e-9 * (double)ts.tv_nsec;
}

static uint64_t parse_u64(const char *text, const char *name)
{
    char *end = NULL;
    unsigned long long value;

    if (text == NULL || *text == '\0' || *text == '-') {
        fprintf(stderr, "error: invalid %s: %s\n", name,
                text == NULL ? "(null)" : text);
        exit(EXIT_FAILURE);
    }
    errno = 0;
    value = strtoull(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' ||
        value > UINT64_MAX) {
        fprintf(stderr, "error: invalid %s: %s\n", name, text);
        exit(EXIT_FAILURE);
    }
    return (uint64_t)value;
}

static unsigned default_thread_count(void)
{
#if defined(__APPLE__)
    int logical_cpus = 1;
    size_t size = sizeof(logical_cpus);
    if (sysctlbyname("hw.logicalcpu", &logical_cpus, &size, NULL, 0) != 0 ||
        logical_cpus < 1) {
        return 1;
    }
    long n = logical_cpus;
#else
    long n = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    if (n < 1) {
        return 1;
    }
    if (n > (long)MAX_THREADS) {
        return MAX_THREADS;
    }
    return (unsigned)n;
}

static uint64_t isqrt_u64(uint64_t n)
{
    uint64_t r = (uint64_t)sqrtl((long double)n);

    while (r != 0 && r > n / r) {
        --r;
    }
    while (r < UINT32_MAX && r + 1 <= n / (r + 1)) {
        ++r;
    }
    return r;
}

/* All odd primes <= limit. */
static PrimeTable make_prime_table(uint64_t limit)
{
    PrimeTable table = {NULL, 0};
    if (limit < 3) {
        return table;
    }

    const uint64_t odd_count64 = (limit - 1) / 2;
    if (odd_count64 > SIZE_MAX) {
        die("base-prime sieve is too large for this machine");
    }
    const size_t odd_count = (size_t)odd_count64;
    uint8_t *composite = xcalloc(odd_count, sizeof(*composite));

    for (size_t i = 0; i < odd_count; ++i) {
        const uint64_t p = 2 * (uint64_t)i + 3;
        if (p > limit / p) {
            break;
        }
        if (composite[i] != 0) {
            continue;
        }
        const uint64_t first = (p * p - 3) / 2;
        for (uint64_t j = first; j < odd_count64; j += p) {
            composite[(size_t)j] = 1;
        }
    }

    size_t count = 0;
    for (size_t i = 0; i < odd_count; ++i) {
        count += composite[i] == 0;
    }
    table.values = xmalloc(count * sizeof(*table.values));
    table.count = count;

    size_t out = 0;
    for (size_t i = 0; i < odd_count; ++i) {
        if (composite[i] == 0) {
            table.values[out++] = (uint32_t)(2 * (uint64_t)i + 3);
        }
    }
    free(composite);
    return table;
}

static void free_prime_table(PrimeTable *table)
{
    free(table->values);
    table->values = NULL;
    table->count = 0;
}

static void pow_context_init(PowContext *ctx)
{
    mpz_init_set_ui(ctx->base, 2);
    mpz_init(ctx->p);
    mpz_init(ctx->modulus);
    mpz_init(ctx->residue);
    mpz_init(ctx->target);
}

static void pow_context_clear(PowContext *ctx)
{
    mpz_clear(ctx->base);
    mpz_clear(ctx->p);
    mpz_clear(ctx->modulus);
    mpz_clear(ctx->residue);
    mpz_clear(ctx->target);
}

#if defined(__SIZEOF_INT128__)
static uint64_t mul_mod_u64(uint64_t a, uint64_t b, uint64_t modulus)
{
    return (uint64_t)(((__uint128_t)a * b) % modulus);
}

static uint64_t pow_mod_u64(uint64_t base, uint64_t exponent,
                            uint64_t modulus)
{
    uint64_t result = 1;
    while (exponent != 0) {
        if ((exponent & 1) != 0) {
            result = mul_mod_u64(result, base, modulus);
        }
        exponent >>= 1;
        if (exponent != 0) {
            base = mul_mod_u64(base, base, modulus);
        }
    }
    return result;
}
#endif

static bool is_a125854(uint64_t p, PowContext *ctx)
{
#if defined(__SIZEOF_INT128__)
    if (p <= UINT32_MAX) {
        const uint64_t modulus = p * p;
        return pow_mod_u64(2, p - 1, modulus) == p + 1;
    }
#endif

    mpz_set_ui(ctx->p, (unsigned long)p);
    mpz_mul(ctx->modulus, ctx->p, ctx->p);
    mpz_powm_ui(ctx->residue, ctx->base, (unsigned long)(p - 1),
                ctx->modulus);
    mpz_add_ui(ctx->target, ctx->p, 1);
    return mpz_cmp(ctx->residue, ctx->target) == 0;
}

static void sieve_segment(uint8_t *composite, uint64_t low, uint64_t count,
                          const PrimeTable *base_primes)
{
    memset(composite, 0, (size_t)count);
    const uint64_t high = low + 2 * (count - 1);

    for (size_t i = 0; i < base_primes->count; ++i) {
        const uint64_t p = base_primes->values[i];
        if (p > high / p) {
            break;
        }

        uint64_t first;
        const uint64_t p2 = p * p;
        if (p2 >= low) {
            first = p2;
        } else {
            const uint64_t remainder = low % p;
            const uint64_t delta = remainder == 0 ? 0 : p - remainder;
            if (delta > UINT64_MAX - low) {
                continue;
            }
            first = low + delta;
            if ((first & 1) == 0) {
                if (p > UINT64_MAX - first) {
                    continue;
                }
                first += p;
            }
        }

        if (first > high) {
            continue;
        }
        uint64_t j = (first - low) / 2;
        while (j < count) {
            composite[(size_t)j] = 1;
            if (p > UINT64_MAX - j) {
                break;
            }
            j += p;
        }
    }
}

static void record_match(SearchJob *job, uint64_t p)
{
    pthread_mutex_lock(&job->output_lock);

    if (job->match_count == job->match_capacity) {
        size_t new_capacity = job->match_capacity == 0
                                  ? 8
                                  : 2 * job->match_capacity;
        uint64_t *new_matches =
            realloc(job->matches, new_capacity * sizeof(*new_matches));
        if (new_matches == NULL) {
            job->failed = true;
            pthread_mutex_unlock(&job->output_lock);
            return;
        }
        job->matches = new_matches;
        job->match_capacity = new_capacity;
    }
    job->matches[job->match_count++] = p;

    if (job->print_matches) {
        printf("%" PRIu64 "\n", p);
        fflush(stdout);
    }
    pthread_mutex_unlock(&job->output_lock);
}

static void maybe_report_progress(SearchJob *job, uint64_t completed)
{
    if (!job->show_progress) {
        return;
    }

    const double now = monotonic_seconds();
    pthread_mutex_lock(&job->output_lock);
    if (completed == job->segment_count || now - job->last_report_at >= 5.0) {
        job->last_report_at = now;
        const uint64_t tested = atomic_load_explicit(
            &job->primes_tested, memory_order_relaxed);
        const double elapsed = now - job->started_at;
        const double percent = job->segment_count == 0
                                   ? 100.0
                                   : 100.0 * (double)completed /
                                         (double)job->segment_count;
        fprintf(stderr,
                "%6.2f%%  segments=%" PRIu64 "/%" PRIu64
                "  primes=%" PRIu64 "  elapsed=%.1fs\n",
                percent, completed, job->segment_count, tested, elapsed);
        fflush(stderr);
    }
    pthread_mutex_unlock(&job->output_lock);
}

static void *search_worker(void *argument)
{
    SearchJob *job = argument;
    uint8_t *composite = xmalloc((size_t)job->segment_odds);
    PowContext pow_ctx;
    pow_context_init(&pow_ctx);

    for (;;) {
        const uint64_t segment = atomic_fetch_add_explicit(
            &job->next_segment, 1, memory_order_relaxed);
        if (segment >= job->segment_count) {
            break;
        }

        const uint64_t offset = segment * job->segment_odds;
        const uint64_t remaining = job->odd_count - offset;
        const uint64_t count = remaining < job->segment_odds
                                   ? remaining
                                   : job->segment_odds;
        const uint64_t low = job->first_odd + 2 * offset;

        sieve_segment(composite, low, count, job->base_primes);

        uint64_t local_prime_count = 0;
        for (uint64_t i = 0; i < count; ++i) {
            if (composite[(size_t)i] != 0) {
                continue;
            }
            const uint64_t p = low + 2 * i;
            ++local_prime_count;
            if (is_a125854(p, &pow_ctx)) {
                record_match(job, p);
            }
        }
        atomic_fetch_add_explicit(&job->primes_tested, local_prime_count,
                                  memory_order_relaxed);
        const uint64_t completed = atomic_fetch_add_explicit(
                                       &job->completed_segments, 1,
                                       memory_order_relaxed) +
                                   1;
        maybe_report_progress(job, completed);
    }

    pow_context_clear(&pow_ctx);
    free(composite);
    return NULL;
}

static int compare_u64(const void *left, const void *right)
{
    const uint64_t a = *(const uint64_t *)left;
    const uint64_t b = *(const uint64_t *)right;
    return (a > b) - (a < b);
}

static uint64_t *search_interval(uint64_t start, uint64_t end,
                                 unsigned threads, uint64_t segment_odds,
                                 bool print_matches, bool show_progress,
                                 size_t *match_count_out,
                                 uint64_t *primes_tested_out)
{
    *match_count_out = 0;
    *primes_tested_out = 0;
    if (start > end || end < 3) {
        return NULL;
    }
    if (start < 3) {
        start = 3;
    }
    if ((start & 1) == 0) {
        ++start;
    }
    if ((end & 1) == 0) {
        --end;
    }
    if (start > end) {
        return NULL;
    }

    const uint64_t odd_count = (end - start) / 2 + 1;
    const uint64_t segment_count =
        odd_count / segment_odds + (odd_count % segment_odds != 0);
    if (threads > segment_count) {
        threads = (unsigned)segment_count;
    }
    if (threads == 0) {
        threads = 1;
    }

    const uint64_t root = isqrt_u64(end);
    const double sieve_started = monotonic_seconds();
    PrimeTable base_primes = make_prime_table(root);
    if (show_progress) {
        fprintf(stderr,
                "range=[%" PRIu64 ", %" PRIu64 "]  threads=%u"
                "  segment_odds=%" PRIu64 "\n",
                start, end, threads, segment_odds);
        fprintf(stderr,
                "base sieve: %zu odd primes <= %" PRIu64 " (%.3fs)\n",
                base_primes.count, root,
                monotonic_seconds() - sieve_started);
    }

    SearchJob job;
    memset(&job, 0, sizeof(job));
    job.first_odd = start;
    job.odd_count = odd_count;
    job.segment_odds = segment_odds;
    job.segment_count = segment_count;
    job.base_primes = &base_primes;
    job.print_matches = print_matches;
    job.show_progress = show_progress;
    job.started_at = monotonic_seconds();
    job.last_report_at = job.started_at;
    atomic_init(&job.next_segment, 0);
    atomic_init(&job.completed_segments, 0);
    atomic_init(&job.primes_tested, 0);
    if (pthread_mutex_init(&job.output_lock, NULL) != 0) {
        die("pthread_mutex_init failed");
    }

    pthread_t *thread_ids = xmalloc(threads * sizeof(*thread_ids));
    unsigned created = 0;
    for (; created < threads; ++created) {
        if (pthread_create(&thread_ids[created], NULL, search_worker, &job) !=
            0) {
            job.failed = true;
            break;
        }
    }
    for (unsigned i = 0; i < created; ++i) {
        if (pthread_join(thread_ids[i], NULL) != 0) {
            job.failed = true;
        }
    }
    free(thread_ids);

    qsort(job.matches, job.match_count, sizeof(*job.matches), compare_u64);
    *match_count_out = job.match_count;
    *primes_tested_out = atomic_load_explicit(&job.primes_tested,
                                               memory_order_relaxed);

    pthread_mutex_destroy(&job.output_lock);
    free_prime_table(&base_primes);
    if (job.failed) {
        free(job.matches);
        die("parallel search failed");
    }
    return job.matches;
}

static int self_test(void)
{
    static const uint64_t known[] = {
        UINT64_C(3), UINT64_C(29), UINT64_C(37), UINT64_C(3373),
        UINT64_C(2001907169),
    };
    static const uint64_t nonmembers[] = {
        UINT64_C(5), UINT64_C(7), UINT64_C(11), UINT64_C(31),
        UINT64_C(1093), UINT64_C(3511), UINT64_C(4294967311),
    };

    PowContext ctx;
    pow_context_init(&ctx);
    for (size_t i = 0; i < sizeof(known) / sizeof(known[0]); ++i) {
        if (!is_a125854(known[i], &ctx)) {
            fprintf(stderr, "self-test failed: known term %" PRIu64 "\n",
                    known[i]);
            pow_context_clear(&ctx);
            return EXIT_FAILURE;
        }
    }
    for (size_t i = 0; i < sizeof(nonmembers) / sizeof(nonmembers[0]); ++i) {
        if (is_a125854(nonmembers[i], &ctx)) {
            fprintf(stderr, "self-test failed: nonmember %" PRIu64 "\n",
                    nonmembers[i]);
            pow_context_clear(&ctx);
            return EXIT_FAILURE;
        }
    }
    pow_context_clear(&ctx);

    size_t count = 0;
    uint64_t tested = 0;
    uint64_t *found = search_interval(3, 5000, 2, 128, false, false,
                                      &count, &tested);
    static const uint64_t expected_small[] = {
        UINT64_C(3), UINT64_C(29), UINT64_C(37), UINT64_C(3373),
    };
    const size_t expected_count =
        sizeof(expected_small) / sizeof(expected_small[0]);
    bool ok = count == expected_count;
    for (size_t i = 0; ok && i < expected_count; ++i) {
        ok = found[i] == expected_small[i];
    }
    free(found);

    if (!ok || tested != 668) {
        fprintf(stderr,
                "self-test failed: segmented search found %zu matches and "
                "tested %" PRIu64 " primes\n",
                count, tested);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "self-test passed (five known terms; sieve through 5000)\n");
    return EXIT_SUCCESS;
}

static void print_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s START END [THREADS [SEGMENT_ODDS]]\n"
            "       %s --self-test\n",
            program, program);
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--self-test") == 0) {
        return self_test();
    }
    if (argc < 3 || argc > 5) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const uint64_t start = parse_u64(argv[1], "START");
    const uint64_t end = parse_u64(argv[2], "END");
    if (start > end) {
        die("START must not exceed END");
    }

    unsigned threads = default_thread_count();
    if (argc >= 4) {
        const uint64_t parsed = parse_u64(argv[3], "THREADS");
        if (parsed == 0 || parsed > MAX_THREADS) {
            die("THREADS must be in 1..1024");
        }
        threads = (unsigned)parsed;
    }

    uint64_t segment_odds = DEFAULT_SEGMENT_ODDS;
    if (argc >= 5) {
        segment_odds = parse_u64(argv[4], "SEGMENT_ODDS");
        if (segment_odds == 0 || segment_odds > SIZE_MAX) {
            die("SEGMENT_ODDS is too large for this machine");
        }
    }

    const double started = monotonic_seconds();
    size_t match_count = 0;
    uint64_t primes_tested = 0;
    uint64_t *matches = search_interval(start, end, threads, segment_odds,
                                        true, true, &match_count,
                                        &primes_tested);
    free(matches);

    fprintf(stderr,
            "done: tested %" PRIu64 " primes, found %zu match%s in %.3fs\n",
            primes_tested, match_count, match_count == 1 ? "" : "es",
            monotonic_seconds() - started);
    return EXIT_SUCCESS;
}
