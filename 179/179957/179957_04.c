/*
 * A179957 (k=4) and its general threshold-k version.
 * Inclusion-exclusion plus a linear-forest frontier DP.
 *
 * Count permutations p of {1,...,n} satisfying
 *
 *                 |p(i)-p(i-1)| >= k,  i>1.
 *
 * Let H_{n,k} have vertex set {1,...,n}, with {u,v} an edge exactly when
 * 1 <= |u-v| < k.  These are the forbidden adjacencies.  A selected edge
 * set F can occur in a linear permutation only when F is a linear forest.
 * If F has j edges and c nontrivial path components, contracting those
 * paths shows that its inclusion-exclusion intersection has
 *
 *                         2^c (n-j)!
 *
 * permutations.  Hence
 *
 *   Q_{n,k}(x) = sum_{F subset H_{n,k}, F linear forest}
 *                    2^c(F) x^|F|,
 *
 *   a_k(n) = sum_j (-1)^j [x^j] Q_{n,k}(x) (n-j)!.
 *
 * H_{n,k} is the (k-1)-st power of a path.  Introduce its vertices in
 * increasing order.  A new vertex is adjacent only to the previous k-1,
 * so the frontier width is k-1.  A state records the frontier degrees and
 * connected-component partition.  A transition selects zero, one, or two
 * new edges, rejects degree 3 and cycles, and forgets the oldest vertex.
 * When a path component disappears from the frontier, its orientation
 * factor 2 is applied exactly once.  Components still open at the end are
 * finalized in the same way.
 *
 * For every fixed k, this uses S(k) frontier states and computes all terms
 * through N in O(S(k)*k^2*N^2) exact-arithmetic operations and
 * O(S(k)*N) GMP coefficients.  S(k) grows quickly: this is intended for
 * small fixed k; the supported and intended input range is 1<=k<=10.
 * The elementary boundary cases a_k(0)=a_k(1)=1, a_k(n)=0 for
 * 2<=n<2k, and a_k(2k)=2 are returned without constructing frontier
 * states.  (For n<2k the allowed-adjacency graph has an isolated vertex;
 * for n=2k its chain graph has one Hamilton path up to reversal.)
 * Structural guards stop before 100000 simultaneous states or 2000000 GMP
 * frontier coefficient objects per layer; hitting a guard reports an error,
 * never a partial value as a completed answer.
 *
 * All coefficients, factorials, and answers use GMP.  State hashes are
 * collision-resolved by full 128-bit-key equality.  Allocation products,
 * polynomial degrees, transition weights, counters, state canonical form,
 * maximum degree, cycles, answer sign, and reversal parity are checked.
 * A range run writes b179957_04_kK_part.txt term by term and atomically
 * renames it to b179957_04_kK.txt only after success.
 *
 * --check independently compares with:
 *   - OEIS A179957 through n=21 when k=4;
 *   - a definition-level scan of all permutations through n=10;
 *   - direct enumeration of every F subset H_{n,k}, while H has <=22
 *     edges (through n=9 whenever that edge limit permits).
 *
 * References:
 *   https://oeis.org/A179957
 *   Ira M. Gessel, Generalized Rook Polynomials and Orthogonal Polynomials,
 *   section 6:
 *   https://people.brandeis.edu/~gessel/homepage/papers/rookp.pdf
 *
 * Build (Homebrew GMP on macOS):
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     179957_04.c -lgmp -o 179957_04
 *
 * Usage:
 *
 *   ./179957_04 --k 4
 *   ./179957_04 --k 3 --upto 100
 *   ./179957_04 --k 5 --term 100
 *   ./179957_04 --k 4 --check
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* GMP needs FILE to be declared before gmp.h for gmp_fprintf. */
#include <gmp.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#define SEQUENCE_OFFSET 0
#define DEFAULT_MAX_N 21
#define MAX_SUPPORTED_N 10000
#define DEFAULT_K 4
#define MAX_SUPPORTED_K 10
#define MAX_FRONTIER_WIDTH (MAX_SUPPORTED_K - 1)
#define MAX_BAG_SIZE MAX_SUPPORTED_K
#define MAX_RAW_TRANSITIONS \
    (1 + MAX_FRONTIER_WIDTH + \
     MAX_FRONTIER_WIDTH * (MAX_FRONTIER_WIDTH - 1) / 2)
#define MAX_ACTIVE_STATES 100000U
#define MAX_FRONTIER_COEFFICIENT_SLOTS 2000000U

#define KNOWN_MAX_N 21
#define DEFAULT_CHECK_N 21
#define DEFAULT_GENERAL_CHECK_N 10
#define DIRECT_CHECK_MAX_N 10
#define FOREST_CHECK_MAX_N 9
#define FOREST_CHECK_MAX_EDGES 22

static const char *const known_terms[KNOWN_MAX_N + 1] = {
    "1", "1", "0", "0", "0", "0", "0", "0", "2", "72",
    "1496", "25384", "399848", "6231544", "99133496",
    "1634227958", "28120703888", "506599465896",
    "9562489659952", "189055933191880", "3911226769004280",
    "84567966329357056"
};

