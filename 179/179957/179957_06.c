/*
 * Minimum-jump permutations by a proved scalar holonomic recurrence.
 * A179957 is the k=4 specialization.
 *
 * We count permutations p of [n] satisfying
 *
 *                 |p(i)-p(i-1)| >= k  (i>1).
 *
 * This program supports k=2,3,4.  Unlike 03--05, its runtime state is only
 * the final scalar sequence: no frontier state vector and no polynomial
 * Q_n(x) is advanced.  The recurrence is NOT a numerical guess.
 *
 * Mathematical proof
 * ------------------
 * Put an edge {u,v} in G_{n,k} when 0<|u-v|<k.  Inclusion-exclusion over
 * forbidden adjacencies leaves only edge sets which are linear forests.
 * If
 *
 *   Q_{n,k}(x) = Sum_F 2^c(F) x^|F|,
 *
 * where F runs over linear forests in G_{n,k} and c(F) is the number of
 * nontrivial path components, then orienting every path component gives
 *
 *   a_k(n) = Sum_j (-1)^j [x^j] Q_{n,k}(x) (n-j)!.              (1)
 *
 * For fixed k, the width-(k-1) transfer calculation for these forests gives
 * an exact rational ordinary generating function
 *
 *   Q_k(z,x) = Sum_n Q_{n,k}(x) z^n = N_k(z,x)/D_k(z,x).
 *
 * Therefore the ordinary generating function A_k(z)=Sum_n a_k(n)z^n is
 * the formal Laplace integral
 *
 *   A_k(z) = Integral_0^infinity exp(-t)
 *              Q_k(t*z,-1/t) dt.                              (2)
 *
 * Expanding (2) proves (1), since Integral exp(-t)t^(n-j)dt=(n-j)!.
 * Continuous Almkvist-Zeilberger telescoping, performed over QQ(z,t),
 * produced an exact rational certificate C_k(z,t) satisfying
 *
 *   (Sum_i B_{k,i}(z) d_z^i) H_k(z,t)
 *       = d_t(C_k(z,t) H_k(z,t)),
 *   H_k = exp(-t) Q_k(t*z,-1/t).
 *
 * The certificate identity was verified after clearing denominators as a
 * polynomial identity over QQ[z,t].  At t=infinity the boundary is zero;
 * the t=0 boundary is the rational function P_k(z)/R_k(z).  Hence
 *
 *   Sum_i B_{k,i}(z) A_k^(i)(z) = P_k(z)/R_k(z).               (3)
 *
 * The exact integer coefficients of B, P and R are in
 * 179957_06_data.inc.  The differential orders are 1, 2 and 5 for k=2,3,4.
 * After removal of common factors their operator degrees are respectively
 * (4,5), (28,29,29), and (300,301,301,300,299,298).
 *
 * Write B_i(z)=Sum_j b[i,j]z^j and P/R=Sum_n r(n)z^n.  Comparing
 * coefficients in (3) gives the recurrence used below:
 *
 *   R[0] r(n) = P[n] - Sum_{j>=1} R[j] r(n-j),
 *
 *   b[0,0] a(n) = r(n)
 *       - Sum_{(i,j)!=(0,0)} b[i,j]
 *           (n-j+1)(n-j+2)...(n-j+i) a(n-j+i).                (4)
 *
 * The z-valuations of the B_i are
 *   k=2: (0,2), k=3: (0,2,5), k=4: (0,2,6,7,11,15).
 * Thus every term on the right of (4) has index < n.  Equation (4), its
 * rational boundary recurrence, and the exact divisibility checks in this
 * program prove every computed value.  No fitted or guessed recurrence is
 * used.  --check additionally compares with an independent subset DP
 * through n=16 and with fixed known prefixes.
 *
 * The underlying generalized-rook/linear-forest argument is described by
 * I. Gessel, "Generalized Rook Polynomials and Orthogonal Polynomials".  The
 * continuous creative-telescoping step follows the exact AZ implementation
 * used by G. Spahn and D. Zeilberger in ResPerms (2022).
 *
 * Complexity for fixed k: O(C_k*N) GMP add-multiplications and O(L_k)
 * big integers, where C_k is the stored operator size and L_k is its maximum
 * lag (4, 28, 300 for k=2,3,4).  In particular, it is polynomial in N.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     179957_06.c -lgmp -o 179957_06
 *
 * Examples:
 *   ./179957_06 --k 4 --upto 500
 *   ./179957_06 --k 3 --term 500
 *   ./179957_06 --k 2 --check 100
 *
 * Range runs write b179957_06_kK.txt atomically via a _part.txt file.  To
 * follow OEIS b-file policy, output stops before the first value having more
 * than 1000 decimal digits; computation still continues through MAX_N.
 */

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

