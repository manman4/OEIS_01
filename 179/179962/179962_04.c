/*
 * A179962 from the proved part of 179962_02.c only.
 *
 * Define A_k(n) as the number of permutations p of [2*n+k] satisfying
 *
 *                       |p(i+1)-p(i)| > n.
 *
 * This program does NOT use the conjectural recurrence, its proposed roots
 * r*(k-r), its proposed odd-k Jordan factor, or its proposed start index.
 * It implements only proved Lemmas A1--A3 from 179962_02.c.
 *
 * For n>=k-1, split V=L union M union R and enumerate every valid gadget g,
 * i.e. the one or two path-neighbors chosen for each M_s.  Discard gadgets
 * containing a cycle or a vertex of degree at least three, and the immediate
 * zero cases with a closed D--D component or more than two global D ends.
 * The remaining gadgets are grouped by the proved trace
 *
 *   (multiset of component endpoint-type pairs, intL, intR).
 *
 * Boundary twins in the Ferrers graph S=G-M prove that every gadget in one
 * trace has the same completion count.  For one representative g per trace,
 * form H_g=S union g.  A Hamilton path of H_g uses every gadget edge exactly
 * when no degree-two M vertex is an endpoint: degree-one M vertices have
 * their unique edge forced, and an internal degree-two M vertex has both
 * incident edges forced.  A standard exact subset DP counts these oriented
 * Hamilton paths.  Therefore
 *
 *             A_k(n)=Sum_trace multiplicity(trace)*HP(H_g).       (1)
 *
 * This is exactly 2*Sum_g N_g from Lemma A1, because HP(H_g) counts the two
 * orientations of every completed undirected path.  For n<k-1, where M need
 * not be independent, the program uses the exact definition DP instead.
 *
 * This is an independent, proof-based checker for 02, not a polynomial-time
 * replacement: if B(k) is the number of traces, its worst-case cost is
 * O(B(k)*T^2*2^T) time and O(T*binomial(T,floor(T/2))+2^T) memory,
 * T=2*n+k.  Gadget enumeration itself has
 * ((k-1)+binomial(k-1,2))^k candidates.  The practical implementation range
 * is 2<=k<=5 and T<=24.  Counts are exact unsigned 128-bit integers; T<=24
 * is far inside the proved 34! < 2^128 numeric bound.
 *
 * --check compares (1) with the dense definition DP, direct permutation
 * enumeration for T<=10, an independently written forward DP for T<=16,
 * and embedded known values.  Range output uses an atomic _part.txt rename.
 *
 * Build (179957_01.c must be beside this file):
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     179962_04.c -o 179962_04
 *
 * Examples:
 *
 *   ./179962_04 --k 4 --upto 6
 *   ./179962_04 --k 4 --term 8 --memory-mib 2048
 *   ./179962_04 --k 4 --check 6
 */

#define main m4_imported_definition_main
#include "179957_01.c"
#undef main

#define M4_MIN_K 2
#define M4_MAX_K 5
#define M4_MAX_TOTAL 24
#define M4_DEFAULT_K 4
#define M4_DEFAULT_N 6
#define M4_DEFAULT_CHECK_N 6
#define M4_MAX_GAMMA (2 * (M4_MAX_K - 1))
#define M4_MAX_GADGET_VERTICES (3 * M4_MAX_K - 2)
#define M4_MAX_TRACE 128
#define M4_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

typedef struct {
    uint64_t key;
    uint64_t multiplicity;
    uint16_t choice[M4_MAX_K];
} M4Sector;

typedef struct {
    int k;
    int gamma_count;
    M4Sector sector[M4_MAX_TRACE];
    size_t sector_count;
    uint64_t valid_gadgets;
} M4Sectors;

typedef struct {
    int total;
    uint64_t choose[MAX_SUPPORTED_N + 1][MAX_SUPPORTED_N + 1];
    uint32_t *rank;
    size_t rank_entries;
    MemoryBudget budget;
    uint64_t states;
    uint64_t transitions;
} M4DpContext;