typedef enum {
    RUN_UPTO,
    RUN_TERM,
    RUN_CHECK
} RunMode;

/* One byte per slot: low 2 bits degree, next 4 bits component label. */
typedef struct {
    uint64_t low;
    uint64_t high;
} StateKey;

typedef struct {
    StateKey key;
    uint8_t edges;
    unsigned long weight;
} Transition;

typedef struct {
    StateKey key;
    mpz_t *coefficient;
} PolynomialState;

typedef struct {
    PolynomialState *item;
    size_t count;
    size_t capacity;
    size_t *bucket;             /* zero means empty; otherwise index+1 */
    size_t bucket_count;        /* always a power of two */
    size_t coefficient_count;
} PolynomialMap;

typedef struct {
    size_t peak_active_states;
    size_t peak_frontier_coefficient_slots;
    uint64_t state_transition_applications;
    uint64_t polynomial_addmuls;
    double seconds;
} DpStats;

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

static void increment_u64(uint64_t *value, const char *what)
{
    if (*value == UINT64_MAX) {
        fprintf(stderr, "error: %s counter overflow\n", what);
        exit(EXIT_FAILURE);
    }
    ++*value;
}

static void *xcalloc(size_t count, size_t size)
{
    size_t bytes = checked_product_size(count, size);
    void *pointer = calloc(bytes == 0U ? 1U : count,
                           bytes == 0U ? 1U : size);
    if (pointer == NULL) {
        die("out of memory");
    }
    return pointer;
}

static void *xreallocarray(void *pointer, size_t count, size_t size)
{
    size_t bytes = checked_product_size(count, size);
    void *result = realloc(pointer, bytes == 0U ? 1U : bytes);
    if (result == NULL) {
        die("out of memory");
    }
    return result;
}

static int parse_bounded_integer(const char *text, const char *label,
                                 int minimum, int maximum)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < minimum || value > maximum) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                label, minimum, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static int dsu_find(int parent[MAX_BAG_SIZE], int vertex)
{
    int root = vertex;
    while (parent[root] != root) {
        root = parent[root];
    }
    while (parent[vertex] != vertex) {
        int next = parent[vertex];
        parent[vertex] = root;
        vertex = next;
    }
    return root;
}

static void dsu_union(int parent[MAX_BAG_SIZE], int left, int right)
{
    int left_root = dsu_find(parent, left);
    int right_root = dsu_find(parent, right);
    if (left_root != right_root) {
        parent[right_root] = left_root;
    }
}

static bool state_key_equal(StateKey left, StateKey right)
{
    return left.low == right.low && left.high == right.high;
}

static uint8_t state_key_byte(StateKey key, int slot)
{
    if (slot < 0 || slot >= MAX_FRONTIER_WIDTH) {
        die("frontier slot is out of range");
    }
    if (slot < 8) {
        return (uint8_t)(key.low >> (8U * (unsigned)slot));
    }
    return (uint8_t)(key.high >> (8U * (unsigned)(slot - 8)));
}

static void state_key_set_byte(StateKey *key, int slot, uint8_t byte)
{
    if (slot < 0 || slot >= MAX_FRONTIER_WIDTH) {
        die("frontier slot is out of range while encoding");
    }
    if (slot < 8) {
        key->low |= (uint64_t)byte << (8U * (unsigned)slot);
    } else {
        key->high |= (uint64_t)byte <<
                     (8U * (unsigned)(slot - 8));
    }
}

static StateKey encode_state(int size,
                             const uint8_t degree[MAX_BAG_SIZE],
                             int parent[MAX_BAG_SIZE], int first)
{
    if (size < 0 || size > MAX_FRONTIER_WIDTH || first < 0 ||
        first + size > MAX_BAG_SIZE) {
        die("invalid frontier size while encoding");
    }
    int root_label[MAX_BAG_SIZE];
    for (int i = 0; i < MAX_BAG_SIZE; ++i) {
        root_label[i] = -1;
    }
    int next_label = 0;
    StateKey key = {UINT64_C(0), UINT64_C(0)};
    for (int slot = 0; slot < size; ++slot) {
        int bag_vertex = first + slot;
        if (degree[bag_vertex] > 2U) {
            die("internal degree exceeds 2 while encoding");
        }
        int root = dsu_find(parent, bag_vertex);
        if (root_label[root] < 0) {
            root_label[root] = next_label++;
        }
        if (root_label[root] >= MAX_FRONTIER_WIDTH) {
            die("component label exceeds encoded range");
        }
        uint8_t byte = (uint8_t)(degree[bag_vertex] |
                                 ((unsigned)root_label[root] << 2U));
        state_key_set_byte(&key, slot, byte);
    }
    return key;
}