#define M6_DEFAULT_K 4
#define M6_MIN_K 2
#define M6_MAX_K 4
#define M6_DEFAULT_N 30
#define M6_DEFAULT_CHECK_N 100
#define M6_MAX_N 10000
#define M6_BFILE_MAX_DIGITS 1000UL
#define M6_DIRECT_CHECK_MAX_N 16

#define M6_ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

typedef struct {
    const char *const *coefficient;
    size_t count;
} M6SourcePolynomial;

typedef struct {
    int k;
    size_t operator_count;
    const M6SourcePolynomial *operator_polynomial;
    M6SourcePolynomial rhs_numerator;
    M6SourcePolynomial rhs_denominator;
} M6Source;

#define M6_SOURCE_POLYNOMIAL(name) { (name), M6_ARRAY_COUNT(name) }
#include "179957_06_data.inc"
#undef M6_SOURCE_POLYNOMIAL

typedef struct {
    mpz_t *coefficient;
    size_t count;
} M6Polynomial;

typedef struct {
    int k;
    size_t operator_count;
    M6Polynomial *operator_polynomial;
    M6Polynomial rhs_numerator;
    M6Polynomial rhs_denominator;
    size_t answer_lag;
    size_t nonzero_operator_terms;
} M6Recurrence;

typedef struct {
    uint64_t rhs_addmuls;
    uint64_t answer_addmuls;
    size_t answer_lag;
    size_t rhs_lag;
    double seconds;
} M6Stats;

typedef enum {
    M6_RUN_UPTO,
    M6_RUN_TERM,
    M6_RUN_CHECK
} M6RunMode;

static void m6_die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static void *m6_xcalloc(size_t count, size_t size)
{
    if (size != 0U && count > SIZE_MAX / size) {
        m6_die("allocation size overflow");
    }
    void *pointer = calloc(count, size);
    if (pointer == NULL && count != 0U) {
        m6_die("out of memory");
    }
    return pointer;
}

static mpz_t *m6_mpz_array_create(size_t count)
{
    mpz_t *array = m6_xcalloc(count, sizeof(*array));
    for (size_t i = 0U; i < count; ++i) {
        mpz_init(array[i]);
    }
    return array;
}

static void m6_mpz_array_destroy(mpz_t *array, size_t count)
{
    if (array == NULL) {
        return;
    }
    for (size_t i = 0U; i < count; ++i) {
        mpz_clear(array[i]);
    }
    free(array);
}

