/*
 * A179962 and the general 2*n+k family -- sparse symmetry-quotient DP.
 *
 * Count permutations p of [2*n+k] satisfying
 *
 *                   |p(i+1)-p(i)| > n.
 *
 * This is an exact definition-level algorithm, independent of the C-finite
 * recurrence in 179962_02.c.  It is also algorithmically different from
 * 179962_01.c:
 *
 *   01: dense cardinality-layer Held--Karp arrays; every (S,last) state is
 *       allocated and evaluated by total-minus-forbidden subtraction.
 *
 *   03: open-addressed sparse layers; only reachable (S,last) states are
 *       stored, and only allowed unused successors are enumerated.
 *       The automorphism v -> M-1-v is quotiented by canonicalizing every
 *       state, normally halving the number of stored states again.
 *
 * Let M=2*n+k, D=n+1, and let H(S,v) count valid orderings of S ending at
 * v.  The invariant and transition are
 *
 *   H({v},v)=1,
 *   H(S union {w},w) += H(S,v)  if w not in S and |w-v|>=D.
 *
 * Induction on |S| proves the invariant.  Summing H([M],v) over v counts
 * every required permutation once.  Canonicalization is exact because
 * value reflection is a graph automorphism: a transition and its reflected
 * transition always enter the same quotient state.  A reflection-fixed
 * state is handled correctly because its distinct reflected successors are
 * still enumerated separately.
 *
 * Worst-case complexity remains O(M^2*2^M) time and O(M*2^M) memory, but
 * the actual bounds are proportional to reachable states and transitions.
 * For this near-half minimum-jump family, this is normally substantially
 * smaller than 01.  Unlike 02 it is still exponential, so 03 is intended as
 * an independent fast reference and recurrence checker.
 *
 * Safety:
 *   - M<=34 and every prefix-state count is at most M! < 2^128;
 *   - all 128-bit additions, key operations, table growth, allocation-size
 *     arithmetic, and counters are checked;
 *   - --memory-mib is a hard table-allocation budget;
 *   - range output is written to _part.txt and renamed only after success.
 *
 * Verification:
 *   - supplied/OEIS prefixes for k=2,...,9 are embedded;
 *   - --check compares quotient and non-quotient sparse DP;
 *   - --check compares 03 with the dense 01 kernel;
 *   - direct permutation enumeration is used for M<=10;
 *   - a separately written forward dense DP is used for M<=16.
 *
 * Build (179957_01.c must be beside this file; it is check-only support):
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     179962_03.c -o 179962_03
 *
 * Examples:
 *
 *   ./179962_03 --k 4 --upto 10
 *   ./179962_03 --k 8 --term 7 --memory-mib 4096
 *   ./179962_03 --k 4 --check 8
 *   ./179962_03 --k 4 --no-symmetry --term 8
 */

#define main m3_imported_dense_check_main
#include "179957_01.c"
#undef main

#define M3_MIN_K 2
#define M3_MAX_K 34
#define M3_MAX_TOTAL 34
#define M3_DEFAULT_K 4
#define M3_DEFAULT_N 8
#define M3_DEFAULT_CHECK_N 8
#define M3_ENDPOINT_BITS 6U
#define M3_ENDPOINT_MASK UINT64_C(63)
#define M3_INITIAL_CAPACITY ((size_t)16)
#define M3_LOAD_NUMERATOR ((size_t)7)
#define M3_LOAD_DENOMINATOR ((size_t)10)
#define M3_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

typedef struct {
    uint64_t key_plus_one;
    Count128 count;
} M3Slot;

typedef struct {
    M3Slot *slot;
    size_t capacity;
    size_t size;
    MemoryBudget *budget;
} M3Map;

typedef struct {
    uint64_t transitions;
    size_t peak_states;
    size_t peak_bytes;
    double seconds;
} M3Stats;

typedef enum {
    M3_RUN_UPTO,
    M3_RUN_TERM,
    M3_RUN_CHECK
} M3Mode;

typedef struct {
    int k;
    int maximum_n;
    uint64_t memory_mib;
    bool use_symmetry;
    M3Mode mode;
} M3Options;

typedef struct {
    int k;
    const char *const *term;
    size_t count;
} M3KnownFamily;

