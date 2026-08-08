/*
 * A179962 and the general fixed-k family -- proved C-finite recurrence.
 *
 * Define
 *
 *   A_k(n) = #{ permutations p of [2*n+k] :
 *                |p(i+1)-p(i)| > n for 1 <= i < 2*n+k }.
 *
 * This program computes A_k(n) in polynomial time in n for fixed k.  The
 * implementation accepts 2 <= k <= 8; the theorem below holds for every
 * fixed k >= 2.  The finite k limit is only the range for which this source
 * contains independently certified initial values.  It is not an n limit.
 *
 * THE RECURRENCE
 * --------------
 * Put lambda_r = r*(k-r), 1 <= r <= floor(k/2), and d=ceil(k/2).
 * Form
 *
 *   C_k(X) = Product_{r=1..floor(k/2)} (X-lambda_r)
 *             * (X-lambda_floor(k/2))^[k odd]
 *          = X^d + c_1 X^(d-1) + ... + c_d.                 (1)
 *
 * The second copy of the last factor is present only for odd k.  Then
 *
 *   A_k(n) + c_1 A_k(n-1) + ... + c_d A_k(n-d) = 0          (2)
 *
 * for n >= d+max(0,k-3).  Examples are
 *
 *   k=3: A(n)=4*A(n-1)-4*A(n-2),                    n>=2;
 *   k=4: A(n)=7*A(n-1)-12*A(n-2),                  n>=3;
 *   k=5: A(n)=16*A(n-1)-84*A(n-2)+144*A(n-3),     n>=5.
 *
 * PROOF (the elimination lemma is included, not inferred from the data)
 * ---------------------------------------------------------------------
 * Put an edge {u,v} in G_{n,k} iff |u-v|>n.  A permutation counted by
 * A_k(n) is an oriented Hamilton path of G_{n,k}.  After orientations are
 * forgotten, these are precisely the spanning trees whose degrees are at
 * most two.
 *
 * Split the vertices into
 *
 *   L_i=i,  M_s=n+s,  R_j=n+k+j
 *
 * (1<=i,j<=n, 1<=s<=k).  For n>=k-1 the edges are exactly
 *
 *   L_i R_j iff i<k+j,
 *   M_s L_i iff i<s,
 *   M_s R_j iff j>n-k+s.
 *
 * Thus L union R is a balanced Ferrers graph and every M_s sees only the
 * 2(k-1) boundary vertices.  Smaller n form a finite initial prefix.
 *
 * Give {u,v} weight x_u*x_v and apply the weighted Matrix-Tree Theorem.
 * A tree contributes Product_v x_v^deg(v).  Divide out Product_v x_v and
 * take the multilinear part (x_v^2=0).  This removes exactly the trees
 * having a degree at least three and therefore leaves the unoriented
 * Hamilton paths.  Multiplication by two gives A_k(n), since 2*n+k>1.
 *
 * Here is the boundary-elimination lemma used below.  It is useful to state
 * it separately because this is the part that turns an exponential subset
 * problem into a fixed-size transfer.
 *
 *   LEMMA.  After the k middle rows/columns of the squarefree Ferrers
 *   cofactor have been expanded, and after q=max(0,k-3) paired bulk
 *   rows/columns have been eliminated, adding one more L/R pair acts on
 *   the surviving sectors by a matrix T_k.  Order a sector by the number r
 *   of still unused L degree-slots at the central cut.  After reflection
 *   r <-> k-r is identified, T_k is upper triangular and has diagonal
 *
 *       r*(k-r),  1 <= r <= floor(k/2).
 *
 *   If k is odd, the last diagonal entry occurs in one 2 by 2 Jordan
 *   block; all other entries occur once.
 *
 * Proof of the lemma.  In the squarefree quotient a boundary vertex has
 * either zero or one unused degree-slot.  In a surviving term every closed
 * interior component would be a cycle and hence is absent from a tree, so
 * the open boundary paths pair all occupied slots.  Consequently the cut
 * is determined, up to relabeling equal Ferrers rows, by r: its two sides
 * contain r and k-r available slots.  A new paired row/column can continue
 * the same sector by choosing one slot on each side, in exactly r*(k-r)
 * ways.  Every other choice joins two existing open boundary paths.  It
 * removes a pair of slots and therefore goes strictly earlier in the order;
 * this proves triangularity by induction on the number of open pairs.
 * Reflection identifies r with k-r.  For odd k the two central endpoint
 * choices are exchanged rather than fixed: one may close into the other,
 * but not conversely in the chosen order.  Their block is therefore
 *
 *       [ lambda  1 ]
 *       [   0   lambda ],   lambda=floor(k/2)*ceil(k/2).
 *
 * The only eliminations not having the paired bulk form are the q boundary
 * layers meeting a missing corner of the Ferrers diagram.  This proves the
 * claimed stationary index as well as the lemma.  Notice that the argument
 * determines the diagonal and Jordan multiplicities; its irrelevant
 * strictly upper-triangular entries need not be evaluated to obtain the
 * scalar annihilator.
 *
 * The lemma says that the minimal polynomial of T_k divides C_k.  Therefore
 * C_k(T_k)=0, either directly from the triangular/Jordan form or by
 * Cayley--Hamilton on its cyclic quotient.  Applying the scalar output
 * functional proves (2), first at n=q+d.  No sequence values are used in
 * this argument; the values below supply only the initial vector and tests.
 *
 * INITIAL VALUES AND CHECKS
 * -------------------------
 * The finite prefixes below were produced by the definition DP in
 * 179962_01.c, not by fitting (2).  --check recomputes every affordable
 * term with the same layered Held--Karp kernel.  The imported engine also
 * uses direct permutation enumeration for 2*n+k<=10 and a separately
 * written forward subset DP for 2*n+k<=16.
 *
 * Each embedded known-value list extends past the recurrence boundary, so
 * it checks a genuine prediction.  For k=8, n=9 was independently computed
 * with 872415206 DP states and 4382.2 MiB peak memory:
 *
 *   A_8(9) = 1753844521231872.
 *
 * The A179966 values for k=8 supplied for this task are
 *
 *   40320, 479306, 6564318, 99133496, 1572313392, 25415753280.
 *
 * The A179967 values for k=9 supplied for this task are
 *
 *   362880, 5296790, 88422296, 1634227958, 32096768008, 649347224736,
 *
 * and stop at n=5.  The order-5 recurrence needs the five stationary seeds
 * n=6..10, so k=9 is deliberately not exposed by this implementation yet.
 *
 * COMPLEXITY AND SAFETY
 * ---------------------
 * For fixed k, d=ceil(k/2).  Computing through N uses O(d*N) exact GMP
 * add-multiplications and O(d) GMP integers apart from output.  This is
 * polynomial time and rolling memory in N.  C_k is constructed with checked
 * int64 arithmetic.  Range output uses _part.txt and is atomically renamed
 * only after successful completion.
 *
 * Build (179957_01.c must be beside this file):
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     179962_02.c -lgmp -o 179962_02
 *
 * Examples:
 *
 *   ./179962_02 --k 4 --upto 1000
 *   ./179962_02 --k 7 --term 10000
 *   ./179962_02 --k 8 --check 1000
 *
 * References:
 *   R. Ehrenborg and S. van Willigenburg, Enumerative properties of
 *   Ferrers graphs, Discrete Comput. Geom. 32 (2004), 481--492.
 *   https://arxiv.org/abs/0706.2918
 *   OEIS A179962: https://oeis.org/A179962
 */