static int m6_parse_integer(const char *text, const char *name,
                            int minimum, int maximum)
{
    if (text == NULL || *text == '\0') {
        m6_die("missing integer argument");
    }
    errno = 0;
    char *past = NULL;
    long value = strtol(text, &past, 10);
    if (errno != 0 || past == text || *past != '\0' ||
        value < minimum || value > maximum) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                name, minimum, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static const M6Source *m6_find_source(int k)
{
    for (size_t i = 0U; i < M6_ARRAY_COUNT(m6_source_table); ++i) {
        if (m6_source_table[i].k == k) {
            return &m6_source_table[i];
        }
    }
    m6_die("proved scalar recurrence is available only for k=2,3,4");
    return NULL;
}

static void m6_polynomial_init(M6Polynomial *destination,
                               const M6SourcePolynomial *source)
{
    destination->count = source->count;
    destination->coefficient = m6_mpz_array_create(source->count);
    for (size_t i = 0U; i < source->count; ++i) {
        if (mpz_set_str(destination->coefficient[i],
                        source->coefficient[i], 10) != 0) {
            m6_die("invalid embedded recurrence coefficient");
        }
    }
}

static void m6_polynomial_destroy(M6Polynomial *polynomial)
{
    m6_mpz_array_destroy(polynomial->coefficient, polynomial->count);
    polynomial->coefficient = NULL;
    polynomial->count = 0U;
}

static void m6_recurrence_init(M6Recurrence *recurrence, int k)
{
    memset(recurrence, 0, sizeof(*recurrence));
    const M6Source *source = m6_find_source(k);
    recurrence->k = k;
    recurrence->operator_count = source->operator_count;
    recurrence->operator_polynomial = m6_xcalloc(
        source->operator_count, sizeof(*recurrence->operator_polynomial));
    for (size_t derivative = 0U;
         derivative < source->operator_count; ++derivative) {
        M6Polynomial *polynomial =
            &recurrence->operator_polynomial[derivative];
        m6_polynomial_init(polynomial,
                           &source->operator_polynomial[derivative]);
        for (size_t power = 0U; power < polynomial->count; ++power) {
            if (mpz_sgn(polynomial->coefficient[power]) == 0) {
                continue;
            }
            ++recurrence->nonzero_operator_terms;
            if (derivative == 0U && power == 0U) {
                continue;
            }
            if (power <= derivative) {
                m6_die("embedded differential equation is not causal");
            }
            size_t lag = power - derivative;
            if (lag > recurrence->answer_lag) {
                recurrence->answer_lag = lag;
            }
        }
    }
    m6_polynomial_init(&recurrence->rhs_numerator,
                       &source->rhs_numerator);
    m6_polynomial_init(&recurrence->rhs_denominator,
                       &source->rhs_denominator);
    if (recurrence->operator_count == 0U ||
        recurrence->operator_polynomial[0].count == 0U ||
        mpz_sgn(recurrence->operator_polynomial[0].coefficient[0]) == 0) {
        m6_die("embedded differential equation has zero leading term");
    }
    if (recurrence->rhs_denominator.count == 0U ||
        mpz_sgn(recurrence->rhs_denominator.coefficient[0]) == 0) {
        m6_die("embedded boundary denominator has zero constant term");
    }
    if (recurrence->answer_lag == 0U) {
        m6_die("embedded recurrence has no backward dependency");
    }
}

static void m6_recurrence_destroy(M6Recurrence *recurrence)
{
    for (size_t derivative = 0U;
         derivative < recurrence->operator_count; ++derivative) {
        m6_polynomial_destroy(
            &recurrence->operator_polynomial[derivative]);
    }
    free(recurrence->operator_polynomial);
    m6_polynomial_destroy(&recurrence->rhs_numerator);
    m6_polynomial_destroy(&recurrence->rhs_denominator);
    memset(recurrence, 0, sizeof(*recurrence));
}

static void m6_increment(uint64_t *counter, const char *name)
{
    if (*counter == UINT64_MAX) {
        fprintf(stderr, "error: %s counter overflow\n", name);
        exit(EXIT_FAILURE);
    }
    ++*counter;
}

static const char *const m6_known_k2[] = {
    "1", "1", "0", "0", "2", "14", "90", "646", "5242",
    "47622", "479306", "5296790", "63779034", "831283558",
    "11661506218", "175203184374", "2806878055610",
    "47767457130566", "860568917787402", "16362838542699862",
    "327460573946510746", "6880329406055690790"
};

static const char *const m6_known_k3[] = {
    "1", "1", "0", "0", "0", "0", "2", "32", "368", "3984",
    "44304", "521606", "6564318", "88422296", "1272704694",
    "19521035238", "318120059458", "5491779703870",
    "100150978723568", "1924351621839740", "38864316540425434",
    "823161467837784388"
};

static const char *const m6_known_k4[] = {
    "1", "1", "0", "0", "0", "0", "0", "0", "2", "72",
    "1496", "25384", "399848", "6231544", "99133496",
    "1634227958", "28120703888", "506599465896", "9562489659952",
    "189055933191880", "3911226769004280", "84567966329357056"
};

static const char *const *m6_known_values(int k, size_t *count)
{
    if (k == 2) {
        *count = M6_ARRAY_COUNT(m6_known_k2);
        return m6_known_k2;
    }
    if (k == 3) {
        *count = M6_ARRAY_COUNT(m6_known_k3);
        return m6_known_k3;
    }
    *count = M6_ARRAY_COUNT(m6_known_k4);
    return m6_known_k4;
}

static double m6_elapsed_seconds(struct timespec start,
                                 struct timespec finish)
{
    return (double)(finish.tv_sec - start.tv_sec) +
           (double)(finish.tv_nsec - start.tv_nsec) / 1000000000.0;
}

static void m6_compute(const M6Recurrence *recurrence, int maximum_n,
                       FILE *bfile, mpz_t final_value,
                       mpz_t *check_prefix, size_t check_prefix_count,
                       int *written_maximum_n, M6Stats *stats)
{
    memset(stats, 0, sizeof(*stats));
    stats->answer_lag = recurrence->answer_lag;
    stats->rhs_lag = recurrence->rhs_denominator.count - 1U;
    *written_maximum_n = -1;

    size_t answer_ring_count = recurrence->answer_lag + 1U;
    size_t rhs_ring_count = recurrence->rhs_denominator.count;
    mpz_t *answer_ring = m6_mpz_array_create(answer_ring_count);
    mpz_t *rhs_ring = m6_mpz_array_create(rhs_ring_count);
    mpz_t accumulator;
    mpz_t multiplier;
    mpz_t first_excluded_value;
    mpz_t expected;
    mpz_inits(accumulator, multiplier, first_excluded_value, expected, NULL);
    mpz_ui_pow_ui(first_excluded_value, 10UL, M6_BFILE_MAX_DIGITS);

    size_t known_count = 0U;
    const char *const *known = m6_known_values(recurrence->k,
                                               &known_count);
    bool bfile_open_for_values = bfile != NULL;
    struct timespec start;
    struct timespec finish;
    (void)timespec_get(&start, TIME_UTC);

    for (int n = 0; n <= maximum_n; ++n) {
        size_t rhs_slot = (size_t)n % rhs_ring_count;
        if ((size_t)n < recurrence->rhs_numerator.count) {
            mpz_set(rhs_ring[rhs_slot],
                    recurrence->rhs_numerator.coefficient[n]);
        } else {
            mpz_set_ui(rhs_ring[rhs_slot], 0UL);
        }
        size_t rhs_limit = (size_t)n < recurrence->rhs_denominator.count - 1U
                               ? (size_t)n
                               : recurrence->rhs_denominator.count - 1U;
        for (size_t lag = 1U; lag <= rhs_limit; ++lag) {
            mpz_srcptr denominator_coefficient =
                recurrence->rhs_denominator.coefficient[lag];
            if (mpz_sgn(denominator_coefficient) == 0) {
                continue;
            }
            size_t previous_slot = ((size_t)n - lag) % rhs_ring_count;
            mpz_submul(rhs_ring[rhs_slot], rhs_ring[previous_slot],
                       denominator_coefficient);
            m6_increment(&stats->rhs_addmuls, "boundary operation");
        }
        mpz_srcptr rhs_constant =
            recurrence->rhs_denominator.coefficient[0];
        if (!mpz_divisible_p(rhs_ring[rhs_slot], rhs_constant)) {
            m6_die("boundary recurrence lost exact divisibility");
        }
        mpz_divexact(rhs_ring[rhs_slot], rhs_ring[rhs_slot], rhs_constant);

        mpz_set(accumulator, rhs_ring[rhs_slot]);
        for (size_t derivative = 0U;
             derivative < recurrence->operator_count; ++derivative) {
            const M6Polynomial *polynomial =
                &recurrence->operator_polynomial[derivative];
            size_t power_limit = (size_t)n < polynomial->count - 1U
                                     ? (size_t)n
                                     : polynomial->count - 1U;
            for (size_t power = 0U; power <= power_limit; ++power) {
                mpz_srcptr coefficient = polynomial->coefficient[power];
                if (mpz_sgn(coefficient) == 0 ||
                    (derivative == 0U && power == 0U)) {
                    continue;
                }
                if (power <= derivative) {
                    m6_die("noncausal recurrence term reached at runtime");
                }
                size_t lag = power - derivative;
                if (lag > (size_t)n) {
                    continue;
                }
                mpz_set(multiplier, coefficient);
                for (size_t factor = 1U;
                     factor <= derivative; ++factor) {
                    unsigned long value =
                        (unsigned long)((size_t)n - power + factor);
                    mpz_mul_ui(multiplier, multiplier, value);
                }
                size_t previous_slot = ((size_t)n - lag) %
                                       answer_ring_count;
                mpz_submul(accumulator, answer_ring[previous_slot],
                           multiplier);
                m6_increment(&stats->answer_addmuls,
                             "scalar recurrence operation");
            }
        }
        mpz_srcptr leading =
            recurrence->operator_polynomial[0].coefficient[0];
        if (!mpz_divisible_p(accumulator, leading)) {
            fprintf(stderr,
                    "error: scalar recurrence is not exactly divisible "
                    "at k=%d, n=%d\n", recurrence->k, n);
            exit(EXIT_FAILURE);
        }
        size_t answer_slot = (size_t)n % answer_ring_count;
        mpz_divexact(answer_ring[answer_slot], accumulator, leading);
        if (mpz_sgn(answer_ring[answer_slot]) < 0) {
            fprintf(stderr,
                    "error: negative permutation count at k=%d, n=%d\n",
                    recurrence->k, n);
            exit(EXIT_FAILURE);
        }

        if ((size_t)n < known_count) {
            if (mpz_set_str(expected, known[n], 10) != 0 ||
                mpz_cmp(answer_ring[answer_slot], expected) != 0) {
                fprintf(stderr,
                        "error: known-prefix mismatch at k=%d, n=%d\n",
                        recurrence->k, n);
                exit(EXIT_FAILURE);
            }
        }
        if ((size_t)n < check_prefix_count) {
            mpz_set(check_prefix[n], answer_ring[answer_slot]);
        }
        if (bfile_open_for_values) {
            if (mpz_cmpabs(answer_ring[answer_slot],
                           first_excluded_value) >= 0) {
                bfile_open_for_values = false;
            } else {
                if (gmp_fprintf(bfile, "%d %Zd\n", n,
                                answer_ring[answer_slot]) < 0) {
                    m6_die("could not write b-file value");
                }
                *written_maximum_n = n;
            }
        }
        if (n == maximum_n) {
            mpz_set(final_value, answer_ring[answer_slot]);
        }
    }

    (void)timespec_get(&finish, TIME_UTC);
    stats->seconds = m6_elapsed_seconds(start, finish);
    mpz_clears(accumulator, multiplier, first_excluded_value, expected, NULL);
    m6_mpz_array_destroy(answer_ring, answer_ring_count);
    m6_mpz_array_destroy(rhs_ring, rhs_ring_count);
}

static uint64_t m6_direct_count(int n, int k)
{
    if (n == 0) {
        return UINT64_C(1);
    }
    if (n < 0 || n > M6_DIRECT_CHECK_MAX_N) {
        m6_die("independent DP called outside its proved safe range");
    }
    size_t mask_count = (size_t)1U << (unsigned)n;
    if ((size_t)n > SIZE_MAX / mask_count ||
        mask_count * (size_t)n > SIZE_MAX / sizeof(uint64_t)) {
        m6_die("independent DP allocation overflow");
    }
    size_t slot_count = mask_count * (size_t)n;
    uint64_t *dp = m6_xcalloc(slot_count, sizeof(*dp));
    for (int value = 0; value < n; ++value) {
        size_t mask = (size_t)1U << (unsigned)value;
        dp[mask * (size_t)n + (size_t)value] = UINT64_C(1);
    }
    for (size_t mask = 1U; mask < mask_count; ++mask) {
        for (int last = 0; last < n; ++last) {
            uint64_t current = dp[mask * (size_t)n + (size_t)last];
            if (current == 0U) {
                continue;
            }
            for (int next = 0; next < n; ++next) {
                size_t bit = (size_t)1U << (unsigned)next;
                int difference = last > next ? last - next : next - last;
                if ((mask & bit) != 0U || difference < k) {
                    continue;
                }
                size_t destination = (mask | bit) * (size_t)n +
                                     (size_t)next;
                if (UINT64_MAX - dp[destination] < current) {
                    free(dp);
                    m6_die("independent DP counter overflow");
                }
                dp[destination] += current;
            }
        }
    }
    size_t full = mask_count - 1U;
    uint64_t answer = UINT64_C(0);
    for (int last = 0; last < n; ++last) {
        uint64_t addend = dp[full * (size_t)n + (size_t)last];
        if (UINT64_MAX - answer < addend) {
            free(dp);
            m6_die("independent DP final counter overflow");
        }
        answer += addend;
    }
    free(dp);
    return answer;
}

static char *m6_output_path(const char *program, int k, bool partial)
{
    const char *slash = strrchr(program, '/');
    size_t directory_length = slash == NULL ? 0U :
                              (size_t)(slash - program + 1);
    const char *suffix = partial ? "_part.txt" : ".txt";
    int needed = snprintf(NULL, 0, "b179957_06_k%d%s", k, suffix);
    if (needed < 0) {
        m6_die("could not form output filename");
    }
    size_t filename_length = (size_t)needed;
    if (directory_length > SIZE_MAX - filename_length - 1U) {
        m6_die("output path length overflow");
    }
    char *path = m6_xcalloc(directory_length + filename_length + 1U,
                            sizeof(*path));
    if (directory_length != 0U) {
        memcpy(path, program, directory_length);
    }
    (void)snprintf(path + directory_length, filename_length + 1U,
                   "b179957_06_k%d%s", k, suffix);
    return path;
}

static void m6_print_stats(int maximum_n, const M6Recurrence *recurrence,
                           const M6Stats *stats)
{
    fprintf(stderr,
            "179957_06: k=%d, n=0..%d, proved scalar AZ recurrence, "
            "differential order=%zu, answer lag=%zu, rhs lag=%zu, "
            "operator addmuls=%" PRIu64 ", rhs addmuls=%" PRIu64
            ", %.3f s\n",
            recurrence->k, maximum_n, recurrence->operator_count - 1U,
            stats->answer_lag, stats->rhs_lag, stats->answer_addmuls,
            stats->rhs_addmuls, stats->seconds);
}

static int m6_check(int maximum_n, int k)
{
    M6Recurrence recurrence;
    m6_recurrence_init(&recurrence, k);
    int direct_maximum = maximum_n < M6_DIRECT_CHECK_MAX_N
                             ? maximum_n : M6_DIRECT_CHECK_MAX_N;
    mpz_t *prefix = m6_mpz_array_create((size_t)direct_maximum + 1U);
    mpz_t final_value;
    mpz_init(final_value);
    M6Stats stats;
    int ignored_written_maximum = -1;
    m6_compute(&recurrence, maximum_n, NULL, final_value, prefix,
               (size_t)direct_maximum + 1U, &ignored_written_maximum,
               &stats);
    for (int n = 0; n <= direct_maximum; ++n) {
        uint64_t direct = m6_direct_count(n, k);
        if (mpz_cmp_ui(prefix[n], (unsigned long)direct) != 0) {
            gmp_fprintf(stderr,
                        "error: independent DP mismatch at k=%d, n=%d: "
                        "recurrence=%Zd, direct=%" PRIu64 "\n",
                        k, n, prefix[n], direct);
            mpz_clear(final_value);
            m6_mpz_array_destroy(prefix,
                                 (size_t)direct_maximum + 1U);
            m6_recurrence_destroy(&recurrence);
            return EXIT_FAILURE;
        }
    }
    m6_print_stats(maximum_n, &recurrence, &stats);
    printf("ok: k=%d scalar recurrence is exactly divisible through n=%d; "
           "known prefix n=0..%d and independent subset DP n=0..%d "
           "verified\n",
           k, maximum_n, maximum_n < 21 ? maximum_n : 21,
           direct_maximum);
    mpz_clear(final_value);
    m6_mpz_array_destroy(prefix, (size_t)direct_maximum + 1U);
    m6_recurrence_destroy(&recurrence);
    return EXIT_SUCCESS;
}

static void m6_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K]\n"
            "       %s --upto MAX_N [--k K]\n"
            "       %s --term N [--k K]\n"
            "       %s --check [MAX_N] [--k K]\n"
            "Proved scalar recurrences are available for K=2,3,4; "
            "the default is 4; N is 0..%d.\n"
            "A range run writes b179957_06_kK.txt, stopping before the "
            "first term over %lu digits.\n",
            program, program, program, program, M6_MAX_N,
            M6_BFILE_MAX_DIGITS);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 ||
         strcmp(argv[1], "-h") == 0)) {
        m6_usage(argv[0]);
        return EXIT_SUCCESS;
    }
    M6RunMode mode = M6_RUN_UPTO;
    int maximum_n = -1;
    int k = M6_DEFAULT_K;
    bool have_n = false;
    bool have_k = false;
    bool have_mode = false;
    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--k") == 0) {
            if (have_k || argument + 1 >= argc) {
                m6_usage(argv[0]);
                return EXIT_FAILURE;
            }
            k = m6_parse_integer(argv[++argument], "K",
                                 M6_MIN_K, M6_MAX_K);
            have_k = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                m6_usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = strcmp(text, "--term") == 0
                       ? M6_RUN_TERM : M6_RUN_UPTO;
            have_mode = true;
            maximum_n = m6_parse_integer(argv[++argument],
                                          mode == M6_RUN_TERM
                                              ? "N" : "MAX_N",
                                          0, M6_MAX_N);
            have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode || have_n) {
                m6_usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = M6_RUN_CHECK;
            have_mode = true;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                maximum_n = m6_parse_integer(argv[++argument], "CHECK_N",
                                              0, M6_MAX_N);
                have_n = true;
            }
        } else if (text[0] == '-' || have_n) {
            m6_usage(argv[0]);
            return EXIT_FAILURE;
        } else {
            maximum_n = m6_parse_integer(text,
                                          mode == M6_RUN_CHECK
                                              ? "CHECK_N" : "MAX_N",
                                          0, M6_MAX_N);
            have_n = true;
        }
    }
    if (!have_n) {
        maximum_n = mode == M6_RUN_CHECK
                        ? M6_DEFAULT_CHECK_N : M6_DEFAULT_N;
    }
    if (mode == M6_RUN_CHECK) {
        return m6_check(maximum_n, k);
    }

    M6Recurrence recurrence;
    m6_recurrence_init(&recurrence, k);
    char *final_path = NULL;
    char *partial_path = NULL;
    FILE *bfile = NULL;
    if (mode == M6_RUN_UPTO) {
        final_path = m6_output_path(argv[0], k, false);
        partial_path = m6_output_path(argv[0], k, true);
        bfile = fopen(partial_path, "w");
        if (bfile == NULL) {
            fprintf(stderr, "error: cannot open %s: %s\n",
                    partial_path, strerror(errno));
            free(final_path);
            free(partial_path);
            m6_recurrence_destroy(&recurrence);
            return EXIT_FAILURE;
        }
    }
    mpz_t final_value;
    mpz_init(final_value);
    M6Stats stats;
    int written_maximum_n = -1;
    m6_compute(&recurrence, maximum_n, bfile, final_value, NULL, 0U,
               &written_maximum_n, &stats);
    m6_print_stats(maximum_n, &recurrence, &stats);

    int result = EXIT_SUCCESS;
    if (mode == M6_RUN_TERM) {
        gmp_printf("%d %Zd\n", maximum_n, final_value);
    } else {
        if (fflush(bfile) != 0 || fclose(bfile) != 0) {
            fprintf(stderr, "error: cannot finish %s: %s\n",
                    partial_path, strerror(errno));
            result = EXIT_FAILURE;
        } else if (rename(partial_path, final_path) != 0) {
            fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                    partial_path, final_path, strerror(errno));
            result = EXIT_FAILURE;
        } else {
            printf("wrote %s (n=0..%d", final_path,
                   written_maximum_n);
            if (written_maximum_n < maximum_n) {
                printf("; requested through n=%d, later terms omitted "
                       "because the next term exceeds %lu decimal digits",
                       maximum_n, M6_BFILE_MAX_DIGITS);
            }
            printf(")\n");
        }
    }
    mpz_clear(final_value);
    free(final_path);
    free(partial_path);
    m6_recurrence_destroy(&recurrence);
    return result;
}