static const char *const m3_known_k2[] = {
    "2", "2", "2", "2", "2", "2", "2", "2"
};
static const char *const m3_known_k3[] = {
    "6", "14", "32", "72", "160", "352", "768", "1664",
    "3584", "7680"
};
static const char *const m3_known_k4[] = {
    "24", "90", "368", "1496", "6056", "24440", "98408",
    "395576", "1588136", "6370040", "25532648", "102288056"
};
static const char *const m3_known_k5[] = {
    "120", "646", "3984", "25384", "161136", "1019616",
    "6433728", "40495488", "254319360", "1593945600"
};
static const char *const m3_known_k6[] = {
    "720", "5242", "44304", "399848", "3661824", "33461568",
    "305193408", "2779045440"
};
static const char *const m3_known_k7[] = {
    "5040", "47622", "521606", "6231544", "76972416",
    "957913824", "11902581120", "147689145984", "1830235823616"
};
static const char *const m3_known_k8[] = {
    "40320", "479306", "6564318", "99133496", "1572313392",
    "25415753280", "412583686272", "6690112284672",
    "108370981896192", "1753844521231872"
};
static const char *const m3_known_k9[] = {
    "362880", "5296790", "88422296", "1634227958",
    "32096768008", "649347224736"
};

static const M3KnownFamily m3_known_family[] = {
    {2, m3_known_k2, M3_ARRAY_COUNT(m3_known_k2)},
    {3, m3_known_k3, M3_ARRAY_COUNT(m3_known_k3)},
    {4, m3_known_k4, M3_ARRAY_COUNT(m3_known_k4)},
    {5, m3_known_k5, M3_ARRAY_COUNT(m3_known_k5)},
    {6, m3_known_k6, M3_ARRAY_COUNT(m3_known_k6)},
    {7, m3_known_k7, M3_ARRAY_COUNT(m3_known_k7)},
    {8, m3_known_k8, M3_ARRAY_COUNT(m3_known_k8)},
    {9, m3_known_k9, M3_ARRAY_COUNT(m3_known_k9)}
};

static size_t m3_checked_product(size_t left, size_t right)
{
    if (left != 0U && right > SIZE_MAX / left) {
        die("03 allocation-size overflow");
    }
    return left * right;
}

static void m3_checked_increment(uint64_t *value, const char *label)
{
    if (*value == UINT64_MAX) {
        fprintf(stderr, "error: 03 %s counter overflow\n", label);
        exit(EXIT_FAILURE);
    }
    ++*value;
}

static M3Slot *m3_allocate_slots(MemoryBudget *budget, size_t capacity)
{
    size_t bytes = m3_checked_product(capacity, sizeof(M3Slot));
    budget_reserve(budget, bytes);
    M3Slot *slot = calloc(capacity, sizeof(*slot));
    if (slot == NULL) {
        fprintf(stderr,
                "error: cannot allocate %.1f MiB for 03 sparse table\n",
                (double)bytes / 1048576.0);
        exit(EXIT_FAILURE);
    }
    return slot;
}

static void m3_free_slots(MemoryBudget *budget, M3Slot *slot,
                          size_t capacity)
{
    size_t bytes = m3_checked_product(capacity, sizeof(M3Slot));
    free(slot);
    budget_release(budget, bytes);
}

static uint64_t m3_mix(uint64_t value)
{
    value ^= value >> 30U;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27U;
    value *= UINT64_C(0x94d049bb133111eb);
    value ^= value >> 31U;
    return value;
}

static void m3_map_init(M3Map *map, MemoryBudget *budget)
{
    map->capacity = M3_INITIAL_CAPACITY;
    map->size = 0U;
    map->budget = budget;
    map->slot = m3_allocate_slots(budget, map->capacity);
}

static void m3_map_destroy(M3Map *map)
{
    if (map->slot != NULL) {
        m3_free_slots(map->budget, map->slot, map->capacity);
    }
    map->slot = NULL;
    map->capacity = 0U;
    map->size = 0U;
}

static M3Slot *m3_map_find(M3Map *map, uint64_t key_plus_one)
{
    size_t index = (size_t)m3_mix(key_plus_one) & (map->capacity - 1U);
    for (;;) {
        M3Slot *slot = &map->slot[index];
        if (slot->key_plus_one == 0U ||
            slot->key_plus_one == key_plus_one) {
            return slot;
        }
        index = (index + 1U) & (map->capacity - 1U);
    }
}