static bool decode_state(StateKey key, int size,
                         uint8_t degree[MAX_BAG_SIZE],
                         uint8_t component[MAX_BAG_SIZE])
{
    if (size < 0 || size > MAX_FRONTIER_WIDTH) {
        return false;
    }
    int largest_label = -1;
    for (int slot = 0; slot < size; ++slot) {
        uint8_t byte = state_key_byte(key, slot);
        degree[slot] = byte & 3U;
        component[slot] = (byte >> 2U) & 15U;
        if ((byte & UINT8_C(192)) != 0U || degree[slot] > 2U ||
            component[slot] > (uint8_t)(largest_label + 1)) {
            return false;
        }
        if ((int)component[slot] == largest_label + 1) {
            ++largest_label;
        }
    }
    for (int slot = size; slot < MAX_FRONTIER_WIDTH; ++slot) {
        if (state_key_byte(key, slot) != 0U) {
            return false;
        }
    }
    for (int slot = size; slot < MAX_BAG_SIZE; ++slot) {
        degree[slot] = 0U;
        component[slot] = 0U;
    }
    for (int left = 0; left < size; ++left) {
        if (degree[left] != 0U) {
            continue;
        }
        for (int right = 0; right < size; ++right) {
            if (left != right && component[left] == component[right]) {
                return false;
            }
        }
    }
    return true;
}

/* Add a new vertex and, after saturation, forget the oldest frontier one. */
static bool finish_raw_transition(StateKey source_key, int frontier_size,
                                  int frontier_width, uint16_t edge_mask,
                                  Transition *transition)
{
    uint8_t degree[MAX_BAG_SIZE];
    uint8_t component[MAX_BAG_SIZE];
    if (!decode_state(source_key, frontier_size, degree, component)) {
        die("invalid source frontier state");
    }
    unsigned selected = 0U;
    for (int old = 0; old < frontier_size; ++old) {
        selected += (edge_mask >> old) & 1U;
    }
    if (selected > 2U) {
        return false;
    }
    int parent[MAX_BAG_SIZE];
    for (int vertex = 0; vertex < MAX_BAG_SIZE; ++vertex) {
        parent[vertex] = vertex;
    }
    for (int left = 0; left < frontier_size; ++left) {
        for (int right = left + 1; right < frontier_size; ++right) {
            if (component[left] == component[right]) {
                dsu_union(parent, left, right);
            }
        }
    }
    int newest = frontier_size;
    degree[newest] = (uint8_t)selected;
    for (int old = 0; old < frontier_size; ++old) {
        if (((edge_mask >> old) & 1U) == 0U) {
            continue;
        }
        if (degree[old] >= 2U) {
            return false;
        }
        ++degree[old];
        if (dsu_find(parent, old) == dsu_find(parent, newest)) {
            return false;
        }
        dsu_union(parent, old, newest);
    }
    int bag_size = frontier_size + 1;
    int kept_first = bag_size > frontier_width ? 1 : 0;
    int kept_size = bag_size - kept_first;
    unsigned long weight = 1UL;
    if (kept_first != 0) {
        int forgotten_root = dsu_find(parent, 0);
        bool remains_active = false;
        for (int vertex = 1; vertex < bag_size; ++vertex) {
            if (dsu_find(parent, vertex) == forgotten_root) {
                remains_active = true;
                break;
            }
        }
        if (!remains_active && degree[0] != 0U) {
            weight = 2UL;
        }
    }
    transition->key = encode_state(kept_size, degree, parent, kept_first);
    transition->edges = (uint8_t)selected;
    transition->weight = weight;
    return true;
}

static void append_transition(Transition transitions[MAX_RAW_TRANSITIONS],
                              size_t *count, const Transition *candidate)
{
    for (size_t index = 0U; index < *count; ++index) {
        if (transitions[index].edges == candidate->edges &&
            state_key_equal(transitions[index].key, candidate->key)) {
            if (candidate->weight >
                ULONG_MAX - transitions[index].weight) {
                die("transition weight overflow");
            }
            transitions[index].weight += candidate->weight;
            return;
        }
    }
    if (*count >= MAX_RAW_TRANSITIONS) {
        die("too many transitions from one frontier state");
    }
    transitions[(*count)++] = *candidate;
}

static size_t build_transitions(StateKey source_key, int frontier_size,
                                int frontier_width,
                                Transition transitions[MAX_RAW_TRANSITIONS])
{
    size_t count = 0U;
    Transition candidate;
    if (!finish_raw_transition(source_key, frontier_size, frontier_width,
                               UINT16_C(0), &candidate)) {
        die("zero-edge transition was rejected");
    }
    append_transition(transitions, &count, &candidate);
    for (int first = 0; first < frontier_size; ++first) {
        uint16_t one = (uint16_t)(UINT16_C(1) << first);
        if (finish_raw_transition(source_key, frontier_size,
                                  frontier_width, one, &candidate)) {
            append_transition(transitions, &count, &candidate);
        }
        for (int second = first + 1; second < frontier_size; ++second) {
            uint16_t two = (uint16_t)(one |
                              (uint16_t)(UINT16_C(1) << second));
            if (finish_raw_transition(source_key, frontier_size,
                                      frontier_width, two, &candidate)) {
                append_transition(transitions, &count, &candidate);
            }
        }
    }
    return count;
}

