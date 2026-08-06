/*
 * Minimum-jump permutations for k=2,3,4,5,6 -- certified polynomial
 * recurrences.  A179957 is the k=4 specialization.
 *
 * Let v_n(x) be the saturated frontier vector from the sparse transfer
 * matrix M_k of 179957_03.c.  Saturation starts at n=k-1 and
 * v_{n+1}=M v_n.  Exact symbolic elimination over Z[x] gives
 *
 *   k=2:  v_n + p_1(x)v_{n-1} + ... + p_2(x)v_{n-2} = 0, n>=3;
 *   k=3:  v_n + p_1(x)v_{n-1} + ... + p_6(x)v_{n-6} = 0, n>=8;
 *   k=4:  v_n + p_1(x)v_{n-1} + ... + p_21(x)v_{n-21}=0, n>=24.
 *   k=5:  v_n + p_1(x)v_{n-1} + ... + p_79(x)v_{n-79}=0, n>=83.
 *   k=6:  v_n + p_1(x)v_{n-1} + ... + p_314(x)v_{n-314}=0, n>=319.
 *
 * The p_i are stored below.  This is not a guessed numerical recurrence.
 * On every run this program constructs the exact matrix and streams the
 * required initial vectors over GMP polynomial coefficients.  It generates
 * the Q_n seeds with one rolling pair and verifies P(M)v by a second rolling
 * Horner pair plus the immutable start vector.  Thus only five vectors are
 * retained, reducing certificate memory from O(S R^2) to O(S R).  The
 * certificate sizes (states, matrix edges, order) are
 * (2,4,2), (9,26,6), (49,200,21), (320,1730,79), and
 * (2357,16331,314), respectively.
 *
 * Since the same matrix M advances every later vector and polynomial
 * coefficients commute with M, multiplying the verified identity by M^r
 * proves it for every later n.  This finite vector identity is the runtime
 * proof certificate.
 *
 * Q_n(x), the oriented linear-forest polynomial, is a fixed linear output
 * of v_n, so it satisfies the same recurrence.  Its required initial values
 * are extracted during the same streaming pass; all later Q_n are computed
 * only by the recurrence.  Finally
 *
 *       a(n) = sum_j (-1)^j [x^j]Q_n(x) (n-j)!.
 *
 * Deriving and certifying analogous recurrences for k=7 and above would
 * require much larger state elimination (S(7)=19248, ...), so --k accepts
 * 2..6 here.
 * All arithmetic is exact GMP arithmetic.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     179957_05.c -lgmp -o 179957_05
 *
 * Usage:
 *   ./179957_05 --k 2 --upto 1000
 *   ./179957_05 --k 3 --term 1000
 *   ./179957_05 --k 4 --check 80
 *   ./179957_05 --k 5 --upto 1000
 *   ./179957_05 --k 6 --upto 1000
 *
 * Range runs still compute every requested term, but their OEIS b-file stops
 * immediately before the first term having more than 1000 decimal digits.
 * A b-file cannot have index gaps; --term remains unrestricted by this output
 * policy and can be used to print any individually computed term.
 */

#define A179957_03_NO_MAIN
#include "179957_03.c"
#undef A179957_03_NO_MAIN

#define M5_DEFAULT_K 4
#define M5_MIN_K 2
#define M5_MAX_K 6
#define M5_MAX_ORDER 314
#define M5_DEFAULT_CHECK_N 80
#define M5_BFILE_MAX_DIGITS 1000UL

typedef struct {
    uint16_t power;
    int32_t coefficient;
    const char *coefficient_text;
} M5Monomial;

typedef struct {
    const M5Monomial *term;
    size_t count;
} M5Polynomial;

#define M5_TERMS(name, ...) \
    static const M5Monomial name[] = {__VA_ARGS__}
#define M5_TERM(power_value, coefficient_value) \
    { (uint16_t)(power_value), (int32_t)(coefficient_value), NULL }
#define M5_BIG_TERM(power_value, coefficient_text_value) \
    { (uint16_t)(power_value), 0, (coefficient_text_value) }

M5_TERMS(m5_p1,  M5_TERM(0,-1), M5_TERM(1,-4));
M5_TERMS(m5_p2,  M5_TERM(1,-1), M5_TERM(2,3));
M5_TERMS(m5_p3,  M5_TERM(2,2), M5_TERM(3,4));
M5_TERMS(m5_p4,  M5_TERM(1,-1), M5_TERM(2,-10), M5_TERM(3,-19),
                     M5_TERM(4,-7));