typedef struct {
    size_t sectors;
    uint64_t valid_gadgets;
    uint64_t states;
    uint64_t transitions;
    size_t peak_bytes;
    double seconds;
    bool used_gadgets;
} M4Stats;

typedef enum {
    M4_RUN_UPTO,
    M4_RUN_TERM,
    M4_RUN_CHECK
} M4Mode;

typedef struct {
    int k;
    int maximum_n;
    uint64_t memory_mib;
    M4Mode mode;
} M4Options;

typedef struct {
    int k;
    const char *const *term;
    size_t count;
} M4KnownFamily;

static const char *const m4_known_k2[] = {
    "2", "2", "2", "2", "2", "2", "2", "2"
};
static const char *const m4_known_k3[] = {
    "6", "14", "32", "72", "160", "352", "768", "1664",
    "3584", "7680"
};
static const char *const m4_known_k4[] = {
    "24", "90", "368", "1496", "6056", "24440", "98408",
    "395576", "1588136", "6370040", "25532648", "102288056"
};
static const char *const m4_known_k5[] = {
    "120", "646", "3984", "25384", "161136", "1019616",
    "6433728", "40495488", "254319360", "1593945600"
};

static const M4KnownFamily m4_known_family[] = {
    {2, m4_known_k2, M4_ARRAY_COUNT(m4_known_k2)},
    {3, m4_known_k3, M4_ARRAY_COUNT(m4_known_k3)},
    {4, m4_known_k4, M4_ARRAY_COUNT(m4_known_k4)},
    {5, m4_known_k5, M4_ARRAY_COUNT(m4_known_k5)}
};

static void m4_increment_u64(uint64_t *value, const char *label)
{
    if (*value == UINT64_MAX) {
        fprintf(stderr, "error: 04 %s counter overflow\n", label);
        exit(EXIT_FAILURE);
    }
    ++*value;
}

static unsigned m4_popcount(uint16_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return (unsigned)__builtin_popcount((unsigned)value);
#else
    unsigned count = 0;
    while (value != 0U) {
        value = (uint16_t)(value & (uint16_t)(value - 1U));
        ++count;
    }
    return count;
#endif
}

static int m4_endpoint_pair(int left, int right)
{
    if (left > right) {
        int temporary = left;
        left = right;
        right = temporary;
    }
    if (left == 0 && right == 0) return 0; /* LL */
    if (left == 0 && right == 1) return 1; /* LR */
    if (left == 0 && right == 2) return 2; /* LD */
    if (left == 1 && right == 1) return 3; /* RR */
    if (left == 1 && right == 2) return 4; /* RD */
    if (left == 2 && right == 2) return 5; /* DD */
    die("04 invalid gadget endpoint types");
    return -1;
}

static int m4_vertex_type(int vertex, int gamma_count)
{
    if (vertex < gamma_count) {
        return vertex < gamma_count / 2 ? 0 : 1;
    }
    return 2;
}