static uint64_t mix_u64(uint64_t value)
{
    value ^= value >> 30U;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27U;
    value *= UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31U);
}

static size_t state_hash(StateKey key)
{
    uint64_t rotated = (key.high << 23U) | (key.high >> 41U);
    return (size_t)mix_u64(key.low ^ rotated);
}

static mpz_t *mpz_array_create(size_t count)
{
    mpz_t *array = xcalloc(count, sizeof(*array));
    for (size_t index = 0U; index < count; ++index) {
        mpz_init(array[index]);
    }
    return array;
}

static void mpz_array_destroy(mpz_t *array, size_t count)
{
    if (array == NULL) {
        return;
    }
    for (size_t index = 0U; index < count; ++index) {
        mpz_clear(array[index]);
    }
    free(array);
}

static void polynomial_map_init(PolynomialMap *map,
                                size_t coefficient_count)
{
    map->item = NULL;
    map->count = 0U;
    map->capacity = 0U;
    map->bucket = NULL;
    map->bucket_count = 0U;
    map->coefficient_count = coefficient_count;
}

static void polynomial_map_destroy(PolynomialMap *map)
{
    for (size_t index = 0U; index < map->count; ++index) {
        mpz_array_destroy(map->item[index].coefficient,
                          map->coefficient_count);
    }
    free(map->item);
    free(map->bucket);
    polynomial_map_init(map, map->coefficient_count);
}

static void polynomial_map_rehash(PolynomialMap *map,
                                  size_t new_bucket_count)
{
    if (new_bucket_count < 16U ||
        (new_bucket_count & (new_bucket_count - 1U)) != 0U) {
        die("invalid frontier hash-table size");
    }
    size_t *new_bucket = xcalloc(new_bucket_count, sizeof(*new_bucket));
    for (size_t index = 0U; index < map->count; ++index) {
        size_t slot = state_hash(map->item[index].key) &
                      (new_bucket_count - 1U);
        while (new_bucket[slot] != 0U) {
            slot = (slot + 1U) & (new_bucket_count - 1U);
        }
        new_bucket[slot] = index + 1U;
    }
    free(map->bucket);
    map->bucket = new_bucket;
    map->bucket_count = new_bucket_count;
}

static PolynomialState *polynomial_map_get(PolynomialMap *map,
                                            StateKey key)
{
    if (map->bucket_count == 0U) {
        polynomial_map_rehash(map, 16U);
    }
    size_t slot = state_hash(key) & (map->bucket_count - 1U);
    while (map->bucket[slot] != 0U) {
        size_t index = map->bucket[slot] - 1U;
        if (index >= map->count) {
            die("corrupt frontier hash-table index");
        }
        if (state_key_equal(map->item[index].key, key)) {
            return &map->item[index];
        }
        slot = (slot + 1U) & (map->bucket_count - 1U);
    }

    if (map->count >= MAX_ACTIVE_STATES) {
        die("frontier state limit reached; use a smaller k or n");
    }
    size_t new_count = checked_add_size(map->count, 1U);
    size_t coefficient_slots =
        checked_product_size(new_count, map->coefficient_count);
    if (coefficient_slots > MAX_FRONTIER_COEFFICIENT_SLOTS) {
        die("frontier coefficient-object limit reached; use a smaller k or n");
    }
    if (new_count * 10U >= map->bucket_count * 7U) {
        if (map->bucket_count > SIZE_MAX / 2U) {
            die("frontier hash-table size overflow");
        }
        polynomial_map_rehash(map, map->bucket_count * 2U);
        slot = state_hash(key) & (map->bucket_count - 1U);
        while (map->bucket[slot] != 0U) {
            slot = (slot + 1U) & (map->bucket_count - 1U);
        }
    }
    if (map->count == map->capacity) {
        size_t new_capacity = map->capacity == 0U ? 16U
                                                  : map->capacity * 2U;
        if (new_capacity < map->capacity ||
            new_capacity > MAX_ACTIVE_STATES) {
            new_capacity = MAX_ACTIVE_STATES;
        }
        if (new_capacity <= map->capacity) {
            die("frontier-state array capacity overflow");
        }
        map->item = xreallocarray(map->item, new_capacity,
                                  sizeof(*map->item));
        map->capacity = new_capacity;
    }
    size_t index = map->count++;
    map->item[index].key = key;
    map->item[index].coefficient =
        mpz_array_create(map->coefficient_count);
    map->bucket[slot] = index + 1U;
    return &map->item[index];
}