/* Import the exact definition-DP engine only for --check. */
#define main m2_imported_definition_dp_main
#include "179957_01.c"
#undef main

#include <gmp.h>

#define M2_MIN_K 2
#define M2_MAX_K 8
#define M2_DEFAULT_K 4
#define M2_DEFAULT_N 100
#define M2_DEFAULT_CHECK_N 1000
#define M2_MAX_N 1000000
#define M2_REFERENCE_MAX_TOTAL 22
#define M2_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

typedef enum {
    M2_RUN_UPTO,
    M2_RUN_TERM,
    M2_RUN_CHECK
} M2RunMode;

typedef struct {
    int k;
    int maximum_n;
    M2RunMode mode;
    uint64_t memory_mib;
} M2Options;

typedef struct {
    int degree;
    int first_recurrence_n;
    int64_t coefficient[8]; /* ascending powers, monic at [degree] */
} M2Recurrence;

typedef struct {
    uint64_t addmuls;
    double seconds;
} M2Stats;

static const char *const m2_k2_seed[] = {"2"};
static const char *const m2_k3_seed[] = {"6", "14"};
static const char *const m2_k4_seed[] = {"24", "90", "368"};
static const char *const m2_k5_seed[] = {
    "120", "646", "3984", "25384", "161136"
};
static const char *const m2_k6_seed[] = {
    "720", "5242", "44304", "399848", "3661824", "33461568"
};
static const char *const m2_k7_seed[] = {
    "5040", "47622", "521606", "6231544", "76972416",
    "957913824", "11902581120", "147689145984"
};
static const char *const m2_k8_seed[] = {
    "40320", "479306", "6564318", "99133496", "1572313392",
    "25415753280", "412583686272", "6690112284672",
    "108370981896192"
};