/* Return false for a cycle, degree>=3, or another non-path component. */
static bool m4_trace_key(int k, const uint16_t choice[M4_MAX_K],
                         uint64_t *key_result)
{
    int gamma_count = 2 * (k - 1);
    int vertex_count = gamma_count + k;
    uint16_t adjacency[M4_MAX_GADGET_VERTICES] = {0};
    unsigned degree[M4_MAX_GADGET_VERTICES] = {0};

    for (int s = 0; s < k; ++s) {
        int middle = gamma_count + s;
        uint16_t neighbors = choice[s];
        while (neighbors != 0U) {
            uint16_t bit = (uint16_t)(neighbors & (uint16_t)(0U-neighbors));
            neighbors = (uint16_t)(neighbors ^ bit);
#if defined(__GNUC__) || defined(__clang__)
            int gamma = __builtin_ctz((unsigned)bit);
#else
            int gamma = 0;
            while ((bit >> gamma) != 1U) ++gamma;
#endif
            if (gamma >= gamma_count) {
                die("04 gadget neighbor outside Gamma");
            }
            adjacency[middle] |= (uint16_t)(1U << gamma);
            adjacency[gamma] |= (uint16_t)(1U << middle);
            ++degree[middle];
            ++degree[gamma];
        }
    }
    for (int vertex = 0; vertex < vertex_count; ++vertex) {
        if (degree[vertex] > 2U) {
            return false;
        }
        if (vertex >= gamma_count &&
            (degree[vertex] < 1U || degree[vertex] > 2U)) {
            return false;
        }
    }

    unsigned pair_count[6] = {0};
    unsigned internal_left = 0;
    unsigned internal_right = 0;
    unsigned global_endpoints = 0;
    uint16_t visited = 0;
    for (int start = 0; start < vertex_count; ++start) {
        bool active = start >= gamma_count || degree[start] != 0U;
        uint16_t start_bit = (uint16_t)(1U << start);
        if (!active || (visited & start_bit) != 0U) {
            continue;
        }
        int stack[M4_MAX_GADGET_VERTICES];
        int stack_size = 0;
        int endpoints[2] = {-1, -1};
        int endpoint_count = 0;
        int component_vertices = 0;
        int degree_sum = 0;
        stack[stack_size++] = start;
        visited |= start_bit;
        while (stack_size != 0) {
            int vertex = stack[--stack_size];
            ++component_vertices;
            degree_sum += (int)degree[vertex];
            if (degree[vertex] == 1U) {
                if (endpoint_count >= 2) return false;
                endpoints[endpoint_count++] = vertex;
            } else if (degree[vertex] == 2U && vertex < gamma_count) {
                if (vertex < k - 1) ++internal_left;
                else ++internal_right;
            }
            uint16_t next = adjacency[vertex];
            while (next != 0U) {
                uint16_t bit = (uint16_t)(next & (uint16_t)(0U-next));
                next = (uint16_t)(next ^ bit);
#if defined(__GNUC__) || defined(__clang__)
                int neighbor = __builtin_ctz((unsigned)bit);
#else
                int neighbor = 0;
                while ((bit >> neighbor) != 1U) ++neighbor;
#endif
                if ((visited & bit) == 0U) {
                    visited |= bit;
                    stack[stack_size++] = neighbor;
                }
            }
        }
        int edges = degree_sum / 2;
        if (edges != component_vertices - 1 || endpoint_count != 2) {
            return false;
        }
        int type_left = m4_vertex_type(endpoints[0], gamma_count);
        int type_right = m4_vertex_type(endpoints[1], gamma_count);
        int pair = m4_endpoint_pair(type_left, type_right);
        global_endpoints += (unsigned)(type_left == 2) +
                            (unsigned)(type_right == 2);
        /* A D--D component is already closed off from S, and more than two
         * D ends cannot be the two ends of one completed Hamilton path. */
        if (pair == 5 || global_endpoints > 2U) {
            return false;
        }
        if (pair_count[pair] == UINT_MAX) {
            die("04 trace component counter overflow");
        }
        ++pair_count[pair];
    }

    uint64_t base = (uint64_t)k + UINT64_C(1);
    uint64_t key = 0;
    uint64_t place = 1;
    for (int pair = 0; pair < 6; ++pair) {
        key += place * (uint64_t)pair_count[pair];
        place *= base;
    }
    key += place * (uint64_t)internal_left;
    place *= base;
    key += place * (uint64_t)internal_right;
    *key_result = key;
    return true;
}

static void m4_add_sector(M4Sectors *sectors,
                          const uint16_t choice[M4_MAX_K])
{
    uint64_t key;
    if (!m4_trace_key(sectors->k, choice, &key)) {
        return;
    }
    m4_increment_u64(&sectors->valid_gadgets, "valid gadget");
    for (size_t i = 0; i < sectors->sector_count; ++i) {
        if (sectors->sector[i].key == key) {
            m4_increment_u64(&sectors->sector[i].multiplicity,
                             "trace multiplicity");
            return;
        }
    }
    if (sectors->sector_count >= M4_MAX_TRACE) {
        die("04 trace table capacity exceeded");
    }
    M4Sector *sector = &sectors->sector[sectors->sector_count++];
    sector->key = key;
    sector->multiplicity = 1U;
    memcpy(sector->choice, choice, sizeof(sector->choice));
}