static void m3_map_grow(M3Map *map)
{
    if (map->capacity > SIZE_MAX / 2U) {
        die("03 hash-table capacity overflow");
    }
    size_t old_capacity = map->capacity;
    M3Slot *old_slot = map->slot;
    size_t new_capacity = old_capacity * 2U;
    map->slot = m3_allocate_slots(map->budget, new_capacity);
    map->capacity = new_capacity;
    for (size_t i = 0; i < old_capacity; ++i) {
        if (old_slot[i].key_plus_one != 0U) {
            M3Slot *destination =
                m3_map_find(map, old_slot[i].key_plus_one);
            *destination = old_slot[i];
        }
    }
    m3_free_slots(map->budget, old_slot, old_capacity);
}

static void m3_map_add(M3Map *map, uint64_t packed_key,
                       Count128 addend)
{
    if (packed_key == UINT64_MAX) {
        die("03 packed-key overflow");
    }
    uint64_t key_plus_one = packed_key + UINT64_C(1);
    M3Slot *slot = m3_map_find(map, key_plus_one);
    if (slot->key_plus_one != 0U) {
        count_add_to(&slot->count, addend);
        return;
    }
    if (map->size + 1U >
        (map->capacity / M3_LOAD_DENOMINATOR) * M3_LOAD_NUMERATOR) {
        m3_map_grow(map);
        slot = m3_map_find(map, key_plus_one);
        if (slot->key_plus_one != 0U) {
            die("03 duplicate state after table growth");
        }
    }
    slot->key_plus_one = key_plus_one;
    slot->count = addend;
    ++map->size;
}

static void m3_map_clear(M3Map *map)
{
    memset(map->slot, 0,
           m3_checked_product(map->capacity, sizeof(M3Slot)));
    map->size = 0U;
}

static uint64_t m3_reverse_bits(uint64_t value)
{
    value = ((value & UINT64_C(0x5555555555555555)) << 1U) |
            ((value >> 1U) & UINT64_C(0x5555555555555555));
    value = ((value & UINT64_C(0x3333333333333333)) << 2U) |
            ((value >> 2U) & UINT64_C(0x3333333333333333));
    value = ((value & UINT64_C(0x0f0f0f0f0f0f0f0f)) << 4U) |
            ((value >> 4U) & UINT64_C(0x0f0f0f0f0f0f0f0f));
    value = ((value & UINT64_C(0x00ff00ff00ff00ff)) << 8U) |
            ((value >> 8U) & UINT64_C(0x00ff00ff00ff00ff));
    value = ((value & UINT64_C(0x0000ffff0000ffff)) << 16U) |
            ((value >> 16U) & UINT64_C(0x0000ffff0000ffff));
    return (value << 32U) | (value >> 32U);
}

static uint64_t m3_reverse_low(uint64_t mask, int total)
{
    return m3_reverse_bits(mask) >> (64U - (unsigned)total);
}

static uint64_t m3_pack(uint64_t used, unsigned last)
{
    return (used << M3_ENDPOINT_BITS) | (uint64_t)last;
}

static void m3_unpack(uint64_t packed, uint64_t *used, unsigned *last)
{
    *last = (unsigned)(packed & M3_ENDPOINT_MASK);
    *used = packed >> M3_ENDPOINT_BITS;
}

static uint64_t m3_canonical(uint64_t used, unsigned last, int total,
                             bool use_symmetry)
{
    uint64_t original = m3_pack(used, last);
    if (!use_symmetry) {
        return original;
    }
    uint64_t reflected = m3_pack(
        m3_reverse_low(used, total),
        (unsigned)(total - 1 - (int)last));
    return original < reflected ? original : reflected;
}

static void m3_update_peak(M3Stats *stats, const M3Map *map,
                           const MemoryBudget *budget)
{
    if (stats->peak_states < map->size) {
        stats->peak_states = map->size;
    }
    if (stats->peak_bytes < budget->peak) {
        stats->peak_bytes = budget->peak;
    }
}