M5_TERMS(m5_p5,  M5_TERM(2,-1), M5_TERM(4,23), M5_TERM(5,7));
M5_TERMS(m5_p6,  M5_TERM(3,3), M5_TERM(4,23), M5_TERM(5,33),
                     M5_TERM(6,5));
M5_TERMS(m5_p7,  M5_TERM(3,2), M5_TERM(4,21), M5_TERM(5,52),
                     M5_TERM(6,8), M5_TERM(7,-11));
M5_TERMS(m5_p8,  M5_TERM(4,2), M5_TERM(5,5), M5_TERM(6,-21),
                     M5_TERM(7,-58), M5_TERM(8,-3));
M5_TERMS(m5_p9,  M5_TERM(5,-2), M5_TERM(6,-17), M5_TERM(7,-48),
                     M5_TERM(8,-28), M5_TERM(9,-4));
M5_TERMS(m5_p10, M5_TERM(5,-1), M5_TERM(6,-10), M5_TERM(7,-29),
                     M5_TERM(8,-20), M5_TERM(9,24), M5_TERM(10,-8));
M5_TERMS(m5_p11, M5_TERM(6,-1), M5_TERM(7,-2), M5_TERM(8,4),
                     M5_TERM(9,35), M5_TERM(10,35), M5_TERM(11,1));
M5_TERMS(m5_p12, M5_TERM(9,2), M5_TERM(10,9), M5_TERM(11,2),
                     M5_TERM(12,2));
M5_TERMS(m5_p13, M5_TERM(8,-1), M5_TERM(9,-6), M5_TERM(10,-5),
                     M5_TERM(11,-12), M5_TERM(12,-7), M5_TERM(13,6));
M5_TERMS(m5_p14, M5_TERM(9,-1), M5_TERM(10,1), M5_TERM(11,5),
                     M5_TERM(12,-2), M5_TERM(13,-5), M5_TERM(14,10));
M5_TERMS(m5_p15, M5_TERM(11,2), M5_TERM(12,4), M5_TERM(13,5),
                     M5_TERM(14,6), M5_TERM(15,3));
M5_TERMS(m5_p16, M5_TERM(12,1), M5_TERM(14,-4), M5_TERM(15,-4));
M5_TERMS(m5_p17, M5_TERM(14,-4), M5_TERM(15,4), M5_TERM(16,-4),
                     M5_TERM(17,1));
M5_TERMS(m5_p18, M5_TERM(15,1), M5_TERM(17,-3), M5_TERM(18,-2));
M5_TERMS(m5_p19, M5_TERM(17,2), M5_TERM(18,-3), M5_TERM(19,-3));
M5_TERMS(m5_p20, M5_TERM(18,-1), M5_TERM(19,-1), M5_TERM(20,-1));
M5_TERMS(m5_p21, M5_TERM(20,1));

#define M5_POLY(name) { name, sizeof(name) / sizeof((name)[0]) }
static const M5Polynomial m5_polynomial_k4[22] = {
    {NULL, 0U},
    M5_POLY(m5_p1), M5_POLY(m5_p2), M5_POLY(m5_p3),
    M5_POLY(m5_p4), M5_POLY(m5_p5), M5_POLY(m5_p6),
    M5_POLY(m5_p7), M5_POLY(m5_p8), M5_POLY(m5_p9),
    M5_POLY(m5_p10), M5_POLY(m5_p11), M5_POLY(m5_p12),
    M5_POLY(m5_p13), M5_POLY(m5_p14), M5_POLY(m5_p15),
    M5_POLY(m5_p16), M5_POLY(m5_p17), M5_POLY(m5_p18),
    M5_POLY(m5_p19), M5_POLY(m5_p20), M5_POLY(m5_p21)
};

/* P_2(L,x) = L^2 - (1+x)L - x. */
M5_TERMS(m5_k2_p1, M5_TERM(0,-1), M5_TERM(1,-1));
M5_TERMS(m5_k2_p2, M5_TERM(1,-1));
static const M5Polynomial m5_polynomial_k2[3] = {
    {NULL, 0U}, M5_POLY(m5_k2_p1), M5_POLY(m5_k2_p2)
};

/*
 * P_3(L,x) = L^6 -(1+3x)L^5 +3x^2 L^4
 *             -(x+3x^2+2x^3)L^3 +(3x^3+2x^4)L^2
 *             +(x^3-x^5)L -x^5.
 */