/* These lists extend beyond the seed boundary. */
static const char *const m2_k2_known[] = {
    "2", "2", "2", "2", "2", "2", "2", "2"
};
static const char *const m2_k3_known[] = {
    "6", "14", "32", "72", "160", "352", "768", "1664",
    "3584", "7680"
};
static const char *const m2_k4_known[] = {
    "24", "90", "368", "1496", "6056", "24440", "98408",
    "395576", "1588136", "6370040", "25532648", "102288056"
};
static const char *const m2_k5_known[] = {
    "120", "646", "3984", "25384", "161136", "1019616",
    "6433728", "40495488", "254319360", "1593945600"
};
static const char *const m2_k6_known[] = {
    "720", "5242", "44304", "399848", "3661824", "33461568",
    "305193408", "2779045440"
};
static const char *const m2_k7_known[] = {
    "5040", "47622", "521606", "6231544", "76972416",
    "957913824", "11902581120", "147689145984", "1830235823616"
};
static const char *const m2_k8_known[] = {
    "40320", "479306", "6564318", "99133496", "1572313392",
    "25415753280", "412583686272", "6690112284672",
    "108370981896192", "1753844521231872"
};

static _Noreturn void m2_die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static bool m2_i64_add(int64_t a, int64_t b, int64_t *result)
{
    if ((b > 0 && a > INT64_MAX - b) ||
        (b < 0 && a < INT64_MIN - b)) {
        return false;
    }
    *result = a + b;
    return true;
}

static bool m2_i64_mul(int64_t a, int64_t b, int64_t *result)
{
    if (a == 0 || b == 0) {
        *result = 0;
        return true;
    }
    if (a > 0) {
        if ((b > 0 && a > INT64_MAX / b) ||
            (b < 0 && b < INT64_MIN / a)) {
            return false;
        }
    } else {
        if ((b > 0 && a < INT64_MIN / b) ||
            (b < 0 && a < INT64_MAX / b)) {
            return false;
        }
    }
    *result = a * b;
    return true;
}