static unsigned long final_orientation_weight(StateKey key,
                                               int frontier_size)
{
    uint8_t degree[MAX_BAG_SIZE];
    uint8_t component[MAX_BAG_SIZE];
    if (!decode_state(key, frontier_size, degree, component)) {
        die("invalid frontier state during finalization");
    }
    bool counted[MAX_FRONTIER_WIDTH] = {false};
    unsigned nontrivial_components = 0U;
    for (int slot = 0; slot < frontier_size; ++slot) {
        if (degree[slot] != 0U && !counted[component[slot]]) {
            counted[component[slot]] = true;
            ++nontrivial_components;
        }
    }
    if (nontrivial_components >= sizeof(unsigned long) * CHAR_BIT) {
        die("orientation factor exceeds unsigned long");
    }
    return 1UL << nontrivial_components;
}

static void update_peaks(DpStats *stats, size_t current_states,
                         size_t next_states, size_t coefficient_count)
{
    if (current_states > stats->peak_active_states) {
        stats->peak_active_states = current_states;
    }
    if (next_states > stats->peak_active_states) {
        stats->peak_active_states = next_states;
    }
    size_t states = checked_add_size(current_states, next_states);
    size_t slots = checked_product_size(states, coefficient_count);
    if (slots > stats->peak_frontier_coefficient_slots) {
        stats->peak_frontier_coefficient_slots = slots;
    }
}

static void write_complete_term(FILE *stream, int n, const mpz_t value)
{
    if (stream != NULL &&
        (gmp_fprintf(stream, "%d %Zd\n", n, value) < 0 ||
         fflush(stream) != 0)) {
        die("could not write the partial threshold-k output file");
    }
}

static mpz_t *compute_sequence(int maximum_n, int k, FILE *stream,
                               DpStats *stats)
{
    memset(stats, 0, sizeof(*stats));
    double start = monotonic_seconds();
    size_t coefficient_count = (size_t)maximum_n + 1U;
    mpz_t *answer = mpz_array_create(coefficient_count);

    if (maximum_n <= 2 * k) {
        for (int n = 0; n <= maximum_n; ++n) {
            unsigned long value = n <= 1 ? 1UL
                                  : n < 2 * k ? 0UL : 2UL;
            mpz_set_ui(answer[n], value);
            write_complete_term(stream, n, answer[n]);
        }
        stats->seconds = monotonic_seconds() - start;
        return answer;
    }

    int frontier_width = k - 1;
    mpz_t *factorial = mpz_array_create(coefficient_count);
    mpz_t *q = mpz_array_create(coefficient_count);
    mpz_set_ui(factorial[0], 1UL);
    for (int n = 1; n <= maximum_n; ++n) {
        mpz_mul_ui(factorial[n], factorial[n - 1], (unsigned long)n);
    }

    PolynomialMap maps[2];
    polynomial_map_init(&maps[0], coefficient_count);
    polynomial_map_init(&maps[1], coefficient_count);
    StateKey empty_key = {UINT64_C(0), UINT64_C(0)};
    PolynomialState *empty = polynomial_map_get(&maps[0], empty_key);
    mpz_set_ui(empty->coefficient[0], 1UL);
    int current_index = 0;

    for (int n = 0; n <= maximum_n; ++n) {
        PolynomialMap *current = &maps[current_index];
        int frontier_size = n < frontier_width ? n : frontier_width;
        int maximum_edges = n == 0 ? 0 : n - 1;
        for (int edges = 0; edges <= maximum_edges; ++edges) {
            mpz_set_ui(q[edges], 0UL);
        }
        for (size_t state_index = 0U;
             state_index < current->count; ++state_index) {
            const PolynomialState *state = &current->item[state_index];
            unsigned long orientation =
                final_orientation_weight(state->key, frontier_size);
            for (int edges = 0; edges <= maximum_edges; ++edges) {
                if (mpz_sgn(state->coefficient[edges]) != 0) {
                    mpz_addmul_ui(q[edges], state->coefficient[edges],
                                  orientation);
                    increment_u64(&stats->polynomial_addmuls,
                                  "polynomial operation");
                }
            }
        }
        mpz_set_ui(answer[n], 0UL);
        for (int edges = 0; edges <= maximum_edges; ++edges) {
            if ((edges & 1) == 0) {
                mpz_addmul(answer[n], q[edges], factorial[n - edges]);
            } else {
                mpz_submul(answer[n], q[edges], factorial[n - edges]);
            }
        }
        if (mpz_sgn(answer[n]) < 0) {
            die("inclusion-exclusion produced a negative answer");
        }
        if (n > 1 && mpz_odd_p(answer[n])) {
            die("reversal parity check failed");
        }
        write_complete_term(stream, n, answer[n]);
        update_peaks(stats, current->count, 0U, coefficient_count);
        if (n == maximum_n) {
            break;
        }

        int next_index = 1 - current_index;
        PolynomialMap *next = &maps[next_index];
        if (next->count != 0U) {
            die("next frontier-state map was not empty");
        }
        for (size_t state_index = 0U;
             state_index < current->count; ++state_index) {
            const PolynomialState *source = &current->item[state_index];
            Transition transitions[MAX_RAW_TRANSITIONS];
            size_t transition_count =
                build_transitions(source->key, frontier_size,
                                  frontier_width, transitions);
            for (size_t transition_index = 0U;
                 transition_index < transition_count;
                 ++transition_index) {
                const Transition *transition =
                    &transitions[transition_index];
                PolynomialState *destination =
                    polynomial_map_get(next, transition->key);
                increment_u64(&stats->state_transition_applications,
                              "state transition");
                for (int edges = 0; edges <= maximum_edges; ++edges) {
                    if (mpz_sgn(source->coefficient[edges]) == 0) {
                        continue;
                    }
                    int destination_edges = edges + transition->edges;
                    if (destination_edges > n) {
                        die("linear-forest polynomial degree exceeds n");
                    }
                    mpz_addmul_ui(destination->coefficient[destination_edges],
                                  source->coefficient[edges],
                                  transition->weight);
                    increment_u64(&stats->polynomial_addmuls,
                                  "polynomial operation");
                }
            }
        }
        update_peaks(stats, current->count, next->count,
                     coefficient_count);
        polynomial_map_destroy(current);
        polynomial_map_init(current, coefficient_count);
        current_index = next_index;
    }

    polynomial_map_destroy(&maps[0]);
    polynomial_map_destroy(&maps[1]);
    mpz_array_destroy(factorial, coefficient_count);
    mpz_array_destroy(q, coefficient_count);
    stats->seconds = monotonic_seconds() - start;
    return answer;
}