static void m4_enumerate_gadgets(M4Sectors *sectors, int middle,
                                 uint16_t choice[M4_MAX_K])
{
    int k = sectors->k;
    if (middle == k) {
        m4_add_sector(sectors, choice);
        return;
    }
    uint16_t neighbors = 0;
    for (int left = 0; left < middle; ++left) {
        neighbors |= (uint16_t)(1U << left);
    }
    for (int right = middle; right < k - 1; ++right) {
        int local = (k - 1) + right;
        neighbors |= (uint16_t)(1U << local);
    }
    for (int first = 0; first < sectors->gamma_count; ++first) {
        uint16_t first_bit = (uint16_t)(1U << first);
        if ((neighbors & first_bit) == 0U) continue;
        choice[middle] = first_bit;
        m4_enumerate_gadgets(sectors, middle + 1, choice);
        for (int second = first + 1;
             second < sectors->gamma_count; ++second) {
            uint16_t second_bit = (uint16_t)(1U << second);
            if ((neighbors & second_bit) == 0U) continue;
            choice[middle] = (uint16_t)(first_bit | second_bit);
            m4_enumerate_gadgets(sectors, middle + 1, choice);
        }
    }
}

static M4Sectors m4_make_sectors(int k)
{
    M4Sectors sectors;
    memset(&sectors, 0, sizeof(sectors));
    sectors.k = k;
    sectors.gamma_count = 2 * (k - 1);
    uint16_t choice[M4_MAX_K] = {0};
    m4_enumerate_gadgets(&sectors, 0, choice);
    /* Regression values for exactly the syntactic filters in m4_trace_key.
     * They deliberately retain sectors whose completion count may later be
     * zero; deleting those would require an additional completion argument. */
    if ((k == 3 && (sectors.valid_gadgets != 16U ||
                    sectors.sector_count != 14U)) ||
        (k == 4 && (sectors.valid_gadgets != 511U ||
                    sectors.sector_count != 49U)) ||
        (k == 5 && (sectors.valid_gadgets != 26452U ||
                    sectors.sector_count != 120U))) {
        fprintf(stderr,
                "error: k=%d has %" PRIu64 " valid gadgets and %zu traces\n",
                k, sectors.valid_gadgets, sectors.sector_count);
        die("04 gadget/trace classification check failed");
    }
    return sectors;
}

static void m4_context_init(M4DpContext *context, int total,
                            uint64_t memory_mib)
{
    memset(context, 0, sizeof(*context));
    context->total = total;
    context->budget.limit = checked_product_size(
        (size_t)memory_mib, (size_t)UINT64_C(1048576));
    make_binomials(context->choose, total);
    context->rank = make_subset_ranks(
        total, context->choose, &context->budget, &context->rank_entries);
}

static void m4_context_destroy(M4DpContext *context)
{
    budget_free_array(&context->budget, context->rank,
                      context->rank_entries, sizeof(*context->rank));
    context->rank = NULL;
    if (context->budget.in_use != 0U) {
        die("04 internal DP memory leak");
    }
}

static bool m4_count_is_zero(Count128 value)
{
    return value.low == 0U && value.high == 0U;
}

