/*
 * A179957 (k=4) and its general threshold-k version -- exact layered
 * subset DP.
 *
 * Count permutations p of {1,...,n} satisfying
 *
 *                 |p(i)-p(i-1)| >= k,  i>1.
 *
 * Let D(S,v) be the number of valid orderings of the set S that end in v.
 * With T=S\{v}, the ordinary Held--Karp recurrence is
 *
 *        D(S,v) = sum_{u in T, |u-v|>=k} D(T,u).
 *
 * Put R(T)=sum_{u in T}D(T,u).  Differences 1,...,k-1 are forbidden, so
 *
 *        D(S,v) = R(T)
 *                 - sum_{u in T, 1<=|u-v|<k} D(T,u).
 *
 * The second sum has at most 2(k-1) terms.  For fixed k, all states are
 * computed in O(n*2^n) arithmetic operations instead of O(n^2*2^n).
 *
 * States are stored by subset cardinality.  For a k-set S, only its k
 * possible endpoints are allocated.  A precomputed colexicographic subset
 * rank and one popcount locate D(S,v):
 *
 *   index = rank(S)*k + popcount(S & ((1<<v)-1)).
 *
 * Only the current and next cardinality layers are alive.  There is no hash
 * table, no collision, and no zero-state overhead.  Every next state has the
 * unique predecessor subset T=S\{v}, so it is assigned exactly once.
 *
 * Safety:
 *   - every DP value and every within-subset endpoint total is <= n!;
 *   - 34! < 2^128, so unsigned 128-bit counts are exact for n<=34;
 *   - additions and subtractions are nevertheless checked;
 *   - all shifts, subset ranks, indices, products, and allocation sizes are
 *     checked;
 *   - --memory-mib is a hard budget for the rank and DP arrays;
 *   - b179957_01_kK_part.txt is flushed after every complete term and
 *     renamed atomically to b179957_01_kK.txt only after the range succeeds.
 *
 * The numeric type is safe through n=34.  The exponential arrays impose a
 * much smaller practical limit; approximately n=26 is the likely range for
 * a machine with about 6 GiB available to this process.
 *
 * Checks:
 *   - OEIS b-file values a(0)..a(21) are built in;
 *   - a definition-level full permutation scan is used through n=10;
 *   - a separate forward candidate-enumerating subset DP is used through
 *     n=16;
 *   - reversal parity 2|a(n), n>1, is checked.
 *
 * Reference:
 *   https://oeis.org/A179957
 *
 * Build:
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     179957_01.c -o 179957_01
 *
 * Usage:
 *
 *   ./179957_01 --k 4                    # write a(0)..a(21)
 *   ./179957_01 --k 5 --upto 24 --memory-mib 2048
 *   ./179957_01 --k 5 --term 24 --memory-mib 2048
 *   ./179957_01 --k 4 --check
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#if !defined(__SIZEOF_INT128__)
#error "179957_01 requires compiler support for unsigned __int128"
#endif

__extension__ typedef unsigned __int128 uint128_t;

#define SEQUENCE_OFFSET 0
#define DEFAULT_MAX_N 21
#define MAX_SUPPORTED_N 34
#define DEFAULT_K 4
#define MAX_SUPPORTED_K 10
#define KNOWN_MAX_N 21
#define DEFAULT_CHECK_N 21
#define DEFAULT_GENERAL_CHECK_N 10
#define DIRECT_CHECK_MAX_N 10
#define INDEPENDENT_CHECK_MAX_N 16
#define DEFAULT_MEMORY_MIB UINT64_C(1024)
#define MIN_MEMORY_MIB UINT64_C(16)

_Static_assert(MAX_SUPPORTED_N < 64,
               "the subset mask must fit in uint64_t");
_Static_assert(UINT32_MAX >= UINT32_C(2333606220),
               "subset ranks through C(34,17) must fit in uint32_t");

/* A179957, n=0..21, from https://oeis.org/A179957/b179957.txt. */
static const char *const known_terms[KNOWN_MAX_N + 1] = {
    "1",
    "1",
    "0",
    "0",
    "0",
    "0",
    "0",
    "0",
    "2",
    "72",
    "1496",
    "25384",
    "399848",
    "6231544",
    "99133496",
    "1634227958",
    "28120703888",
    "506599465896",
    "9562489659952",
    "189055933191880",
    "3911226769004280",
    "84567966329357056"
};