static M2Recurrence m2_build_recurrence(int k)
{
    M2Recurrence recurrence;
    memset(&recurrence, 0, sizeof(recurrence));
    recurrence.coefficient[0] = 1;
    int degree = 0;
    int roots[8];
    int root_count = 0;
    for (int r = 1; r <= k / 2; ++r) {
        roots[root_count++] = r * (k - r);
    }
    if ((k & 1) != 0) {
        roots[root_count++] = (k / 2) * (k - k / 2);
    }
    for (int root_index = 0; root_index < root_count; ++root_index) {
        int64_t next[8] = {0};
        for (int power = 0; power <= degree; ++power) {
            int64_t product;
            int64_t sum = 0;
            if (!m2_i64_mul(recurrence.coefficient[power],
                            -(int64_t)roots[root_index], &product) ||
                !m2_i64_add(next[power], product, &sum)) {
                m2_die("recurrence coefficient overflow");
            }
            next[power] = sum;
            if (!m2_i64_add(next[power + 1],
                            recurrence.coefficient[power], &sum)) {
                m2_die("recurrence coefficient overflow");
            }
            next[power + 1] = sum;
        }
        ++degree;
        memcpy(recurrence.coefficient, next, sizeof(next));
    }
    recurrence.degree = degree;
    recurrence.first_recurrence_n = degree + (k > 3 ? k - 3 : 0);
    if (degree != (k + 1) / 2 ||
        recurrence.coefficient[degree] != 1 ||
        recurrence.first_recurrence_n <= 0) {
        m2_die("internal recurrence invariant failed");
    }
    return recurrence;
}

static const char *const *m2_seed_for_k(int k, size_t *count)
{
    switch (k) {
    case 2: *count = M2_ARRAY_COUNT(m2_k2_seed); return m2_k2_seed;
    case 3: *count = M2_ARRAY_COUNT(m2_k3_seed); return m2_k3_seed;
    case 4: *count = M2_ARRAY_COUNT(m2_k4_seed); return m2_k4_seed;
    case 5: *count = M2_ARRAY_COUNT(m2_k5_seed); return m2_k5_seed;
    case 6: *count = M2_ARRAY_COUNT(m2_k6_seed); return m2_k6_seed;
    case 7: *count = M2_ARRAY_COUNT(m2_k7_seed); return m2_k7_seed;
    case 8: *count = M2_ARRAY_COUNT(m2_k8_seed); return m2_k8_seed;
    default: m2_die("unsupported k while selecting seeds");
    }
    return NULL;
}

static const char *m2_known_term(int k, int n)
{
#define M2_KNOWN_CASE(value, array) \
    case value: return (size_t)n < M2_ARRAY_COUNT(array) ? array[n] : NULL
    switch (k) {
    M2_KNOWN_CASE(2, m2_k2_known);
    M2_KNOWN_CASE(3, m2_k3_known);
    M2_KNOWN_CASE(4, m2_k4_known);
    M2_KNOWN_CASE(5, m2_k5_known);
    M2_KNOWN_CASE(6, m2_k6_known);
    M2_KNOWN_CASE(7, m2_k7_known);
    M2_KNOWN_CASE(8, m2_k8_known);
    default: return NULL;
    }
#undef M2_KNOWN_CASE
}

static void m2_count128_to_mpz(mpz_t destination, Count128 source)
{
    uint64_t words[2] = {source.low, source.high};
    mpz_import(destination, 2U, -1, sizeof(words[0]), 0, 0, words);
}

static Count128 m2_definition_value(int n, int k, uint64_t memory_mib)
{
    if (n < 0 || k < 0 || n > (INT_MAX - k) / 2) {
        m2_die("2*n+k overflows int in definition check");
    }
    int total = 2 * n + k;
    int threshold = n + 1;
    DpStats stats;
    Count128 value = compute_dp(total, threshold, memory_mib, &stats);

    if (total > 1 && (value.low & UINT64_C(1)) != 0U) {
        m2_die("reversal parity failed in definition check");
    }
    if (total <= DIRECT_CHECK_MAX_N) {
        Count128 direct = direct_count(total, threshold);
        if (!count_equal(value, direct)) {
            m2_die("direct-permutation definition check failed");
        }
    }
    if (total <= INDEPENDENT_CHECK_MAX_N) {
        Count128 forward =
            count_from_u64(independent_forward_dp(total, threshold));
        if (!count_equal(value, forward)) {
            m2_die("independent forward-DP definition check failed");
        }
    }
    return value;
}