M5_TERMS(m5_k3_p1, M5_TERM(0,-1), M5_TERM(1,-3));
M5_TERMS(m5_k3_p2, M5_TERM(2,3));
M5_TERMS(m5_k3_p3, M5_TERM(1,-1), M5_TERM(2,-3), M5_TERM(3,-2));
M5_TERMS(m5_k3_p4, M5_TERM(3,3), M5_TERM(4,2));
M5_TERMS(m5_k3_p5, M5_TERM(3,1), M5_TERM(5,-1));
M5_TERMS(m5_k3_p6, M5_TERM(5,-1));
static const M5Polynomial m5_polynomial_k3[7] = {
    {NULL, 0U}, M5_POLY(m5_k3_p1), M5_POLY(m5_k3_p2),
    M5_POLY(m5_k3_p3), M5_POLY(m5_k3_p4),
    M5_POLY(m5_k3_p5), M5_POLY(m5_k3_p6)
};

#include "179957_05_k5.inc"
#include "179957_05_k6.inc"

typedef struct {
    int k;
    int order;
    size_t expected_states;
    size_t expected_edges;
    const M5Polynomial *polynomial;
} M5Recurrence;

static const M5Recurrence m5_recurrence[] = {
    {2, 2, 2U, 4U, m5_polynomial_k2},
    {3, 6, 9U, 26U, m5_polynomial_k3},
    {4, 21, 49U, 200U, m5_polynomial_k4},
    {5, 79, 320U, 1730U, m5_polynomial_k5},
    {6, 314, 2357U, 16331U, m5_polynomial_k6}
};

static const M5Recurrence *m5_find_recurrence(int k)
{
    for (size_t i = 0U;
         i < sizeof(m5_recurrence) / sizeof(m5_recurrence[0]); ++i) {
        if (m5_recurrence[i].k == k) {
            return &m5_recurrence[i];
        }
    }
    die("certified recurrence is available only for k=2,3,4,5,6");
    return NULL;
}

typedef struct {
    size_t certificate_states;
    size_t certificate_matrix_edges;
    size_t certificate_peak_coefficient_slots;
    uint64_t certificate_scalar_checks;
    uint64_t certificate_addmuls;
    uint64_t recurrence_addmuls;
    double seconds;
} M5Stats;

typedef struct {
    uint16_t power;
    signed long small_coefficient;
    bool fits_signed_long;
    mpz_t coefficient;
} M5RuntimeMonomial;

typedef struct {
    M5RuntimeMonomial *term;
    size_t count;
} M5RuntimePolynomial;

typedef struct {
    M5RuntimePolynomial *polynomial;
    int order;
} M5RuntimeRecurrence;

static void m5_runtime_recurrence_init(
    M5RuntimeRecurrence *runtime, const M5Recurrence *source)
{
    runtime->order = source->order;
    runtime->polynomial = xcalloc((size_t)source->order + 1U,
                                  sizeof(*runtime->polynomial));
    for (int lag = 1; lag <= source->order; ++lag) {
        const M5Polynomial *source_polynomial = &source->polynomial[lag];
        M5RuntimePolynomial *destination = &runtime->polynomial[lag];
        destination->count = source_polynomial->count;
        destination->term = xcalloc(destination->count,
                                    sizeof(*destination->term));
        for (size_t t = 0U; t < destination->count; ++t) {
            const M5Monomial *source_term = &source_polynomial->term[t];
            M5RuntimeMonomial *term = &destination->term[t];
            term->power = source_term->power;
            mpz_init(term->coefficient);
            if (source_term->coefficient_text != NULL) {
                if (mpz_set_str(term->coefficient,
                                source_term->coefficient_text, 10) != 0) {
                    die("invalid large recurrence coefficient");
                }
            } else {
                mpz_set_si(term->coefficient,
                           (signed long)source_term->coefficient);
            }
            term->fits_signed_long =
                mpz_fits_slong_p(term->coefficient) != 0;
            if (term->fits_signed_long) {
                term->small_coefficient = mpz_get_si(term->coefficient);
            }
        }
    }
}

static void m5_runtime_recurrence_destroy(M5RuntimeRecurrence *runtime)
{
    if (runtime->polynomial == NULL) {
        return;
    }
    for (int lag = 1; lag <= runtime->order; ++lag) {
        M5RuntimePolynomial *polynomial = &runtime->polynomial[lag];
        for (size_t t = 0U; t < polynomial->count; ++t) {
            mpz_clear(polynomial->term[t].coefficient);
        }
        free(polynomial->term);
    }
    free(runtime->polynomial);
    memset(runtime, 0, sizeof(*runtime));
}