static Count128 m3_compute(int total, int threshold, bool use_symmetry,
                           uint64_t memory_mib, M3Stats *stats)
{
    double started = monotonic_seconds();
    memset(stats, 0, sizeof(*stats));
    if (total == 0 || total == 1) {
        stats->seconds = monotonic_seconds() - started;
        return count_from_u64(UINT64_C(1));
    }
    if (total < 0 || total > M3_MAX_TOTAL ||
        threshold < 1 || threshold >= total) {
        die("03 internal total/threshold range error");
    }

    MemoryBudget budget;
    budget.limit = m3_checked_product((size_t)memory_mib,
                                      (size_t)UINT64_C(1048576));
    budget.in_use = 0U;
    budget.peak = 0U;

    uint64_t full = (UINT64_C(1) << (unsigned)total) - UINT64_C(1);
    uint64_t allowed[M3_MAX_TOTAL] = {0};
    for (int value = 0; value < total; ++value) {
        for (int other = 0; other < total; ++other) {
            int difference = value - other;
            if (difference <= -threshold || difference >= threshold) {
                allowed[value] |= UINT64_C(1) << (unsigned)other;
            }
        }
    }

    M3Map current;
    M3Map next;
    m3_map_init(&current, &budget);
    m3_map_init(&next, &budget);
    Count128 one = count_from_u64(UINT64_C(1));
    for (int value = 0; value < total; ++value) {
        uint64_t used = UINT64_C(1) << (unsigned)value;
        m3_map_add(&current,
                   m3_canonical(used, (unsigned)value, total,
                                use_symmetry),
                   one);
    }
    m3_update_peak(stats, &current, &budget);

    for (int length = 1;
         length < total && current.size != 0U;
         ++length) {
        m3_map_clear(&next);
        for (size_t index = 0; index < current.capacity; ++index) {
            const M3Slot *slot = &current.slot[index];
            if (slot->key_plus_one == 0U) {
                continue;
            }
            uint64_t used;
            unsigned last;
            m3_unpack(slot->key_plus_one - UINT64_C(1), &used, &last);
            if (last >= (unsigned)total ||
                (used & (UINT64_C(1) << last)) == 0U) {
                die("03 corrupt sparse state");
            }
            uint64_t candidates = allowed[last] & (full ^ used);
            while (candidates != 0U) {
                uint64_t bit = candidates & (UINT64_C(0) - candidates);
                candidates ^= bit;
                unsigned successor = bit_index(bit);
                uint64_t new_used = used | bit;
                uint64_t key = m3_canonical(
                    new_used, successor, total, use_symmetry);
                m3_map_add(&next, key, slot->count);
                m3_checked_increment(&stats->transitions, "transition");
            }
        }
        M3Map temporary = current;
        current = next;
        next = temporary;
        m3_update_peak(stats, &current, &budget);
    }

    Count128 result = count_from_u64(UINT64_C(0));
    for (size_t index = 0; index < current.capacity; ++index) {
        if (current.slot[index].key_plus_one != 0U) {
            count_add_to(&result, current.slot[index].count);
        }
    }
    stats->peak_bytes = budget.peak;
    m3_map_destroy(&current);
    m3_map_destroy(&next);
    if (budget.in_use != 0U) {
        die("03 internal DP memory leak");
    }
    stats->seconds = monotonic_seconds() - started;
    return result;
}

static int m3_total(int n, int k)
{
    if (n < 0 || k < M3_MIN_K || n > (INT_MAX - k) / 2) {
        die("03 invalid n or k");
    }
    int total = 2 * n + k;
    if (total > M3_MAX_TOTAL) {
        fprintf(stderr,
                "error: 03 requires 2*n+k <= %d; got n=%d, k=%d, "
                "total=%d\n",
                M3_MAX_TOTAL, n, k, total);
        exit(EXIT_FAILURE);
    }
    return total;
}

static const char *m3_known(int k, int n)
{
    for (size_t i = 0; i < M3_ARRAY_COUNT(m3_known_family); ++i) {
        if (m3_known_family[i].k == k && n >= 0 &&
            (size_t)n < m3_known_family[i].count) {
            return m3_known_family[i].term[n];
        }
    }
    return NULL;
}