/* Exact arbitrary-graph Held--Karp DP with a permitted-endpoint mask. */
static Count128 m4_count_graph(M4DpContext *context,
                               const uint64_t adjacency[M4_MAX_TOTAL],
                               uint64_t endpoint_allowed)
{
    int total = context->total;
    size_t current_count = layer_state_count(
        total, 1, context->choose);
    Count128 *current = budget_calloc_array(
        &context->budget, current_count, sizeof(*current));
    for (int vertex = 0; vertex < total; ++vertex) {
        if ((endpoint_allowed & (UINT64_C(1) << (unsigned)vertex)) != 0U) {
            current[vertex] = count_from_u64(UINT64_C(1));
        }
    }

    for (int cardinality = 1; cardinality < total; ++cardinality) {
        size_t next_count = layer_state_count(
            total, cardinality + 1, context->choose);
        Count128 *next = budget_calloc_array(
            &context->budget, next_count, sizeof(*next));
        uint64_t subset = first_mask_with_popcount(cardinality);
        uint64_t limit = UINT64_C(1) << (unsigned)total;
        while (subset < limit) {
            size_t base = checked_product_size(
                (size_t)context->rank[(size_t)subset],
                (size_t)cardinality);
            bool reachable = false;
            for (int position = 0; position < cardinality; ++position) {
                if (!m4_count_is_zero(current[base + (size_t)position])) {
                    reachable = true;
                    break;
                }
            }
            if (reachable) {
                uint64_t unused = (limit - UINT64_C(1)) & ~subset;
                while (unused != 0U) {
                    uint64_t bit = unused & (UINT64_C(0)-unused);
                    unused ^= bit;
                    unsigned next_vertex = bit_index(bit);
                    Count128 value = count_from_u64(UINT64_C(0));
                    uint64_t predecessors = adjacency[next_vertex] & subset;
                    while (predecessors != 0U) {
                        uint64_t previous_bit =
                            predecessors & (UINT64_C(0)-predecessors);
                        predecessors ^= previous_bit;
                        unsigned previous = bit_index(previous_bit);
                        unsigned position =
                            endpoint_position(subset, previous);
                        count_add_to(&value,
                                     current[base + (size_t)position]);
                        m4_increment_u64(&context->transitions,
                                         "graph transition");
                    }
                    if (!m4_count_is_zero(value)) {
                        uint64_t new_subset = subset | bit;
                        unsigned new_position =
                            endpoint_position(new_subset, next_vertex);
                        size_t next_base = checked_product_size(
                            (size_t)context->rank[(size_t)new_subset],
                            (size_t)(cardinality + 1));
                        size_t index = checked_add_size(
                            next_base, (size_t)new_position);
                        if (index >= next_count) {
                            die("04 next-state index out of range");
                        }
                        next[index] = value;
                        m4_increment_u64(&context->states, "graph state");
                    }
                }
            }
            uint64_t following = next_fixed_popcount_mask(subset);
            if (following <= subset || following >= limit) break;
            subset = following;
        }
        budget_free_array(&context->budget, current,
                          current_count, sizeof(*current));
        current = next;
        current_count = next_count;
    }

    Count128 result = count_from_u64(UINT64_C(0));
    for (int endpoint = 0; endpoint < total; ++endpoint) {
        if ((endpoint_allowed & (UINT64_C(1) << (unsigned)endpoint)) != 0U) {
            count_add_to(&result, current[endpoint]);
        }
    }
    budget_free_array(&context->budget, current,
                      current_count, sizeof(*current));
    return result;
}

static void m4_build_graph(int n, int k,
                           const uint16_t choice[M4_MAX_K],
                           uint64_t adjacency[M4_MAX_TOTAL],
                           uint64_t *endpoint_allowed)
{
    int total = 2 * n + k;
    memset(adjacency, 0, sizeof(uint64_t) * M4_MAX_TOTAL);
    for (int left = 0; left < n; ++left) {
        for (int right = 0; right < n; ++right) {
            if (left + 1 < k + right + 1) {
                int right_vertex = n + k + right;
                adjacency[left] |= UINT64_C(1) << (unsigned)right_vertex;
                adjacency[right_vertex] |= UINT64_C(1) << (unsigned)left;
            }
        }
    }
    uint64_t permitted = (UINT64_C(1) << (unsigned)total)-UINT64_C(1);
    int degree_one_middle = 0;
    for (int s = 0; s < k; ++s) {
        int middle_vertex = n + s;
        unsigned middle_degree = m4_popcount(choice[s]);
        if (middle_degree == 1U) ++degree_one_middle;
        if (middle_degree == 2U) {
            permitted &= ~(UINT64_C(1) << (unsigned)middle_vertex);
        }
        uint16_t neighbors = choice[s];
        while (neighbors != 0U) {
            uint16_t bit = (uint16_t)(neighbors & (uint16_t)(0U-neighbors));
            neighbors = (uint16_t)(neighbors ^ bit);
#if defined(__GNUC__) || defined(__clang__)
            int local = __builtin_ctz((unsigned)bit);
#else
            int local = 0;
            while ((bit >> local) != 1U) ++local;
#endif
            int gamma_vertex = local < k - 1
                                   ? local
                                   : 2 * n + 1 + (local-(k-1));
            adjacency[middle_vertex] |=
                UINT64_C(1) << (unsigned)gamma_vertex;
            adjacency[gamma_vertex] |=
                UINT64_C(1) << (unsigned)middle_vertex;
        }
    }
    if (degree_one_middle > 2) {
        permitted = 0U;
    }
    *endpoint_allowed = permitted;
}