static void m5_addmul_signed_long(mpz_t destination, const mpz_t source,
                                  signed long coefficient)
{
    if (coefficient == 1) {
        mpz_add(destination, destination, source);
    } else if (coefficient == -1) {
        mpz_sub(destination, destination, source);
    } else if (coefficient > 0) {
        mpz_addmul_ui(destination, source, (unsigned long)coefficient);
    } else if (coefficient < 0) {
        mpz_submul_ui(destination, source,
                      (unsigned long)(-coefficient));
    }
}

static void m5_addmul_runtime(mpz_t destination, const mpz_t source,
                              const M5RuntimeMonomial *term, int sign)
{
    if (sign != 1 && sign != -1) {
        die("invalid recurrence coefficient sign");
    }
    if (term->fits_signed_long &&
        !(sign == -1 && term->small_coefficient == LONG_MIN)) {
        signed long coefficient = sign == 1
                                      ? term->small_coefficient
                                      : -term->small_coefficient;
        m5_addmul_signed_long(destination, source, coefficient);
    } else if (sign == 1) {
        mpz_addmul(destination, source, term->coefficient);
    } else {
        mpz_submul(destination, source, term->coefficient);
    }
}

static mpz_t *m5_make_start_vector(const M3Matrix *matrix,
                                   const M5Recurrence *recurrence,
                                   size_t width, M3Stats *matrix_stats)
{
    PolynomialMap maps[2];
    polynomial_map_init(&maps[0], width);
    polynomial_map_init(&maps[1], width);
    StateKey empty = {UINT64_C(0), UINT64_C(0)};
    PolynomialState *initial = polynomial_map_get(&maps[0], empty);
    mpz_set_ui(initial->coefficient[0], 1UL);
    int current = 0;
    int saturation_n = recurrence->k - 1;
    for (int n = 0; n < saturation_n; ++n) {
        int next = 1 - current;
        m3_polynomial_map_step(&maps[current], &maps[next], n, n,
                               saturation_n, matrix_stats);
        polynomial_map_destroy(&maps[current]);
        polynomial_map_init(&maps[current], width);
        current = next;
    }
    size_t slots = checked_product_size(matrix->registry.count, width);
    mpz_t *vector = mpz_array_create(slots);
    for (size_t state = 0U; state < maps[current].count; ++state) {
        size_t index = m3_registry_find(&matrix->registry,
                                        maps[current].item[state].key);
        size_t base = checked_product_size(index, width);
        int maximum_edges = saturation_n == 0 ? 0 : saturation_n - 1;
        for (int edges = 0; edges <= maximum_edges; ++edges) {
            mpz_set(vector[base + (size_t)edges],
                    maps[current].item[state].coefficient[edges]);
        }
    }
    polynomial_map_destroy(&maps[0]);
    polynomial_map_destroy(&maps[1]);
    return vector;
}

static void m5_accumulate_certificate(mpz_t *residual,
                                      const mpz_t *vector,
                                      size_t state_count, size_t width,
                                      const M5RuntimePolynomial *polynomial,
                                      M5Stats *stats)
{
    for (size_t state = 0U; state < state_count; ++state) {
        size_t base = checked_product_size(state, width);
        for (size_t t = 0U; t < polynomial->count; ++t) {
            size_t power = polynomial->term[t].power;
            if (power >= width) {
                die("certificate polynomial degree overflow");
            }
            for (size_t source_degree = 0U;
                 source_degree < width - power; ++source_degree) {
                const mpz_t *source = &vector[base + source_degree];
                if (mpz_sgn(*source) == 0) {
                    continue;
                }
                m5_addmul_runtime(
                    residual[base + source_degree + power], *source,
                    &polynomial->term[t], 1);
                increment_u64(&stats->certificate_addmuls,
                              "recurrence certificate operation");
            }
        }
    }
}

static void m5_verify_certificate(const M3Matrix *matrix,
                                  const mpz_t *residual, size_t width,
                                  M5Stats *stats)
{
    for (size_t state = 0U; state < matrix->registry.count; ++state) {
        size_t base = checked_product_size(state, width);
        for (size_t degree = 0U; degree < width; ++degree) {
            increment_u64(&stats->certificate_scalar_checks,
                          "recurrence certificate check");
            if (mpz_sgn(residual[base + degree]) != 0) {
                gmp_fprintf(stderr,
                            "error: recurrence certificate failed at "
                            "state=%zu, x-degree=%zu: %Zd\n",
                            state, degree, residual[base + degree]);
                exit(EXIT_FAILURE);
            }
        }
    }
}