static bool next_permutation(int *permutation, int length)
{
    int pivot = length - 2;
    while (pivot >= 0 && permutation[pivot] >= permutation[pivot + 1]) {
        --pivot;
    }
    if (pivot < 0) {
        return false;
    }
    int successor = length - 1;
    while (permutation[successor] <= permutation[pivot]) {
        --successor;
    }
    int temporary = permutation[pivot];
    permutation[pivot] = permutation[successor];
    permutation[successor] = temporary;
    for (int left = pivot + 1, right = length - 1;
         left < right; ++left, --right) {
        temporary = permutation[left];
        permutation[left] = permutation[right];
        permutation[right] = temporary;
    }
    return true;
}

static uint64_t direct_permutation_count(int n, int k)
{
    if (n == 0) {
        return UINT64_C(1);
    }
    int permutation[DIRECT_CHECK_MAX_N];
    for (int index = 0; index < n; ++index) {
        permutation[index] = index;
    }
    uint64_t count = 0U;
    bool more = true;
    while (more) {
        bool valid = true;
        for (int index = 1; index < n; ++index) {
            int difference = permutation[index] - permutation[index - 1];
            if (difference < 0) {
                difference = -difference;
            }
            if (difference < k) {
                valid = false;
                break;
            }
        }
        if (valid) {
            if (count == UINT64_MAX) {
                die("direct permutation count overflow");
            }
            ++count;
        }
        more = next_permutation(permutation, n);
    }
    return count;
}

static int forbidden_edge_count(int n, int k)
{
    int maximum_distance = k - 1;
    if (maximum_distance > n - 1) {
        maximum_distance = n - 1;
    }
    int count = 0;
    for (int distance = 1; distance <= maximum_distance; ++distance) {
        count += n - distance;
    }
    return count;
}

static void direct_linear_forest_formula(int n, int k, mpz_t result)
{
    int edge_left[FOREST_CHECK_MAX_N * (FOREST_CHECK_MAX_N - 1) / 2];
    int edge_right[FOREST_CHECK_MAX_N * (FOREST_CHECK_MAX_N - 1) / 2];
    int edge_count = 0;
    for (int right = 0; right < n; ++right) {
        for (int left = 0; left < right; ++left) {
            if (right - left < k) {
                edge_left[edge_count] = left;
                edge_right[edge_count] = right;
                ++edge_count;
            }
        }
    }
    if (edge_count > FOREST_CHECK_MAX_EDGES) {
        die("direct forest check has too many edge subsets");
    }
    uint64_t q[FOREST_CHECK_MAX_N + 1] = {0U};
    uint64_t subset_count = UINT64_C(1) << edge_count;
    for (uint64_t mask = 0U; mask < subset_count; ++mask) {
        int parent[FOREST_CHECK_MAX_N];
        uint8_t degree[FOREST_CHECK_MAX_N] = {0U};
        for (int vertex = 0; vertex < n; ++vertex) {
            parent[vertex] = vertex;
        }
        bool valid = true;
        int selected_edges = 0;
        for (int edge = 0; edge < edge_count; ++edge) {
            if (((mask >> edge) & UINT64_C(1)) == 0U) {
                continue;
            }
            int left = edge_left[edge];
            int right = edge_right[edge];
            if (++degree[left] > 2U || ++degree[right] > 2U) {
                valid = false;
                break;
            }
            int left_root = left;
            while (parent[left_root] != left_root) {
                left_root = parent[left_root];
            }
            int right_root = right;
            while (parent[right_root] != right_root) {
                right_root = parent[right_root];
            }
            if (left_root == right_root) {
                valid = false;
                break;
            }
            parent[right_root] = left_root;
            ++selected_edges;
        }
        if (!valid) {
            continue;
        }
        bool root_seen[FOREST_CHECK_MAX_N] = {false};
        unsigned components = 0U;
        for (int vertex = 0; vertex < n; ++vertex) {
            if (degree[vertex] == 0U) {
                continue;
            }
            int root = vertex;
            while (parent[root] != root) {
                root = parent[root];
            }
            if (!root_seen[root]) {
                root_seen[root] = true;
                ++components;
            }
        }
        uint64_t weight = UINT64_C(1) << components;
        if (q[selected_edges] > UINT64_MAX - weight) {
            die("direct forest polynomial coefficient overflow");
        }
        q[selected_edges] += weight;
    }

    mpz_t factorial;
    mpz_t term;
    mpz_init(factorial);
    mpz_init(term);
    mpz_set_ui(result, 0UL);
    int maximum_edges = n == 0 ? 0 : n - 1;
    for (int edges = 0; edges <= maximum_edges; ++edges) {
        mpz_fac_ui(factorial, (unsigned long)(n - edges));
        mpz_mul_ui(term, factorial, (unsigned long)q[edges]);
        if ((edges & 1) == 0) {
            mpz_add(result, result, term);
        } else {
            mpz_sub(result, result, term);
        }
    }
    mpz_clear(factorial);
    mpz_clear(term);
}