static Count128 m4_multiply_small(Count128 value, uint64_t multiplier)
{
    uint128_t wide = count_to_u128(value);
    uint128_t maximum = ~(uint128_t)0;
    if (multiplier != 0U && wide > maximum/(uint128_t)multiplier) {
        die("04 unsigned 128-bit product overflow");
    }
    wide *= (uint128_t)multiplier;
    Count128 result = {(uint64_t)wide, (uint64_t)(wide >> 64U)};
    return result;
}

static int m4_total(int n, int k)
{
    if (n < 0 || k < M4_MIN_K || k > M4_MAX_K ||
        n > (INT_MAX-k)/2) {
        die("04 invalid n or k");
    }
    int total = 2*n+k;
    if (total > M4_MAX_TOTAL) {
        fprintf(stderr,
                "error: 04 requires 2*n+k <= %d; got n=%d,k=%d,total=%d\n",
                M4_MAX_TOTAL, n, k, total);
        exit(EXIT_FAILURE);
    }
    return total;
}

static const char *m4_known(int k, int n)
{
    for (size_t i = 0; i < M4_ARRAY_COUNT(m4_known_family); ++i) {
        if (m4_known_family[i].k == k && n >= 0 &&
            (size_t)n < m4_known_family[i].count) {
            return m4_known_family[i].term[n];
        }
    }
    return NULL;
}

static void m4_verify(Count128 value, int n, int k, bool independent)
{
    int total = m4_total(n, k);
    const char *known = m4_known(k, n);
    if (known != NULL && !count_equal(value, parse_count(known))) {
        fprintf(stderr, "error: 04 known mismatch at n=%d,k=%d\n", n, k);
        exit(EXIT_FAILURE);
    }
    if (total > 1 && (value.low & UINT64_C(1)) != 0U) {
        die("04 reversal parity failed");
    }
    if (!independent) return;
    if (total <= DIRECT_CHECK_MAX_N) {
        Count128 direct = direct_count(total, n+1);
        if (!count_equal(value, direct)) {
            die("04 direct-permutation check failed");
        }
    }
    if (total <= INDEPENDENT_CHECK_MAX_N) {
        Count128 forward = count_from_u64(
            independent_forward_dp(total, n+1));
        if (!count_equal(value, forward)) {
            die("04 independent forward-DP check failed");
        }
    }
}

static Count128 m4_value(int n, int k, const M4Sectors *sectors,
                         uint64_t memory_mib, M4Stats *stats)
{
    double started = monotonic_seconds();
    memset(stats, 0, sizeof(*stats));
    int total = m4_total(n, k);
    if (n < k-1) {
        DpStats dense_stats;
        Count128 value = compute_dp(total, n+1, memory_mib, &dense_stats);
        stats->states = dense_stats.states;
        stats->peak_bytes = dense_stats.peak_bytes;
        stats->seconds = monotonic_seconds()-started;
        m4_verify(value, n, k, false);
        return value;
    }

    M4DpContext context;
    m4_context_init(&context, total, memory_mib);
    Count128 result = count_from_u64(UINT64_C(0));
    for (size_t i = 0; i < sectors->sector_count; ++i) {
        uint64_t adjacency[M4_MAX_TOTAL] = {0};
        uint64_t endpoint_allowed;
        m4_build_graph(n, k, sectors->sector[i].choice,
                       adjacency, &endpoint_allowed);
        if (endpoint_allowed == 0U) continue;
        Count128 representative = m4_count_graph(
            &context, adjacency, endpoint_allowed);
        Count128 contribution = m4_multiply_small(
            representative, sectors->sector[i].multiplicity);
        count_add_to(&result, contribution);
    }
    stats->used_gadgets = true;
    stats->sectors = sectors->sector_count;
    stats->valid_gadgets = sectors->valid_gadgets;
    stats->states = context.states;
    stats->transitions = context.transitions;
    stats->peak_bytes = context.budget.peak;
    m4_context_destroy(&context);
    stats->seconds = monotonic_seconds()-started;
    m4_verify(result, n, k, false);
    return result;
}