static void m5_vector_to_q(const M3Matrix *matrix, const mpz_t *vector,
                           const M5Recurrence *recurrence,
                           size_t vector_width, int n, mpz_t *q)
{
    int maximum_edges = n == 0 ? 0 : n - 1;
    for (int edges = 0; edges <= maximum_edges; ++edges) {
        mpz_set_ui(q[edges], 0UL);
    }
    for (size_t state = 0U; state < matrix->registry.count; ++state) {
        unsigned long orientation = final_orientation_weight(
            matrix->registry.item[state].key, recurrence->k - 1);
        size_t base = checked_product_size(state, vector_width);
        for (int edges = 0; edges <= maximum_edges; ++edges) {
            if (mpz_sgn(vector[base + (size_t)edges]) != 0) {
                mpz_addmul_ui(q[edges], vector[base + (size_t)edges],
                              orientation);
            }
        }
    }
}

static void m5_seed_small_q(int n, mpz_t *q)
{
    for (int degree = 0; degree <= (n == 0 ? 0 : n - 1); ++degree) {
        mpz_set_ui(q[degree], 0UL);
    }
    mpz_set_ui(q[0], 1UL);
    if (n == 2) {
        mpz_set_ui(q[1], 2UL);
    } else if (n == 3) {
        /* The forbidden graph is K_3 before the frontier saturates. */
        mpz_set_ui(q[1], 6UL);
        mpz_set_ui(q[2], 6UL);
    } else if (n == 4) {
        /* Oriented linear forests of K_4, needed before k=6 saturation. */
        mpz_set_ui(q[1], 12UL);
        mpz_set_ui(q[2], 36UL);
        mpz_set_ui(q[3], 24UL);
    }
}

static mpz_t *m5_recurrence_step(int n,
                                 const M5Recurrence *recurrence,
                                 const M5RuntimeRecurrence *runtime,
                                 mpz_t **ring,
                                 mpz_t *temporary, size_t width,
                                 M5Stats *stats)
{
    for (int degree = 0; degree <= n - 1; ++degree) {
        mpz_set_ui(temporary[degree], 0UL);
    }
    for (int lag = 1; lag <= recurrence->order; ++lag) {
        const mpz_t *source = ring[(n - lag) % recurrence->order];
        int source_maximum = n - lag - 1;
        const M5RuntimePolynomial *polynomial = &runtime->polynomial[lag];
        for (size_t t = 0U; t < polynomial->count; ++t) {
            int power = polynomial->term[t].power;
            for (int degree = 0; degree <= source_maximum; ++degree) {
                if (mpz_sgn(source[degree]) == 0) {
                    continue;
                }
                int destination_degree = degree + power;
                if (destination_degree >= n ||
                    (size_t)destination_degree >= width) {
                    die("recurrence polynomial degree overflow");
                }
                m5_addmul_runtime(temporary[destination_degree],
                                  source[degree], &polynomial->term[t], -1);
                increment_u64(&stats->recurrence_addmuls,
                              "recurrence polynomial operation");
            }
        }
    }
    for (int degree = 0; degree <= n - 1; ++degree) {
        if (mpz_sgn(temporary[degree]) < 0) {
            die("recurrence produced a negative forest coefficient");
        }
    }
    int slot = n % recurrence->order;
    mpz_t *old = ring[slot];
    ring[slot] = temporary;
    return old;
}