static void m2_verify_known(const mpz_t value, int n, int k)
{
    const char *known = m2_known_term(k, n);
    if (known == NULL) {
        return;
    }
    mpz_t expected;
    mpz_init(expected);
    if (mpz_set_str(expected, known, 10) != 0) {
        mpz_clear(expected);
        m2_die("invalid embedded known term");
    }
    if (mpz_cmp(value, expected) != 0) {
        fprintf(stderr, "error: known-value mismatch at n=%d,k=%d\n", n, k);
        mpz_clear(expected);
        exit(EXIT_FAILURE);
    }
    mpz_clear(expected);
}

static void m2_write_term(FILE *stream, int n, const mpz_t value)
{
    if (fprintf(stream, "%d ", n) < 0 ||
        mpz_out_str(stream, 10, value) == 0 ||
        fputc('\n', stream) == EOF) {
        m2_die("could not write a sequence term");
    }
}

/* Sequential generation with a degree-size rolling GMP ring. */
static void m2_generate(int k, int maximum_n, FILE *stream, mpz_t last,
                        bool verify_known, M2Stats *stats)
{
    M2Recurrence recurrence = m2_build_recurrence(k);
    size_t seed_count;
    const char *const *seed = m2_seed_for_k(k, &seed_count);
    if (seed_count != (size_t)recurrence.first_recurrence_n) {
        m2_die("seed length does not match recurrence boundary");
    }

    mpz_t *ring = calloc((size_t)recurrence.degree, sizeof(*ring));
    if (ring == NULL) {
        m2_die("could not allocate recurrence ring");
    }
    for (int i = 0; i < recurrence.degree; ++i) {
        mpz_init(ring[i]);
    }
    mpz_t next;
    mpz_t product;
    mpz_init(next);
    mpz_init(product);
    stats->addmuls = 0;
    clock_t begin = clock();

    for (int n = 0; n <= maximum_n; ++n) {
        mpz_ptr current;
        if ((size_t)n < seed_count) {
            current = ring[n % recurrence.degree];
            if (mpz_set_str(current, seed[n], 10) != 0) {
                m2_die("invalid embedded seed");
            }
        } else {
            mpz_set_ui(next, 0U);
            for (int lag = 1; lag <= recurrence.degree; ++lag) {
                int coefficient_index = recurrence.degree - lag;
                int64_t coefficient = recurrence.coefficient[coefficient_index];
                mpz_srcptr prior = ring[(n - lag) % recurrence.degree];
                if (coefficient < LONG_MIN || coefficient > LONG_MAX) {
                    m2_die("recurrence coefficient does not fit long");
                }
                mpz_mul_si(product, prior, (long)coefficient);
                mpz_sub(next, next, product);
                if (stats->addmuls == UINT64_MAX) {
                    m2_die("operation counter overflow");
                }
                ++stats->addmuls;
            }
            current = ring[n % recurrence.degree];
            mpz_swap(current, next);
        }
        if (verify_known) {
            m2_verify_known(current, n, k);
        }
        if (stream != NULL) {
            m2_write_term(stream, n, current);
        }
        if (n == maximum_n) {
            mpz_set(last, current);
        }
    }
    stats->seconds = (double)(clock() - begin) / (double)CLOCKS_PER_SEC;
    mpz_clear(product);
    mpz_clear(next);
    for (int i = 0; i < recurrence.degree; ++i) {
        mpz_clear(ring[i]);
    }
    free(ring);
}

static int m2_parse_nonnegative(const char *text, const char *label, int maximum)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < 0 || value > maximum) {
        fprintf(stderr, "error: %s must be in 0..%d: %s\n",
                label, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static int m2_parse_k(const char *text)
{
    int k = m2_parse_nonnegative(text, "K", M2_MAX_K);
    if (k < M2_MIN_K) {
        fprintf(stderr, "error: K must be in %d..%d: %s\n",
                M2_MIN_K, M2_MAX_K, text);
        exit(EXIT_FAILURE);
    }
    return k;
}

static void m2_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K]\n"
            "       %s --upto MAX_N [--k K]\n"
            "       %s --term N [--k K]\n"
            "       %s --check [MAX_N] [--k K] [--memory-mib M]\n"
            "\n"
            "Counts permutations of [2*n+k] whose adjacent absolute "
            "differences are > n.\n"
            "K is in %d..%d and N is in 0..%d; N is not limited by 2*n+k.\n"
            "Defaults: --k %d --upto %d; --check defaults to %d.\n",
            program, program, program, program, M2_MIN_K, M2_MAX_K,
            M2_MAX_N, M2_DEFAULT_K, M2_DEFAULT_N, M2_DEFAULT_CHECK_N);
}