static int m4_parse_nonnegative(const char *text, const char *label,
                                int maximum)
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

static int m4_parse_k(const char *text)
{
    int k = m4_parse_nonnegative(text, "K", M4_MAX_K);
    if (k < M4_MIN_K) {
        fprintf(stderr, "error: K must be in %d..%d: %s\n",
                M4_MIN_K, M4_MAX_K, text);
        exit(EXIT_FAILURE);
    }
    return k;
}

static void m4_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K] [--memory-mib M]\n"
            "       %s --upto MAX_N [--k K] [--memory-mib M]\n"
            "       %s --term N [--k K] [--memory-mib M]\n"
            "       %s --check [MAX_N] [--k K] [--memory-mib M]\n\n"
            "Proved gadget/trace computation; no conjectural recurrence.\n"
            "Requires K=%d..%d and 2*N+K<=%d. Defaults: K=%d, N=%d, "
            "memory=%" PRIu64 " MiB.\n",
            program, program, program, program,
            M4_MIN_K, M4_MAX_K, M4_MAX_TOTAL,
            M4_DEFAULT_K, M4_DEFAULT_N, DEFAULT_MEMORY_MIB);
}

static M4Options m4_parse_options(int argc, char **argv)
{
    M4Options options;
    options.k = M4_DEFAULT_K;
    options.maximum_n = M4_DEFAULT_N;
    options.memory_mib = DEFAULT_MEMORY_MIB;
    options.mode = M4_RUN_UPTO;
    bool have_k = false, have_n = false, have_memory = false, have_mode=false;
    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--help") == 0 || strcmp(text, "-h") == 0) {
            m4_usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(text, "--k") == 0) {
            if (have_k || argument+1 >= argc) {
                m4_usage(argv[0]); exit(EXIT_FAILURE);
            }
            options.k = m4_parse_k(argv[++argument]);
            have_k = true;
        } else if (strcmp(text, "--memory-mib") == 0) {
            if (have_memory || argument+1 >= argc) {
                m4_usage(argv[0]); exit(EXIT_FAILURE);
            }
            options.memory_mib = parse_memory_mib(argv[++argument]);
            have_memory = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument+1 >= argc) {
                m4_usage(argv[0]); exit(EXIT_FAILURE);
            }
            options.mode = strcmp(text, "--term") == 0
                               ? M4_RUN_TERM : M4_RUN_UPTO;
            options.maximum_n = m4_parse_nonnegative(
                argv[++argument],
                options.mode == M4_RUN_TERM ? "N" : "MAX_N",
                M4_MAX_TOTAL);
            have_mode = true; have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode) { m4_usage(argv[0]); exit(EXIT_FAILURE); }
            options.mode = M4_RUN_CHECK;
            options.maximum_n = M4_DEFAULT_CHECK_N;
            have_mode = true;
            if (argument+1 < argc && argv[argument+1][0] != '-') {
                options.maximum_n = m4_parse_nonnegative(
                    argv[++argument], "CHECK_N", M4_MAX_TOTAL);
                have_n = true;
            }
        } else if (text[0] != '-') {
            if (have_n) { m4_usage(argv[0]); exit(EXIT_FAILURE); }
            options.maximum_n = m4_parse_nonnegative(
                text, "MAX_N", M4_MAX_TOTAL);
            have_n = true;
        } else {
            m4_usage(argv[0]); exit(EXIT_FAILURE);
        }
    }
    (void)m4_total(options.maximum_n, options.k);
    return options;
}