typedef struct {
    uint64_t low;
    uint64_t high;
} Count128;

typedef struct {
    size_t limit;
    size_t in_use;
    size_t peak;
} MemoryBudget;

typedef struct {
    uint64_t subsets;
    uint64_t states;
    uint64_t local_subtractions;
    size_t peak_layer_states;
    size_t rank_entries;
    size_t peak_bytes;
    double seconds;
} DpStats;

typedef enum {
    RUN_UPTO,
    RUN_TERM,
    RUN_CHECK
} RunMode;

static void die(const char *message)
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

static size_t checked_product_size(size_t left, size_t right)
{
    if (left != 0U && right > SIZE_MAX / left) {
        die("size multiplication overflow");
    }
    return left * right;
}

static size_t checked_add_size(size_t left, size_t right)
{
    if (right > SIZE_MAX - left) {
        die("size addition overflow");
    }
    return left + right;
}

static int parse_n(const char *text, const char *label)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < SEQUENCE_OFFSET || value > MAX_SUPPORTED_N) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                label, SEQUENCE_OFFSET, MAX_SUPPORTED_N, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static int parse_k(const char *text)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < 1 || value > MAX_SUPPORTED_K) {
        fprintf(stderr, "error: K must be in 1..%d: %s\n",
                MAX_SUPPORTED_K, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static uint64_t parse_memory_mib(const char *text)
{
    char *end = NULL;
    errno = 0;
    unsigned long long value = strtoull(text, &end, 10);
    uint64_t maximum = (uint64_t)(SIZE_MAX / (size_t)UINT64_C(1048576));
    if (errno != 0 || end == text || *end != '\0' ||
        value < MIN_MEMORY_MIB || value > maximum) {
        fprintf(stderr,
                "error: MEMORY_MIB must be in %" PRIu64 "..%" PRIu64
                ": %s\n",
                MIN_MEMORY_MIB, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (uint64_t)value;
}

static Count128 count_zero(void)
{
    Count128 result = {0, 0};
    return result;
}

static Count128 count_from_u64(uint64_t value)
{
    Count128 result = {value, 0};
    return result;
}

static bool count_equal(Count128 left, Count128 right)
{
    return left.low == right.low && left.high == right.high;
}

static int count_compare(Count128 left, Count128 right)
{
    if (left.high != right.high) {
        return left.high < right.high ? -1 : 1;
    }
    if (left.low != right.low) {
        return left.low < right.low ? -1 : 1;
    }
    return 0;
}

static void count_add_to(Count128 *destination, Count128 addend)
{
    uint64_t old_low = destination->low;
    uint64_t new_low = old_low + addend.low;
    uint64_t carry = new_low < old_low ? UINT64_C(1) : UINT64_C(0);
    uint64_t old_high = destination->high;
    uint64_t new_high = old_high + addend.high;
    if (new_high < old_high) {
        die("unsigned 128-bit addition overflow");
    }
    uint64_t final_high = new_high + carry;
    if (final_high < new_high) {
        die("unsigned 128-bit addition overflow");
    }
    destination->low = new_low;
    destination->high = final_high;
}

static Count128 count_subtract(Count128 minuend, Count128 subtrahend)
{
    if (count_compare(minuend, subtrahend) < 0) {
        die("internal negative DP count");
    }
    Count128 result;
    result.low = minuend.low - subtrahend.low;
    uint64_t borrow = minuend.low < subtrahend.low
                          ? UINT64_C(1) : UINT64_C(0);
    result.high = minuend.high - subtrahend.high - borrow;
    return result;
}

static uint128_t count_to_u128(Count128 value)
{
    return ((uint128_t)value.high << 64U) | (uint128_t)value.low;
}

static Count128 count_from_u128(uint128_t value)
{
    Count128 result = {(uint64_t)value, (uint64_t)(value >> 64U)};
    return result;
}

static Count128 parse_count(const char *text)
{
    uint128_t value = 0;
    for (const unsigned char *p = (const unsigned char *)text;
         *p != '\0'; ++p) {
        if (*p < (unsigned char)'0' || *p > (unsigned char)'9') {
            die("invalid built-in decimal value");
        }
        unsigned digit = (unsigned)(*p - (unsigned char)'0');
        uint128_t maximum = ~(uint128_t)0;
        if (value > (maximum - (uint128_t)digit) / (uint128_t)10U) {
            die("built-in decimal value exceeds unsigned 128-bit range");
        }
        value = value * (uint128_t)10U + (uint128_t)digit;
    }
    return count_from_u128(value);
}

static int fprint_count(FILE *stream, Count128 count)
{
    char digits[40];
    size_t length = 0;
    uint128_t value = count_to_u128(count);
    do {
        digits[length++] = (char)('0' + (unsigned)(value % 10U));
        value /= 10U;
    } while (value != 0);
    while (length != 0U) {
        if (fputc((unsigned char)digits[--length], stream) == EOF) {
            return -1;
        }
    }
    return 0;
}

static void budget_reserve(MemoryBudget *budget, size_t bytes)
{
    if (bytes > budget->limit - budget->in_use) {
        fprintf(stderr,
                "error: DP memory budget exceeded: need another %.1f MiB "
                "with %.1f/%.1f MiB already in use; rerun with a larger "
                "--memory-mib value\n",
                (double)bytes / 1048576.0,
                (double)budget->in_use / 1048576.0,
                (double)budget->limit / 1048576.0);
        exit(EXIT_FAILURE);
    }
    budget->in_use += bytes;
    if (budget->peak < budget->in_use) {
        budget->peak = budget->in_use;
    }
}

static void budget_release(MemoryBudget *budget, size_t bytes)
{
    if (bytes > budget->in_use) {
        die("internal memory-budget underflow");
    }
    budget->in_use -= bytes;
}

static void *budget_malloc_array(MemoryBudget *budget,
                                 size_t count, size_t size)
{
    size_t bytes = checked_product_size(count, size);
    budget_reserve(budget, bytes);
    void *pointer = malloc(bytes == 0U ? 1U : bytes);
    if (pointer == NULL) {
        fprintf(stderr, "error: could not allocate %.1f MiB\n",
                (double)bytes / 1048576.0);
        exit(EXIT_FAILURE);
    }
    return pointer;
}

static void *budget_calloc_array(MemoryBudget *budget,
                                 size_t count, size_t size)
{
    size_t bytes = checked_product_size(count, size);
    budget_reserve(budget, bytes);
    void *pointer = calloc(count == 0U ? 1U : count,
                           size == 0U ? 1U : size);
    if (pointer == NULL) {
        fprintf(stderr, "error: could not allocate %.1f MiB\n",
                (double)bytes / 1048576.0);
        exit(EXIT_FAILURE);
    }
    return pointer;
}

static void budget_free_array(MemoryBudget *budget, void *pointer,
                              size_t count, size_t size)
{
    size_t bytes = checked_product_size(count, size);
    free(pointer);
    budget_release(budget, bytes);
}

static void make_binomials(
    uint64_t choose[MAX_SUPPORTED_N + 1][MAX_SUPPORTED_N + 1], int n)
{
    memset(choose, 0,
           sizeof(uint64_t) * (MAX_SUPPORTED_N + 1U) *
               (MAX_SUPPORTED_N + 1U));
    choose[0][0] = UINT64_C(1);
    for (int row = 1; row <= n; ++row) {
        choose[row][0] = UINT64_C(1);
        choose[row][row] = UINT64_C(1);
        for (int column = 1; column < row; ++column) {
            uint64_t left = choose[row - 1][column - 1];
            uint64_t right = choose[row - 1][column];
            if (right > UINT64_MAX - left) {
                die("binomial coefficient overflow");
            }
            choose[row][column] = left + right;
        }
    }
}

static uint32_t *make_subset_ranks(
    int n,
    const uint64_t choose[MAX_SUPPORTED_N + 1][MAX_SUPPORTED_N + 1],
    MemoryBudget *budget, size_t *entry_count)
{
    *entry_count = (size_t)1U << (unsigned)n;
    uint32_t *rank = budget_malloc_array(
        budget, *entry_count, sizeof(*rank));
    rank[0] = UINT32_C(0);
    for (size_t index = 1; index < *entry_count; ++index) {
        uint64_t mask = (uint64_t)index;
#if defined(__GNUC__) || defined(__clang__)
        unsigned high = 63U - (unsigned)__builtin_clzll(mask);
        unsigned count = (unsigned)__builtin_popcountll(mask);
#else
        unsigned high = 0;
        unsigned count = 0;
        for (unsigned bit = 0; bit < (unsigned)n; ++bit) {
            if ((mask & (UINT64_C(1) << bit)) != 0) {
                high = bit;
                ++count;
            }
        }
#endif
        uint64_t high_bit = UINT64_C(1) << high;
        size_t remainder = (size_t)(mask ^ high_bit);
        uint64_t value = choose[high][count] + rank[remainder];
        if (value > UINT32_MAX || value >= choose[n][count]) {
            die("subset rank overflow or inconsistency");
        }
        rank[index] = (uint32_t)value;
    }
    return rank;
}

static uint64_t first_mask_with_popcount(int count)
{
    return (UINT64_C(1) << (unsigned)count) - UINT64_C(1);
}

static uint64_t next_fixed_popcount_mask(uint64_t mask)
{
    if (mask == 0) {
        die("internal zero mask in fixed-popcount iterator");
    }
    uint64_t low_bit = mask & (UINT64_C(0) - mask);
    if (low_bit == 0) {
        die("internal low-bit failure in fixed-popcount iterator");
    }
    uint64_t ripple = mask + low_bit;
    return ripple | (((ripple ^ mask) >> 2U) / low_bit);
}

static unsigned bit_index(uint64_t bit)
{
    if (bit == 0 || (bit & (bit - UINT64_C(1))) != 0) {
        die("internal non-singleton bit");
    }
#if defined(__GNUC__) || defined(__clang__)
    return (unsigned)__builtin_ctzll(bit);
#else
    unsigned index = 0;
    while ((bit >> index) != UINT64_C(1)) {
        ++index;
    }
    return index;
#endif
}

static unsigned endpoint_position(uint64_t mask, unsigned endpoint)
{
    uint64_t lower = (UINT64_C(1) << endpoint) - UINT64_C(1);
#if defined(__GNUC__) || defined(__clang__)
    return (unsigned)__builtin_popcountll(mask & lower);
#else
    uint64_t value = mask & lower;
    unsigned count = 0;
    while (value != 0) {
        value &= value - UINT64_C(1);
        ++count;
    }
    return count;
#endif
}

static size_t layer_state_count(
    int n, int cardinality,
    const uint64_t choose[MAX_SUPPORTED_N + 1][MAX_SUPPORTED_N + 1])
{
    if (cardinality < 0 || cardinality > n) {
        die("internal invalid layer cardinality");
    }
    uint64_t subsets = choose[n][cardinality];
    if (subsets > SIZE_MAX) {
        die("layer subset count exceeds size_t");
    }
    return checked_product_size((size_t)subsets,
                                (size_t)cardinality);
}

static void checked_increment_u64(uint64_t *value, const char *label)
{
    if (*value == UINT64_MAX) {
        fprintf(stderr, "error: %s counter overflow\n", label);
        exit(EXIT_FAILURE);
    }
    ++*value;
}

static Count128 compute_dp(int n, int k, uint64_t memory_mib,
                           DpStats *stats)
{
    double started = monotonic_seconds();
    memset(stats, 0, sizeof(*stats));
    if (n == 0 || n == 1) {
        stats->seconds = monotonic_seconds() - started;
        return count_from_u64(UINT64_C(1));
    }

    MemoryBudget budget;
    budget.limit = checked_product_size((size_t)memory_mib,
                                        (size_t)UINT64_C(1048576));
    budget.in_use = 0;
    budget.peak = 0;

    uint64_t choose[MAX_SUPPORTED_N + 1][MAX_SUPPORTED_N + 1];
    make_binomials(choose, n);
    size_t rank_entries = 0;
    uint32_t *rank = make_subset_ranks(
        n, choose, &budget, &rank_entries);
    stats->rank_entries = rank_entries;

    uint64_t full = (UINT64_C(1) << (unsigned)n) - UINT64_C(1);
    uint64_t bad[MAX_SUPPORTED_N] = {0};
    for (int value = 0; value < n; ++value) {
        int low = value - (k - 1);
        int high = value + (k - 1);
        if (low < 0) {
            low = 0;
        }
        if (high >= n) {
            high = n - 1;
        }
        for (int other = low; other <= high; ++other) {
            bad[value] |= UINT64_C(1) << (unsigned)other;
        }
    }

    size_t current_count = layer_state_count(n, 1, choose);
    Count128 *current = budget_calloc_array(
        &budget, current_count, sizeof(*current));
    for (int value = 0; value < n; ++value) {
        current[value] = count_from_u64(UINT64_C(1));
    }
    stats->peak_layer_states = current_count;

    for (int cardinality = 1; cardinality < n; ++cardinality) {
        size_t next_count =
            layer_state_count(n, cardinality + 1, choose);
        Count128 *next = budget_calloc_array(
            &budget, next_count, sizeof(*next));
        size_t two_layers = checked_add_size(current_count, next_count);
        if (stats->peak_layer_states < two_layers) {
            stats->peak_layer_states = two_layers;
        }

        uint64_t subset = first_mask_with_popcount(cardinality);
        uint64_t limit = UINT64_C(1) << (unsigned)n;
        while (subset < limit) {
            checked_increment_u64(&stats->subsets, "subset");
            size_t base = checked_product_size(
                (size_t)rank[(size_t)subset],
                (size_t)cardinality);
            if (base > current_count ||
                (size_t)cardinality > current_count - base) {
                die("current-layer index out of range");
            }

            Count128 total = count_zero();
            for (int position = 0; position < cardinality; ++position) {
                count_add_to(&total, current[base + (size_t)position]);
            }

            uint64_t unused = full & ~subset;
            while (unused != 0) {
                uint64_t bit = unused & (UINT64_C(0) - unused);
                unused ^= bit;
                unsigned value = bit_index(bit);
                Count128 count = total;

                uint64_t forbidden_previous = bad[value] & subset;
                while (forbidden_previous != 0) {
                    uint64_t previous_bit =
                        forbidden_previous &
                        (UINT64_C(0) - forbidden_previous);
                    forbidden_previous ^= previous_bit;
                    unsigned previous = bit_index(previous_bit);
                    unsigned position =
                        endpoint_position(subset, previous);
                    if (position >= (unsigned)cardinality) {
                        die("previous-endpoint position out of range");
                    }
                    count = count_subtract(
                        count, current[base + (size_t)position]);
                    checked_increment_u64(&stats->local_subtractions,
                                          "local subtraction");
                }

                uint64_t new_subset = subset | bit;
                unsigned new_position =
                    endpoint_position(new_subset, value);
                size_t next_base = checked_product_size(
                    (size_t)rank[(size_t)new_subset],
                    (size_t)(cardinality + 1));
                size_t next_index = checked_add_size(
                    next_base, (size_t)new_position);
                if (new_position >= (unsigned)(cardinality + 1) ||
                    next_index >= next_count) {
                    die("next-layer index out of range");
                }
                next[next_index] = count;
                checked_increment_u64(&stats->states, "state");
            }

            uint64_t following = next_fixed_popcount_mask(subset);
            if (following <= subset || following >= limit) {
                break;
            }
            subset = following;
        }

        budget_free_array(&budget, current,
                          current_count, sizeof(*current));
        current = next;
        current_count = next_count;
    }

    if (current_count != (size_t)n) {
        die("unexpected final-layer size");
    }
    Count128 result = count_zero();
    for (size_t endpoint = 0; endpoint < current_count; ++endpoint) {
        count_add_to(&result, current[endpoint]);
    }

    budget_free_array(&budget, current,
                      current_count, sizeof(*current));
    budget_free_array(&budget, rank,
                      rank_entries, sizeof(*rank));
    if (budget.in_use != 0U) {
        die("internal DP memory leak");
    }
    stats->peak_bytes = budget.peak;
    stats->seconds = monotonic_seconds() - started;
    return result;
}

static bool permutation_is_valid(const int *permutation, int n, int k)
{
    for (int i = 1; i < n; ++i) {
        int difference = permutation[i] - permutation[i - 1];
        if (difference > -k && difference < k) {
            return false;
        }
    }
    return true;
}

static bool next_permutation(int *permutation, int n)
{
    int i = n - 2;
    while (i >= 0 && permutation[i] >= permutation[i + 1]) {
        --i;
    }
    if (i < 0) {
        return false;
    }
    int j = n - 1;
    while (permutation[j] <= permutation[i]) {
        --j;
    }
    int temporary = permutation[i];
    permutation[i] = permutation[j];
    permutation[j] = temporary;
    for (int left = i + 1, right = n - 1; left < right;
         ++left, --right) {
        temporary = permutation[left];
        permutation[left] = permutation[right];
        permutation[right] = temporary;
    }
    return true;
}

static Count128 direct_count(int n, int k)
{
    if (n == 0) {
        return count_from_u64(UINT64_C(1));
    }
    int permutation[DIRECT_CHECK_MAX_N];
    for (int i = 0; i < n; ++i) {
        permutation[i] = i;
    }
    Count128 result = count_zero();
    do {
        if (permutation_is_valid(permutation, n, k)) {
            count_add_to(&result, count_from_u64(UINT64_C(1)));
        }
    } while (next_permutation(permutation, n));
    return result;
}

/* Independent ordinary forward DP: explicitly enumerate every allowed next. */
static uint64_t independent_forward_dp(int n, int k)
{
    if (n == 0) {
        return UINT64_C(1);
    }
    size_t mask_count = (size_t)1U << (unsigned)n;
    size_t state_count = checked_product_size(mask_count, (size_t)n);
    uint64_t *dp = calloc(state_count, sizeof(*dp));
    if (dp == NULL) {
        die("out of memory in independent forward DP");
    }
    uint64_t full = (UINT64_C(1) << (unsigned)n) - UINT64_C(1);
    uint64_t allowed[INDEPENDENT_CHECK_MAX_N] = {0};
    for (int value = 0; value < n; ++value) {
        for (int other = 0; other < n; ++other) {
            int difference = value - other;
            if (difference <= -k || difference >= k) {
                allowed[value] |= UINT64_C(1) << (unsigned)other;
            }
        }
        size_t singleton =
            ((size_t)1U << (unsigned)value) * (size_t)n +
            (size_t)value;
        dp[singleton] = UINT64_C(1);
    }

    for (uint64_t mask = UINT64_C(1); mask <= full; ++mask) {
        size_t base = checked_product_size((size_t)mask, (size_t)n);
        uint64_t endpoint_bits = mask;
        while (endpoint_bits != 0) {
            uint64_t endpoint_bit =
                endpoint_bits & (UINT64_C(0) - endpoint_bits);
            endpoint_bits ^= endpoint_bit;
            unsigned endpoint = bit_index(endpoint_bit);
            uint64_t count = dp[base + endpoint];
            if (count == 0) {
                continue;
            }
            uint64_t candidates = allowed[endpoint] & ~mask & full;
            while (candidates != 0) {
                uint64_t bit = candidates & (UINT64_C(0) - candidates);
                candidates ^= bit;
                unsigned next = bit_index(bit);
                size_t target = checked_product_size(
                                    (size_t)(mask | bit), (size_t)n) +
                                (size_t)next;
                if (dp[target] > UINT64_MAX - count) {
                    free(dp);
                    die("uint64 overflow in independent forward DP");
                }
                dp[target] += count;
            }
        }
    }

    uint64_t result = UINT64_C(0);
    size_t final_base = checked_product_size((size_t)full, (size_t)n);
    for (int endpoint = 0; endpoint < n; ++endpoint) {
        if (result > UINT64_MAX - dp[final_base + (size_t)endpoint]) {
            free(dp);
            die("result overflow in independent forward DP");
        }
        result += dp[final_base + (size_t)endpoint];
    }
    free(dp);
    return result;
}

static void verify_known(Count128 value, int n, int k)
{
    if (k == 4 && n <= KNOWN_MAX_N) {
        Count128 expected = parse_count(known_terms[n]);
        if (!count_equal(value, expected)) {
            fprintf(stderr, "error: A179957 mismatch at n=%d: got ", n);
            (void)fprint_count(stderr, value);
            fprintf(stderr, ", expected %s\n", known_terms[n]);
            exit(EXIT_FAILURE);
        }
    }
    if (n > 1 && (value.low & UINT64_C(1)) != 0) {
        fprintf(stderr, "error: reversal parity check failed at n=%d\n", n);
        exit(EXIT_FAILURE);
    }
}

static Count128 compute_value(int n, int k, uint64_t memory_mib,
                              bool verbose)
{
    DpStats stats;
    Count128 value = compute_dp(n, k, memory_mib, &stats);
    verify_known(value, n, k);
    if (verbose) {
        fprintf(stderr,
                "179957_01: k=%d, n=%d, layered total-minus-local DP, "
                "subsets=%" PRIu64 ", states=%" PRIu64
                ", local subtractions=%" PRIu64
                ", peak two-layer states=%zu, rank entries=%zu, "
                "peak memory=%.1f MiB, %.3f s\n",
                k, n, stats.subsets, stats.states,
                stats.local_subtractions, stats.peak_layer_states,
                stats.rank_entries,
                (double)stats.peak_bytes / 1048576.0,
                stats.seconds);
    }
    return value;
}

static int check_implementation(int maximum_n, int k,
                                uint64_t memory_mib)
{
    if (maximum_n > KNOWN_MAX_N) {
        fprintf(stderr, "error: CHECK_N must be in 0..%d: %d\n",
                KNOWN_MAX_N, maximum_n);
        return EXIT_FAILURE;
    }
    for (int n = 0; n <= maximum_n; ++n) {
        Count128 value = compute_value(n, k, memory_mib, false);
        if (n <= DIRECT_CHECK_MAX_N) {
            Count128 direct = direct_count(n, k);
            if (!count_equal(value, direct)) {
                fprintf(stderr,
                        "error: optimized/direct mismatch at n=%d: ", n);
                (void)fprint_count(stderr, value);
                fputs(" versus ", stderr);
                (void)fprint_count(stderr, direct);
                fputc('\n', stderr);
                return EXIT_FAILURE;
            }
        }
        if (n <= INDEPENDENT_CHECK_MAX_N) {
            uint64_t independent = independent_forward_dp(n, k);
            if (value.high != 0 || value.low != independent) {
                fprintf(stderr,
                        "error: optimized/forward-DP mismatch at n=%d: ",
                        n);
                (void)fprint_count(stderr, value);
                fprintf(stderr, " versus %" PRIu64 "\n", independent);
                return EXIT_FAILURE;
            }
        }
    }
    if (k == 4) {
        printf("ok: k=4, A179957 b-file verified through n=%d; ",
               maximum_n);
    } else {
        printf("ok: k=%d; ", k);
    }
    printf("definition-level full permutation scan through n=%d; "
           "independent forward subset DP through n=%d; reversal parity "
           "checked for n>1\n",
           maximum_n < DIRECT_CHECK_MAX_N
               ? maximum_n : DIRECT_CHECK_MAX_N,
           maximum_n < INDEPENDENT_CHECK_MAX_N
               ? maximum_n : INDEPENDENT_CHECK_MAX_N);
    return EXIT_SUCCESS;
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
                              sizeof(executable) - 1U);
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
        memcpy(executable, argv0, length + 1U);
    }
    const char *base = realpath(executable, resolved);
    if (base == NULL) {
        base = executable;
    }
    const char *slash = strrchr(base, '/');
    size_t directory_length =
        slash == NULL ? 1U : (size_t)(slash - base);
    if (slash != NULL && directory_length == 0U) {
        directory_length = 1U;
    }
    size_t filename_length = strlen(filename);
    if (directory_length > SIZE_MAX - filename_length - 2U) {
        die("output path length overflow");
    }
    char *path = malloc(directory_length + filename_length + 2U);
    if (path == NULL) {
        die("out of memory while making output path");
    }
    if (slash == NULL) {
        path[0] = '.';
    } else if (slash == base) {
        path[0] = '/';
    } else {
        memcpy(path, base, directory_length);
    }
    path[directory_length] = '/';
    memcpy(path + directory_length + 1U, filename,
           filename_length + 1U);
    return path;
}

static void produce_file(const char *argv0, int maximum_n, int k,
                         uint64_t memory_mib)
{
    char final_name[64];
    char part_name[64];
    int final_length = snprintf(final_name, sizeof(final_name),
                                "b179957_01_k%d.txt", k);
    int part_length = snprintf(part_name, sizeof(part_name),
                               "b179957_01_k%d_part.txt", k);
    if (final_length < 0 || (size_t)final_length >= sizeof(final_name) ||
        part_length < 0 || (size_t)part_length >= sizeof(part_name)) {
        die("k-specific output filename is too long");
    }
    char *final_path =
        path_beside_executable(argv0, final_name);
    char *part_path =
        path_beside_executable(argv0, part_name);
    FILE *stream = fopen(part_path, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part_path, strerror(errno));
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }
    for (int n = SEQUENCE_OFFSET; n <= maximum_n; ++n) {
        Count128 value = compute_value(n, k, memory_mib, true);
        if (fprintf(stream, "%d ", n) < 0 ||
            fprint_count(stream, value) != 0 ||
            fputc('\n', stream) == EOF || fflush(stream) != 0) {
            (void)fclose(stream);
            free(final_path);
            free(part_path);
            die("could not write the threshold-k output file");
        }
    }
    if (fclose(stream) != 0) {
        free(final_path);
        free(part_path);
        die("could not close the threshold-k output file");
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }
    printf("wrote %s (n=%d..%d)\n",
           final_path, SEQUENCE_OFFSET, maximum_n);
    free(final_path);
    free(part_path);
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K] [--memory-mib M]\n"
            "       %s --upto MAX_N [--k K] [--memory-mib M]\n"
            "       %s --term N [--k K] [--memory-mib M]\n"
            "       %s --check [MAX_N] [--k K] [--memory-mib M]\n"
            "\n"
            "N and MAX_N may be %d..%d; 128-bit counts are proved safe "
            "throughout.\n"
            "K may be 1..%d and defaults to %d.\n"
            "The default is --upto %d with a %" PRIu64
            " MiB rank-and-DP budget.\n"
            "A range run writes b179957_01_kK.txt beside the executable.\n",
            program, program, program, program,
            SEQUENCE_OFFSET, MAX_SUPPORTED_N,
            MAX_SUPPORTED_K, DEFAULT_K, DEFAULT_MAX_N,
            DEFAULT_MEMORY_MIB);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 ||
         strcmp(argv[1], "-h") == 0)) {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    RunMode mode = RUN_UPTO;
    int n = -1;
    bool have_n = false;
    bool have_mode = false;
    int k = DEFAULT_K;
    bool have_k = false;
    uint64_t memory_mib = DEFAULT_MEMORY_MIB;
    bool have_memory = false;

    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--k") == 0) {
            if (have_k || argument + 1 >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            k = parse_k(argv[++argument]);
            have_k = true;
        } else if (strcmp(text, "--memory-mib") == 0) {
            if (have_memory || argument + 1 >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            memory_mib = parse_memory_mib(argv[++argument]);
            have_memory = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = strcmp(text, "--term") == 0 ? RUN_TERM : RUN_UPTO;
            have_mode = true;
            n = parse_n(argv[++argument],
                        mode == RUN_TERM ? "N" : "MAX_N");
            have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode || have_n) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = RUN_CHECK;
            have_mode = true;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                n = parse_n(argv[++argument], "CHECK_N");
                have_n = true;
            }
        } else if (text[0] == '-' || have_n) {
            usage(argv[0]);
            return EXIT_FAILURE;
        } else {
            n = parse_n(text,
                        mode == RUN_CHECK ? "CHECK_N" : "MAX_N");
            have_n = true;
        }
    }

    if (!have_n) {
        n = mode == RUN_CHECK
                ? (k == 4 ? DEFAULT_CHECK_N : DEFAULT_GENERAL_CHECK_N)
                : DEFAULT_MAX_N;
    }
    if (mode == RUN_CHECK) {
        return check_implementation(n, k, memory_mib);
    }
    if (mode == RUN_TERM) {
        Count128 value = compute_value(n, k, memory_mib, true);
        printf("%d ", n);
        if (fprint_count(stdout, value) != 0 || fputc('\n', stdout) == EOF) {
            die("could not write result");
        }
        return EXIT_SUCCESS;
    }
    produce_file(argv[0], n, k, memory_mib);
    return EXIT_SUCCESS;
}