static void m3_verify(Count128 value, int n, int k, bool independent)
{
    int total = m3_total(n, k);
    int threshold = n + 1;
    const char *known = m3_known(k, n);
    if (known != NULL) {
        Count128 expected = parse_count(known);
        if (!count_equal(value, expected)) {
            fprintf(stderr, "error: 03 known mismatch at n=%d,k=%d\n",
                    n, k);
            exit(EXIT_FAILURE);
        }
    }
    if (total > 1 && (value.low & UINT64_C(1)) != 0U) {
        die("03 reversal-parity check failed");
    }
    if (!independent) {
        return;
    }
    if (total <= DIRECT_CHECK_MAX_N) {
        Count128 direct = direct_count(total, threshold);
        if (!count_equal(value, direct)) {
            die("03 direct-permutation check failed");
        }
    }
    if (total <= INDEPENDENT_CHECK_MAX_N) {
        Count128 forward =
            count_from_u64(independent_forward_dp(total, threshold));
        if (!count_equal(value, forward)) {
            die("03 independent forward-DP check failed");
        }
    }
}

static int m3_parse_nonnegative(const char *text, const char *label,
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

static int m3_parse_k(const char *text)
{
    int k = m3_parse_nonnegative(text, "K", M3_MAX_K);
    if (k < M3_MIN_K) {
        fprintf(stderr, "error: K must be in %d..%d: %s\n",
                M3_MIN_K, M3_MAX_K, text);
        exit(EXIT_FAILURE);
    }
    return k;
}

static void m3_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K] [--memory-mib M] "
            "[--no-symmetry]\n"
            "       %s --upto MAX_N [--k K] [--memory-mib M] "
            "[--no-symmetry]\n"
            "       %s --term N [--k K] [--memory-mib M] "
            "[--no-symmetry]\n"
            "       %s --check [MAX_N] [--k K] [--memory-mib M]\n"
            "\n"
            "Exact sparse DP for permutations of [2*n+k] with adjacent "
            "differences > n.\n"
            "Requires 2*n+k<=%d; defaults: k=%d, upto=%d, memory=%" PRIu64
            " MiB.\n",
            program, program, program, program, M3_MAX_TOTAL,
            M3_DEFAULT_K, M3_DEFAULT_N, DEFAULT_MEMORY_MIB);
}