static int verify_known(const mpz_t *answer, int maximum_n, int k)
{
    if (k != 4) {
        return EXIT_SUCCESS;
    }
    int check_n = maximum_n < KNOWN_MAX_N ? maximum_n : KNOWN_MAX_N;
    mpz_t expected;
    mpz_init(expected);
    for (int n = 0; n <= check_n; ++n) {
        if (mpz_set_str(expected, known_terms[n], 10) != 0) {
            die("invalid built-in OEIS value");
        }
        if (mpz_cmp(answer[n], expected) != 0) {
            gmp_fprintf(stderr,
                        "error: A179957 mismatch at n=%d: %Zd versus %Zd\n",
                        n, answer[n], expected);
            mpz_clear(expected);
            return EXIT_FAILURE;
        }
    }
    mpz_clear(expected);
    return EXIT_SUCCESS;
}

static void print_stats(int maximum_n, int k, const DpStats *stats)
{
    fprintf(stderr,
            "179957_04: k=%d, n=0..%d, linear-forest frontier DP, "
            "peak states=%zu, peak frontier coefficient slots=%zu, "
            "state transitions=%llu, polynomial addmuls=%llu, %.3f s\n",
            k, maximum_n, stats->peak_active_states,
            stats->peak_frontier_coefficient_slots,
            (unsigned long long)stats->state_transition_applications,
            (unsigned long long)stats->polynomial_addmuls,
            stats->seconds);
}