static void m4_write_line(FILE *stream, int n, Count128 value)
{
    if (fprintf(stream, "%d ", n) < 0 ||
        fprint_count(stream, value) != 0 || fputc('\n', stream) == EOF) {
        die("04 output write failed");
    }
}

static void m4_report(int n, int k, const M4Stats *stats)
{
    if (stats->used_gadgets) {
        fprintf(stderr,
                "179962_04: n=%d,k=%d, proved gadget/trace DP, "
                "gadgets=%" PRIu64 ", sectors=%zu, states=%" PRIu64
                ", transitions=%" PRIu64 ", peak memory=%.1f MiB, %.3f s\n",
                n, k, stats->valid_gadgets, stats->sectors,
                stats->states, stats->transitions,
                (double)stats->peak_bytes/1048576.0, stats->seconds);
    } else {
        fprintf(stderr,
                "179962_04: n=%d,k=%d, n<k-1 definition-DP fallback, "
                "states=%" PRIu64 ", peak memory=%.1f MiB, %.3f s\n",
                n, k, stats->states,
                (double)stats->peak_bytes/1048576.0, stats->seconds);
    }
}

static void m4_run_check(const M4Options *options, const M4Sectors *sectors)
{
    for (int n = 0; n <= options->maximum_n; ++n) {
        M4Stats stats;
        Count128 value = m4_value(n, options->k, sectors,
                                  options->memory_mib, &stats);
        int total = m4_total(n, options->k);
        DpStats dense_stats;
        Count128 dense = compute_dp(total, n+1,
                                    options->memory_mib, &dense_stats);
        if (!count_equal(value, dense)) {
            fprintf(stderr,
                    "error: 04 gadget/definition mismatch at n=%d,k=%d\n",
                    n, options->k);
            exit(EXIT_FAILURE);
        }
        m4_verify(value, n, options->k, true);
    }
    printf("ok: proved gadget/trace sum = definition DP; known values, "
           "direct permutations through total %d, independent forward DP "
           "through total %d; n=0..%d,k=%d\n",
           DIRECT_CHECK_MAX_N, INDEPENDENT_CHECK_MAX_N,
           options->maximum_n, options->k);
}

int main(int argc, char **argv)
{
    (void)&m4_imported_definition_main;
    M4Options options = m4_parse_options(argc, argv);
    M4Sectors sectors = m4_make_sectors(options.k);
    if (options.mode == M4_RUN_CHECK) {
        m4_run_check(&options, &sectors);
        return EXIT_SUCCESS;
    }
    if (options.mode == M4_RUN_TERM) {
        M4Stats stats;
        Count128 value = m4_value(options.maximum_n, options.k, &sectors,
                                  options.memory_mib, &stats);
        m4_report(options.maximum_n, options.k, &stats);
        m4_write_line(stdout, options.maximum_n, value);
        return EXIT_SUCCESS;
    }

    char final_name[80], part_name[80];
    snprintf(final_name, sizeof(final_name),
             "b179962_04_k%d.txt", options.k);
    snprintf(part_name, sizeof(part_name),
             "b179962_04_k%d_part.txt", options.k);
    char *final_path = path_beside_executable(argv[0], final_name);
    char *part_path = path_beside_executable(argv[0], part_name);
    FILE *stream = fopen(part_path, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    for (int n = 0; n <= options.maximum_n; ++n) {
        M4Stats stats;
        Count128 value = m4_value(n, options.k, &sectors,
                                  options.memory_mib, &stats);
        m4_report(n, options.k, &stats);
        m4_write_line(stream, n, value);
        if (fflush(stream) != 0) die("04 partial output flush failed");
    }
    if (fclose(stream) != 0) die("04 output close failed");
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("wrote %s (n=0..%d,k=%d)\n",
           final_path, options.maximum_n, options.k);
    free(final_path); free(part_path);
    return EXIT_SUCCESS;
}