static mpz_t *m5_compute_sequence(int maximum_n, int k, FILE *stream,
                                  M5Stats *stats)
{
    const M5Recurrence *recurrence = m5_find_recurrence(k);
    int saturation_n = recurrence->k - 1;
    int certificate_n = saturation_n + recurrence->order;
    int seed_last_n = certificate_n - 1;
    size_t certificate_width = (size_t)certificate_n + 1U;
    memset(stats, 0, sizeof(*stats));
    double started = monotonic_seconds();
    size_t width = (size_t)maximum_n + 1U;
    mpz_t *answer = mpz_array_create(width);

    M3Matrix matrix;
    m3_matrix_build(recurrence->k, &matrix);
    if (matrix.registry.count != recurrence->expected_states ||
        matrix.edge_count != recurrence->expected_edges) {
        m3_matrix_destroy(&matrix);
        mpz_array_destroy(answer, width);
        die("unexpected certificate matrix dimensions");
    }
    stats->certificate_states = matrix.registry.count;
    stats->certificate_matrix_edges = matrix.edge_count;

    M5RuntimeRecurrence runtime;
    memset(&runtime, 0, sizeof(runtime));
    m5_runtime_recurrence_init(&runtime, recurrence);

    M3Stats matrix_stats;
    memset(&matrix_stats, 0, sizeof(matrix_stats));
    size_t certificate_slots = checked_product_size(
        matrix.registry.count, certificate_width);
    stats->certificate_peak_coefficient_slots = checked_product_size(
        certificate_slots, 5U);

    mpz_t *factorial = mpz_array_create(width);
    mpz_set_ui(factorial[0], 1UL);
    for (int n = 1; n <= maximum_n; ++n) {
        mpz_mul_ui(factorial[n], factorial[n - 1], (unsigned long)n);
    }
    mpz_t **ring = xcalloc((size_t)recurrence->order, sizeof(*ring));
    for (int slot = 0; slot < recurrence->order; ++slot) {
        ring[slot] = mpz_array_create(width);
    }
    mpz_t *temporary = mpz_array_create(width);

    int seed_end = maximum_n < seed_last_n ? maximum_n : seed_last_n;
    int small_end = seed_end < saturation_n - 1
                        ? seed_end : saturation_n - 1;
    for (int n = 0; n <= small_end; ++n) {
        mpz_t *q = ring[n % recurrence->order];
        m5_seed_small_q(n, q);
        m3_evaluate_q((const mpz_t *)q, n,
                      (const mpz_t *)factorial, answer[n]);
        write_complete_term(stream, n, answer[n]);
    }

    mpz_t *start_vector = m5_make_start_vector(
        &matrix, recurrence, certificate_width, &matrix_stats);
    mpz_t *sequence_work[2] = {
        mpz_array_create(certificate_slots),
        mpz_array_create(certificate_slots)
    };
    mpz_t *horner_work[2] = {
        mpz_array_create(certificate_slots),
        mpz_array_create(certificate_slots)
    };
    mpz_t *matrix_certificate_q = maximum_n >= certificate_n
                                      ? mpz_array_create(width) : NULL;
    mpz_t *sequence_current = start_vector;
    mpz_t *horner_current = start_vector;
    /*
     * The first rolling pair generates v_s,...,v_{s+R} for Q seeds.  The
     * second verifies the same annihilator in Horner form:
     *
     *   h_0=v_s,  h_i=M h_{i-1}+p_i v_s,
     *
     * so h_R=P(M)v_s.  This replaces O(S R^2) stored GMP coefficients by
     * five O(S R) vectors and avoids multiplying every p_i by a large v_j.
     */
    for (int step = 0; step <= recurrence->order; ++step) {
        int vector_n = saturation_n + step;
        if (vector_n <= seed_end) {
            mpz_t *q = ring[vector_n % recurrence->order];
            m5_vector_to_q(&matrix, sequence_current, recurrence,
                           certificate_width, vector_n, q);
            m3_evaluate_q((const mpz_t *)q, vector_n,
                          (const mpz_t *)factorial, answer[vector_n]);
            write_complete_term(stream, vector_n, answer[vector_n]);
        } else if (vector_n == certificate_n &&
                   matrix_certificate_q != NULL) {
            m5_vector_to_q(&matrix, sequence_current, recurrence,
                           certificate_width, vector_n,
                           matrix_certificate_q);
        }
        if (step < recurrence->order) {
            mpz_t *sequence_next = step == 0
                                       ? sequence_work[0]
                                       : (sequence_current == sequence_work[0]
                                              ? sequence_work[1]
                                              : sequence_work[0]);
            mpz_t *horner_next = step == 0
                                     ? horner_work[0]
                                     : (horner_current == horner_work[0]
                                            ? horner_work[1]
                                            : horner_work[0]);
            m3_apply_matrix(&matrix, sequence_current, sequence_next,
                            vector_n, certificate_width, &matrix_stats);
            m3_apply_matrix(&matrix, horner_current, horner_next,
                            vector_n, certificate_width, &matrix_stats);
            m5_accumulate_certificate(
                horner_next, start_vector, matrix.registry.count,
                certificate_width, &runtime.polynomial[step + 1], stats);
            sequence_current = sequence_next;
            horner_current = horner_next;
        }
    }
    m5_verify_certificate(&matrix, (const mpz_t *)horner_current,
                          certificate_width, stats);
    mpz_array_destroy(start_vector, certificate_slots);
    mpz_array_destroy(sequence_work[0], certificate_slots);
    mpz_array_destroy(sequence_work[1], certificate_slots);
    mpz_array_destroy(horner_work[0], certificate_slots);
    mpz_array_destroy(horner_work[1], certificate_slots);

    for (int n = certificate_n; n <= maximum_n; ++n) {
        temporary = m5_recurrence_step(n, recurrence, &runtime, ring,
                                       temporary, width, stats);
        mpz_t *q = ring[n % recurrence->order];
        if (n == certificate_n && matrix_certificate_q != NULL) {
            for (int degree = 0; degree <= n - 1; ++degree) {
                if (mpz_cmp(q[degree], matrix_certificate_q[degree]) != 0) {
                    gmp_fprintf(stderr,
                                "error: first recurrence/matrix polynomial "
                                "mismatch at n=%d, degree=%d: %Zd versus "
                                "%Zd\n",
                                n, degree, q[degree],
                                matrix_certificate_q[degree]);
                    exit(EXIT_FAILURE);
                }
            }
            mpz_array_destroy(matrix_certificate_q, width);
            matrix_certificate_q = NULL;
        }
        m3_evaluate_q((const mpz_t *)q, n,
                      (const mpz_t *)factorial, answer[n]);
        write_complete_term(stream, n, answer[n]);
    }
    if (matrix_certificate_q != NULL) {
        mpz_array_destroy(matrix_certificate_q, width);
    }

    for (int slot = 0; slot < recurrence->order; ++slot) {
        mpz_array_destroy(ring[slot], width);
    }
    free(ring);
    mpz_array_destroy(temporary, width);
    mpz_array_destroy(factorial, width);
    m5_runtime_recurrence_destroy(&runtime);
    m3_matrix_destroy(&matrix);
    stats->seconds = monotonic_seconds() - started;
    return answer;
}