static M3Options m3_parse_options(int argc, char **argv)
{
    M3Options options;
    options.k = M3_DEFAULT_K;
    options.maximum_n = M3_DEFAULT_N;
    options.memory_mib = DEFAULT_MEMORY_MIB;
    options.use_symmetry = true;
    options.mode = M3_RUN_UPTO;
    bool have_k = false;
    bool have_n = false;
    bool have_memory = false;
    bool have_mode = false;
    bool have_no_symmetry = false;

    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--help") == 0 || strcmp(text, "-h") == 0) {
            m3_usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(text, "--k") == 0) {
            if (have_k || argument + 1 >= argc) {
                m3_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.k = m3_parse_k(argv[++argument]);
            have_k = true;
        } else if (strcmp(text, "--memory-mib") == 0) {
            if (have_memory || argument + 1 >= argc) {
                m3_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.memory_mib = parse_memory_mib(argv[++argument]);
            have_memory = true;
        } else if (strcmp(text, "--no-symmetry") == 0) {
            if (have_no_symmetry || options.mode == M3_RUN_CHECK) {
                m3_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.use_symmetry = false;
            have_no_symmetry = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                m3_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.mode = strcmp(text, "--term") == 0
                               ? M3_RUN_TERM : M3_RUN_UPTO;
            options.maximum_n = m3_parse_nonnegative(
                argv[++argument],
                options.mode == M3_RUN_TERM ? "N" : "MAX_N",
                M3_MAX_TOTAL);
            have_mode = true;
            have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode) {
                m3_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.mode = M3_RUN_CHECK;
            options.maximum_n = M3_DEFAULT_CHECK_N;
            options.use_symmetry = true;
            have_mode = true;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                options.maximum_n = m3_parse_nonnegative(
                    argv[++argument], "CHECK_N", M3_MAX_TOTAL);
                have_n = true;
            }
        } else if (text[0] != '-') {
            if (have_n) {
                m3_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.maximum_n = m3_parse_nonnegative(
                text, "MAX_N", M3_MAX_TOTAL);
            have_n = true;
        } else {
            m3_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (options.mode == M3_RUN_CHECK && have_no_symmetry) {
        m3_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    (void)m3_total(options.maximum_n, options.k);
    return options;
}

static Count128 m3_value(int n, int k, bool use_symmetry,
                         uint64_t memory_mib, bool verbose,
                         M3Stats *stats)
{
    int total = m3_total(n, k);
    Count128 value = m3_compute(total, n + 1, use_symmetry,
                                memory_mib, stats);
    m3_verify(value, n, k, false);
    if (verbose) {
        fprintf(stderr,
                "179962_03: n=%d, k=%d, total=%d, sparse %s DP, "
                "peak states=%zu, transitions=%" PRIu64
                ", peak table memory=%.1f MiB, %.3f s\n",
                n, k, total,
                use_symmetry ? "reflection-quotient" : "plain",
                stats->peak_states, stats->transitions,
                (double)stats->peak_bytes / 1048576.0, stats->seconds);
    }
    return value;
}

static void m3_write_line(FILE *stream, int n, Count128 value)
{
    if (fprintf(stream, "%d ", n) < 0 ||
        fprint_count(stream, value) != 0 || fputc('\n', stream) == EOF) {
        die("03 output write failed");
    }
}

static void m3_run_term(const M3Options *options)
{
    M3Stats stats;
    Count128 value = m3_value(options->maximum_n, options->k,
                              options->use_symmetry,
                              options->memory_mib, true, &stats);
    m3_write_line(stdout, options->maximum_n, value);
}

static void m3_run_upto(const M3Options *options, const char *argv0)
{
    char final_name[80];
    char part_name[80];
    int final_length = snprintf(final_name, sizeof(final_name),
                                "b179962_03_k%d.txt", options->k);
    int part_length = snprintf(part_name, sizeof(part_name),
                               "b179962_03_k%d_part.txt", options->k);
    if (final_length < 0 || (size_t)final_length >= sizeof(final_name) ||
        part_length < 0 || (size_t)part_length >= sizeof(part_name)) {
        die("03 output filename overflow");
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
    for (int n = 0; n <= options->maximum_n; ++n) {
        M3Stats stats;
        Count128 value = m3_value(n, options->k,
                                  options->use_symmetry,
                                  options->memory_mib, true, &stats);
        m3_write_line(stream, n, value);
        if (fflush(stream) != 0) {
            die("03 partial output flush failed");
        }
    }
    if (fclose(stream) != 0) {
        die("03 output close failed");
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }
    printf("wrote %s (n=0..%d, k=%d)\n",
           final_path, options->maximum_n, options->k);
    free(final_path);
    free(part_path);
}

static void m3_run_check(const M3Options *options)
{
    for (int n = 0; n <= options->maximum_n; ++n) {
        int total = m3_total(n, options->k);
        int threshold = n + 1;
        M3Stats quotient_stats;
        M3Stats plain_stats;
        Count128 quotient = m3_compute(total, threshold, true,
                                       options->memory_mib,
                                       &quotient_stats);
        Count128 plain = m3_compute(total, threshold, false,
                                    options->memory_mib, &plain_stats);
        if (!count_equal(quotient, plain)) {
            fprintf(stderr,
                    "error: 03 quotient/plain mismatch at n=%d,k=%d\n",
                    n, options->k);
            exit(EXIT_FAILURE);
        }
        DpStats dense_stats;
        Count128 dense = compute_dp(total, threshold,
                                    options->memory_mib, &dense_stats);
        if (!count_equal(quotient, dense)) {
            fprintf(stderr,
                    "error: 03 sparse/dense-01 mismatch at n=%d,k=%d\n",
                    n, options->k);
            exit(EXIT_FAILURE);
        }
        m3_verify(quotient, n, options->k, true);
    }
    printf("ok: 03 reflection quotient = plain sparse DP = dense 01 DP; "
           "known values, direct permutations through total %d, and "
           "independent forward DP through total %d; n=0..%d, k=%d\n",
           DIRECT_CHECK_MAX_N, INDEPENDENT_CHECK_MAX_N,
           options->maximum_n, options->k);
}

int main(int argc, char **argv)
{
    (void)&m3_imported_dense_check_main;
    M3Options options = m3_parse_options(argc, argv);
    if (options.mode == M3_RUN_CHECK) {
        m3_run_check(&options);
    } else if (options.mode == M3_RUN_TERM) {
        m3_run_term(&options);
    } else {
        m3_run_upto(&options, argv[0]);
    }
    return EXIT_SUCCESS;
}