static int check_implementation(int maximum_n, int k)
{
    if (maximum_n > KNOWN_MAX_N) {
        fprintf(stderr, "error: CHECK_N must be in 0..%d: %d\n",
                KNOWN_MAX_N, maximum_n);
        return EXIT_FAILURE;
    }
    DpStats stats;
    mpz_t *answer = compute_sequence(maximum_n, k, NULL, &stats);
    if (verify_known(answer, maximum_n, k) != EXIT_SUCCESS) {
        mpz_array_destroy(answer, (size_t)maximum_n + 1U);
        return EXIT_FAILURE;
    }
    int direct_max = maximum_n < DIRECT_CHECK_MAX_N
                         ? maximum_n : DIRECT_CHECK_MAX_N;
    for (int n = 0; n <= direct_max; ++n) {
        uint64_t direct = direct_permutation_count(n, k);
        if (mpz_cmp_ui(answer[n], (unsigned long)direct) != 0) {
            gmp_fprintf(stderr,
                        "error: frontier/direct-permutation mismatch "
                        "at k=%d, n=%d: %Zd versus %llu\n",
                        k, n, answer[n], (unsigned long long)direct);
            mpz_array_destroy(answer, (size_t)maximum_n + 1U);
            return EXIT_FAILURE;
        }
    }
    int forest_max = 0;
    int forest_limit = maximum_n < FOREST_CHECK_MAX_N
                           ? maximum_n : FOREST_CHECK_MAX_N;
    mpz_t forest;
    mpz_init(forest);
    for (int n = 0; n <= forest_limit; ++n) {
        if (forbidden_edge_count(n, k) > FOREST_CHECK_MAX_EDGES) {
            break;
        }
        direct_linear_forest_formula(n, k, forest);
        if (mpz_cmp(answer[n], forest) != 0) {
            gmp_fprintf(stderr,
                        "error: frontier/direct-linear-forest mismatch "
                        "at k=%d, n=%d: %Zd versus %Zd\n",
                        k, n, answer[n], forest);
            mpz_clear(forest);
            mpz_array_destroy(answer, (size_t)maximum_n + 1U);
            return EXIT_FAILURE;
        }
        forest_max = n;
    }
    mpz_clear(forest);
    print_stats(maximum_n, k, &stats);
    if (k == 4) {
        printf("ok: k=4, A179957 b-file verified through n=%d; ",
               maximum_n);
    } else {
        printf("ok: k=%d; ", k);
    }
    printf("definition-level permutation scan through n=%d; direct "
           "linear-forest inclusion-exclusion through n=%d; reversal "
           "parity checked for n>1\n", direct_max, forest_max);
    mpz_array_destroy(answer, (size_t)maximum_n + 1U);
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
    char *path = xcalloc(directory_length + filename_length + 2U,
                         sizeof(*path));
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

static void produce_file(const char *argv0, int maximum_n, int k)
{
    char final_name[64];
    char part_name[64];
    int final_length = snprintf(final_name, sizeof(final_name),
                                "b179957_04_k%d.txt", k);
    int part_length = snprintf(part_name, sizeof(part_name),
                               "b179957_04_k%d_part.txt", k);
    if (final_length < 0 || (size_t)final_length >= sizeof(final_name) ||
        part_length < 0 || (size_t)part_length >= sizeof(part_name)) {
        die("k-specific output filename is too long");
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
    DpStats stats;
    mpz_t *answer = compute_sequence(maximum_n, k, stream, &stats);
    if (verify_known(answer, maximum_n, k) != EXIT_SUCCESS) {
        (void)fclose(stream);
        mpz_array_destroy(answer, (size_t)maximum_n + 1U);
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }
    if (fclose(stream) != 0) {
        mpz_array_destroy(answer, (size_t)maximum_n + 1U);
        free(final_path);
        free(part_path);
        die("could not close the threshold-k output file");
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        mpz_array_destroy(answer, (size_t)maximum_n + 1U);
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }
    print_stats(maximum_n, k, &stats);
    printf("wrote %s (n=%d..%d)\n",
           final_path, SEQUENCE_OFFSET, maximum_n);
    mpz_array_destroy(answer, (size_t)maximum_n + 1U);
    free(final_path);
    free(part_path);
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K]\n"
            "       %s --upto MAX_N [--k K]\n"
            "       %s --term N [--k K]\n"
            "       %s --check [MAX_N] [--k K]\n"
            "\n"
            "N and MAX_N may be %d..%d; all values use GMP.\n"
            "K may be 1..%d and defaults to %d; large K can hit the "
            "explicit state guard.\n"
            "The default is --upto %d.  A range run writes "
            "b179957_04_kK.txt beside the executable.\n",
            program, program, program, program,
            SEQUENCE_OFFSET, MAX_SUPPORTED_N,
            MAX_SUPPORTED_K, DEFAULT_K, DEFAULT_MAX_N);
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
    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--k") == 0) {
            if (have_k || argument + 1 >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            k = parse_bounded_integer(argv[++argument], "K", 1,
                                      MAX_SUPPORTED_K);
            have_k = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = strcmp(text, "--term") == 0 ? RUN_TERM : RUN_UPTO;
            have_mode = true;
            n = parse_bounded_integer(argv[++argument],
                        mode == RUN_TERM ? "N" : "MAX_N",
                        SEQUENCE_OFFSET, MAX_SUPPORTED_N);
            have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode || have_n) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = RUN_CHECK;
            have_mode = true;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                n = parse_bounded_integer(argv[++argument], "CHECK_N",
                                          SEQUENCE_OFFSET,
                                          MAX_SUPPORTED_N);
                have_n = true;
            }
        } else if (text[0] == '-' || have_n) {
            usage(argv[0]);
            return EXIT_FAILURE;
        } else {
            n = parse_bounded_integer(text,
                        mode == RUN_CHECK ? "CHECK_N" : "MAX_N",
                        SEQUENCE_OFFSET, MAX_SUPPORTED_N);
            have_n = true;
        }
    }
    if (!have_n) {
        n = mode == RUN_CHECK
                ? (k == 4 ? DEFAULT_CHECK_N : DEFAULT_GENERAL_CHECK_N)
                : DEFAULT_MAX_N;
    }
    if (mode == RUN_CHECK) {
        return check_implementation(n, k);
    }
    if (mode == RUN_TERM) {
        DpStats stats;
        mpz_t *answer = compute_sequence(n, k, NULL, &stats);
        if (verify_known(answer, n, k) != EXIT_SUCCESS) {
            mpz_array_destroy(answer, (size_t)n + 1U);
            return EXIT_FAILURE;
        }
        print_stats(n, k, &stats);
        if (gmp_printf("%d %Zd\n", n, answer[n]) < 0) {
            die("could not write result");
        }
        mpz_array_destroy(answer, (size_t)n + 1U);
        return EXIT_SUCCESS;
    }
    produce_file(argv[0], n, k);
    return EXIT_SUCCESS;
}