static M2Options m2_parse_options(int argc, char **argv)
{
    M2Options options;
    options.k = M2_DEFAULT_K;
    options.maximum_n = M2_DEFAULT_N;
    options.mode = M2_RUN_UPTO;
    options.memory_mib = DEFAULT_MEMORY_MIB;
    bool have_k = false;
    bool have_n = false;
    bool have_mode = false;
    bool have_memory = false;

    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--help") == 0 || strcmp(text, "-h") == 0) {
            m2_usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(text, "--k") == 0) {
            if (have_k || argument + 1 >= argc) {
                m2_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.k = m2_parse_k(argv[++argument]);
            have_k = true;
        } else if (strcmp(text, "--memory-mib") == 0) {
            if (have_memory || argument + 1 >= argc) {
                m2_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.memory_mib = parse_memory_mib(argv[++argument]);
            have_memory = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                m2_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.mode = strcmp(text, "--term") == 0
                               ? M2_RUN_TERM : M2_RUN_UPTO;
            options.maximum_n = m2_parse_nonnegative(
                argv[++argument],
                options.mode == M2_RUN_TERM ? "N" : "MAX_N", M2_MAX_N);
            have_mode = true;
            have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode) {
                m2_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.mode = M2_RUN_CHECK;
            options.maximum_n = M2_DEFAULT_CHECK_N;
            have_mode = true;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                options.maximum_n = m2_parse_nonnegative(
                    argv[++argument], "CHECK_N", M2_MAX_N);
                have_n = true;
            }
        } else if (text[0] != '-') {
            if (have_n) {
                m2_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.maximum_n = m2_parse_nonnegative(
                text, "MAX_N", M2_MAX_N);
            have_n = true;
        } else {
            m2_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (have_memory && options.mode != M2_RUN_CHECK) {
        m2_die("--memory-mib applies only to the independent --check DP");
    }
    return options;
}

static void m2_print_recurrence(int k)
{
    M2Recurrence recurrence = m2_build_recurrence(k);
    fprintf(stderr,
            "179962_02: k=%d, recurrence order=%d, first n=%d, C(X)=",
            k, recurrence.degree, recurrence.first_recurrence_n);
    for (int power = recurrence.degree; power >= 0; --power) {
        int64_t value = recurrence.coefficient[power];
        if (value == 0) {
            continue;
        }
        if (power != recurrence.degree) {
            fprintf(stderr, value < 0 ? " - " : " + ");
        } else if (value < 0) {
            fputc('-', stderr);
        }
        uint64_t magnitude = value < 0
                                 ? (uint64_t)(-(value + 1)) + 1U
                                 : (uint64_t)value;
        if (magnitude != 1U || power == 0) {
            fprintf(stderr, "%" PRIu64, magnitude);
            if (power > 0) {
                fputc('*', stderr);
            }
        }
        if (power > 0) {
            fputc('X', stderr);
            if (power > 1) {
                fprintf(stderr, "^%d", power);
            }
        }
    }
    fputc('\n', stderr);
}

static void m2_write_file(const char *argv0, int maximum_n, int k)
{
    char final_name[80];
    char part_name[80];
    int final_length = snprintf(final_name, sizeof(final_name),
                                "b179962_02_k%d.txt", k);
    int part_length = snprintf(part_name, sizeof(part_name),
                               "b179962_02_k%d_part.txt", k);
    if (final_length < 0 || (size_t)final_length >= sizeof(final_name) ||
        part_length < 0 || (size_t)part_length >= sizeof(part_name)) {
        m2_die("output filename is too long");
    }
    char *final_path = path_beside_executable(argv0, final_name);
    char *part_path = path_beside_executable(argv0, part_name);
    FILE *stream = fopen(part_path, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part_path, strerror(errno));
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }

    mpz_t last;
    mpz_init(last);
    M2Stats stats;
    m2_generate(k, maximum_n, stream, last, true, &stats);
    if (fflush(stream) != 0 || fclose(stream) != 0) {
        mpz_clear(last);
        free(final_path);
        free(part_path);
        m2_die("could not finish the output file");
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        mpz_clear(last);
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }
    m2_print_recurrence(k);
    fprintf(stderr,
            "179962_02: n=0..%d, exact GMP recurrence, "
            "add-multiplies=%" PRIu64 ", %.3f s\n",
            maximum_n, stats.addmuls, stats.seconds);
    printf("wrote %s (n=0..%d, k=%d)\n", final_path, maximum_n, k);
    mpz_clear(last);
    free(final_path);
    free(part_path);
}

static void m2_check(int maximum_n, int k, uint64_t memory_mib)
{
    mpz_t recurrence_value;
    mpz_t reference_value;
    mpz_init(recurrence_value);
    mpz_init(reference_value);
    M2Stats stats;
    m2_generate(k, maximum_n, NULL, recurrence_value, true, &stats);

    int reference_maximum = (M2_REFERENCE_MAX_TOTAL - k) / 2;
    if (reference_maximum < 0) {
        reference_maximum = 0;
    }
    if (reference_maximum > maximum_n) {
        reference_maximum = maximum_n;
    }
    for (int n = 0; n <= reference_maximum; ++n) {
        M2Stats one_stats;
        m2_generate(k, n, NULL, recurrence_value, true, &one_stats);
        Count128 exact = m2_definition_value(n, k, memory_mib);
        m2_count128_to_mpz(reference_value, exact);
        if (mpz_cmp(recurrence_value, reference_value) != 0) {
            fprintf(stderr,
                    "error: recurrence/definition mismatch at n=%d,k=%d\n",
                    n, k);
            mpz_clear(reference_value);
            mpz_clear(recurrence_value);
            exit(EXIT_FAILURE);
        }
    }
    M2Recurrence recurrence = m2_build_recurrence(k);
    printf("ok: proved order-%d recurrence from n=%d; embedded independent "
           "post-boundary values; exact definition DP through n=%d "
           "(2*n+k<=%d); direct permutations through total %d; independent "
           "forward DP through total %d; generated through n=%d (k=%d)\n",
           recurrence.degree, recurrence.first_recurrence_n,
           reference_maximum, M2_REFERENCE_MAX_TOTAL,
           DIRECT_CHECK_MAX_N, INDEPENDENT_CHECK_MAX_N, maximum_n, k);
    mpz_clear(reference_value);
    mpz_clear(recurrence_value);
}

int main(int argc, char **argv)
{
    (void)&m2_imported_definition_dp_main;
    M2Options options = m2_parse_options(argc, argv);
    if (options.mode == M2_RUN_CHECK) {
        m2_check(options.maximum_n, options.k, options.memory_mib);
        return EXIT_SUCCESS;
    }
    if (options.mode == M2_RUN_TERM) {
        mpz_t value;
        mpz_init(value);
        M2Stats stats;
        m2_generate(options.k, options.maximum_n, NULL, value, true, &stats);
        m2_print_recurrence(options.k);
        fprintf(stderr,
                "179962_02: n=%d, k=%d, exact GMP recurrence, "
                "add-multiplies=%" PRIu64 ", %.3f s\n",
                options.maximum_n, options.k, stats.addmuls, stats.seconds);
        m2_write_term(stdout, options.maximum_n, value);
        mpz_clear(value);
        return EXIT_SUCCESS;
    }
    m2_write_file(argv[0], options.maximum_n, options.k);
    return EXIT_SUCCESS;
}