static void m5_print_stats(int maximum_n, int k, const M5Stats *stats)
{
    const M5Recurrence *recurrence = m5_find_recurrence(k);
    double certificate_header_mib =
        (double)stats->certificate_peak_coefficient_slots *
        (double)sizeof(mpz_t) / (1024.0 * 1024.0);
    fprintf(stderr,
            "179957_05: k=%d, n=0..%d, certified order-%d recurrence, "
            "certificate matrix=%zu states/%zu edges, certificate "
            "stream slots=%zu (%.1f MiB GMP headers), scalars=%llu, "
            "certificate addmuls=%llu, recurrence addmuls=%llu, %.3f s\n",
            k, maximum_n, recurrence->order, stats->certificate_states,
            stats->certificate_matrix_edges,
            stats->certificate_peak_coefficient_slots,
            certificate_header_mib,
            (unsigned long long)stats->certificate_scalar_checks,
            (unsigned long long)stats->certificate_addmuls,
            (unsigned long long)stats->recurrence_addmuls,
            stats->seconds);
}

static int m5_check(int maximum_n, int k)
{
    M5Stats stats;
    mpz_t *answer = m5_compute_sequence(maximum_n, k, NULL, &stats);
    if (verify_known(answer, maximum_n, k) != EXIT_SUCCESS) {
        mpz_array_destroy(answer, (size_t)maximum_n + 1U);
        return EXIT_FAILURE;
    }
    M3Stats reference_stats;
    mpz_t *reference = m3_compute_sequence(maximum_n, k, NULL,
                                           &reference_stats);
    for (int n = 0; n <= maximum_n; ++n) {
        if (mpz_cmp(answer[n], reference[n]) != 0) {
            gmp_fprintf(stderr,
                        "error: recurrence/matrix mismatch at n=%d: "
                        "%Zd versus %Zd\n", n, answer[n], reference[n]);
            mpz_array_destroy(reference, (size_t)maximum_n + 1U);
            mpz_array_destroy(answer, (size_t)maximum_n + 1U);
            return EXIT_FAILURE;
        }
    }
    int direct_max = maximum_n < DIRECT_CHECK_MAX_N
                         ? maximum_n : DIRECT_CHECK_MAX_N;
    for (int n = 0; n <= direct_max; ++n) {
        uint64_t direct = direct_permutation_count(n, k);
        if (mpz_cmp_ui(answer[n], (unsigned long)direct) != 0) {
            fprintf(stderr, "error: recurrence/direct mismatch at n=%d\n", n);
            mpz_array_destroy(reference, (size_t)maximum_n + 1U);
            mpz_array_destroy(answer, (size_t)maximum_n + 1U);
            return EXIT_FAILURE;
        }
    }
    m5_print_stats(maximum_n, k, &stats);
    printf("ok: exact vector certificate verified; recurrence agrees "
           "with 03 through n=%d%s, and full "
           "permutations through n=%d\n",
           maximum_n,
           k == 4 ? ", A179957 through its built-in range" : "",
           direct_max);
    mpz_array_destroy(reference, (size_t)maximum_n + 1U);
    mpz_array_destroy(answer, (size_t)maximum_n + 1U);
    return EXIT_SUCCESS;
}

static void m5_write_file(const char *argv0, int maximum_n, int k,
                          const mpz_t *answer)
{
    char final_name[64];
    char part_name[64];
    int final_length = snprintf(final_name, sizeof(final_name),
                                "b179957_05_k%d.txt", k);
    int part_length = snprintf(part_name, sizeof(part_name),
                               "b179957_05_k%d_part.txt", k);
    if (final_length < 0 || (size_t)final_length >= sizeof(final_name) ||
        part_length < 0 || (size_t)part_length >= sizeof(part_name)) {
        die("recurrence output filename overflow");
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
    mpz_t first_excluded_value;
    mpz_init(first_excluded_value);
    mpz_ui_pow_ui(first_excluded_value, 10UL, M5_BFILE_MAX_DIGITS);
    int written_maximum_n = -1;
    for (int n = 0; n <= maximum_n; ++n) {
        /* |a(n)| >= 10^1000 means that a(n) has at least 1001 digits. */
        if (mpz_cmpabs(answer[n], first_excluded_value) >= 0) {
            break;
        }
        if (gmp_fprintf(stream, "%d %Zd\n", n, answer[n]) < 0 ||
            fflush(stream) != 0) {
            (void)fclose(stream);
            mpz_clear(first_excluded_value);
            free(final_path);
            free(part_path);
            die("could not write recurrence output file");
        }
        written_maximum_n = n;
    }
    mpz_clear(first_excluded_value);
    if (fclose(stream) != 0 || rename(part_path, final_path) != 0) {
        free(final_path);
        free(part_path);
        die("could not finish recurrence output file");
    }
    printf("wrote %s (n=0..%d", final_path, written_maximum_n);
    if (written_maximum_n < maximum_n) {
        printf("; requested through n=%d, later terms omitted because the "
               "next term exceeds %lu decimal digits",
               maximum_n, M5_BFILE_MAX_DIGITS);
    }
    printf(")\n");
    free(final_path);
    free(part_path);
}

static void m5_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K]\n"
            "       %s --upto MAX_N [--k K]\n"
            "       %s --term N [--k K]\n"
            "       %s --check [MAX_N] [--k K]\n"
            "Certified recurrences are available for K=2,3,4,5,6; "
            "the default is 4; N is 0..10000.\n"
            "A range run writes b179957_05_kK.txt, stopping before the "
            "first term over %lu digits.\n",
            program, program, program, program, M5_BFILE_MAX_DIGITS);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        m5_usage(argv[0]);
        return EXIT_SUCCESS;
    }
    RunMode mode = RUN_UPTO;
    int n = -1;
    bool have_n = false;
    bool have_mode = false;
    int k = M5_DEFAULT_K;
    bool have_k = false;
    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--k") == 0) {
            if (have_k || argument + 1 >= argc) {
                m5_usage(argv[0]);
                return EXIT_FAILURE;
            }
            k = parse_bounded_integer(argv[++argument], "K",
                                      M5_MIN_K, M5_MAX_K);
            have_k = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                m5_usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = strcmp(text, "--term") == 0 ? RUN_TERM : RUN_UPTO;
            have_mode = true;
            n = parse_bounded_integer(argv[++argument],
                        mode == RUN_TERM ? "N" : "MAX_N",
                        0, MAX_SUPPORTED_N);
            have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode || have_n) {
                m5_usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = RUN_CHECK;
            have_mode = true;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                n = parse_bounded_integer(argv[++argument], "CHECK_N", 0,
                                          MAX_SUPPORTED_N);
                have_n = true;
            }
        } else if (text[0] == '-' || have_n) {
            m5_usage(argv[0]);
            return EXIT_FAILURE;
        } else {
            n = parse_bounded_integer(text,
                        mode == RUN_CHECK ? "CHECK_N" : "MAX_N",
                        0, MAX_SUPPORTED_N);
            have_n = true;
        }
    }
    if (!have_n) {
        n = mode == RUN_CHECK ? M5_DEFAULT_CHECK_N : DEFAULT_MAX_N;
    }
    if (mode == RUN_CHECK) {
        return m5_check(n, k);
    }
    M5Stats stats;
    mpz_t *answer = m5_compute_sequence(n, k, NULL, &stats);
    if (verify_known(answer, n, k) != EXIT_SUCCESS) {
        mpz_array_destroy(answer, (size_t)n + 1U);
        return EXIT_FAILURE;
    }
    m5_print_stats(n, k, &stats);
    if (mode == RUN_TERM) {
        gmp_printf("%d %Zd\n", n, answer[n]);
    } else {
        m5_write_file(argv[0], n, k, (const mpz_t *)answer);
    }
    mpz_array_destroy(answer, (size_t)n + 1U);
    return EXIT_SUCCESS;
}
