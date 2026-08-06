/*
 * Threshold-k minimum-jump permutations -- independent cluster-block DP.
 *
 * A forbidden cluster on a nonempty value set T is an oriented Hamilton path
 * in the graph H_{n,k}, whose edges satisfy 1 <= |u-v| < k.  A cluster of
 * length |T| represents |T|-1 selected bad adjacencies and has weight
 *
 *             w(T) = (-1)^(|T|-1) * C(T),
 *
 * where C(T) is the number of oriented forbidden Hamilton paths on T.
 * A selected bad-edge set is a linear forest.  Ordering its path components
 * as contracted permutation blocks contributes b! for b blocks.  Therefore
 * the answer is the total weight of ordered sequences of disjoint clusters
 * covering [n].  With G(empty)=1,
 *
 *             G(S) = sum_{nonempty T subset S} w(T) G(S\T),
 *             a_k(n) = G([n]).
 *
 * C(T) is computed first by an endpoint subset DP using only forbidden
 * edges.  G is then a genuine Goulden--Jackson-style cluster expansion.
 * This implementation does not use frontier states or the linear-forest
 * transfer of 03/04, so it is an independent exact check.
 *
 * For a single forbidden difference, the components used by the
 * Spahn--Zeilberger tiling method have much more structure.  Simultaneously
 * forbidding differences 1,...,k-1 destroys that simple one-dimensional
 * tiling.  The honest general cluster recurrence above costs O(3^n) time and
 * O(n*2^n) memory; consequently this implementation caps n at 20 and keeps
 * the practical default at 16.  All signed cluster sums use GMP.  Path counts
 * fit uint64_t through n=20 because each is at most 20!, and every addition
 * is checked anyway.  Runs near the cap can take a very long time.
 *
 * Reference for weighted tilings/clusters in forbidden-difference problems:
 * George Spahn and Doron Zeilberger, Counting Permutations Where The
 * Difference Between Entries Located r Places Apart Can Never Be s (2023),
 * https://sites.math.rutgers.edu/~zeilberg/mamarim/mamarimPDF/permsV2.pdf
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     179957_02.c -lgmp -o 179957_02
 *
 * Usage:
 *   ./179957_02 --k 4 --upto 20
 *   ./179957_02 --k 4 --term 20
 *   ./179957_02 --k 4 --check
 */

#define main a179957_04_embedded_main_for_cluster_check
#include "179957_04.c"
#undef main

#define M2_MAX_N 20
#define M2_DEFAULT_N 16

typedef struct {
    size_t masks;
    size_t endpoint_slots;
    uint64_t path_extensions;
    uint64_t cluster_convolution_pairs;
    double seconds;
} M2Stats;

static unsigned m2_popcount(size_t mask)
{
    unsigned count = 0U;
    while (mask != 0U) {
        mask &= mask - 1U;
        ++count;
    }
    return count;
}

static unsigned m2_bit_index(size_t bit)
{
    if (bit == 0U || (bit & (bit - 1U)) != 0U) {
        die("cluster DP received a non-singleton bit");
    }
    unsigned index = 0U;
    while ((bit >> index) != 1U) {
        ++index;
    }
    return index;
}

static mpz_t *m2_compute_sequence(int maximum_n, int k, M2Stats *stats)
{
    memset(stats, 0, sizeof(*stats));
    double started = monotonic_seconds();
    size_t mask_count = (size_t)1U << (unsigned)maximum_n;
    stats->masks = mask_count;
    mpz_t *answer = mpz_array_create((size_t)maximum_n + 1U);
    if (maximum_n == 0) {
        mpz_set_ui(answer[0], 1UL);
        stats->seconds = monotonic_seconds() - started;
        return answer;
    }

    size_t endpoint_slots = checked_product_size(mask_count,
                                                  (size_t)maximum_n);
    stats->endpoint_slots = endpoint_slots;
    uint64_t *path = xcalloc(endpoint_slots, sizeof(*path));
    uint64_t *cluster_count = xcalloc(mask_count,
                                      sizeof(*cluster_count));
    size_t full = mask_count - 1U;

    uint32_t forbidden[M2_MAX_N] = {0U};
    for (int value = 0; value < maximum_n; ++value) {
        for (int other = 0; other < maximum_n; ++other) {
            int difference = value - other;
            if (difference < 0) {
                difference = -difference;
            }
            if (difference > 0 && difference < k) {
                forbidden[value] |= UINT32_C(1) << (unsigned)other;
            }
        }
        size_t singleton = (size_t)1U << (unsigned)value;
        path[checked_product_size(singleton, (size_t)maximum_n) +
             (size_t)value] = UINT64_C(1);
    }

    /* C(T): every oriented forbidden Hamilton path on every subset T. */
    for (size_t mask = 1U; mask <= full; ++mask) {
        size_t base = checked_product_size(mask, (size_t)maximum_n);
        size_t endpoints = mask;
        while (endpoints != 0U) {
            size_t endpoint_bit = endpoints & (0U - endpoints);
            endpoints ^= endpoint_bit;
            unsigned endpoint = m2_bit_index(endpoint_bit);
            uint64_t count = path[base + endpoint];
            if (count == 0U) {
                continue;
            }
            size_t candidates = (size_t)forbidden[endpoint] &
                                (full ^ mask);
            while (candidates != 0U) {
                size_t next_bit = candidates & (0U - candidates);
                candidates ^= next_bit;
                unsigned next = m2_bit_index(next_bit);
                size_t target_mask = mask | next_bit;
                size_t target = checked_product_size(
                                    target_mask, (size_t)maximum_n) + next;
                if (path[target] > UINT64_MAX - count) {
                    free(cluster_count);
                    free(path);
                    mpz_array_destroy(answer, (size_t)maximum_n + 1U);
                    die("oriented cluster path count overflow");
                }
                path[target] += count;
                increment_u64(&stats->path_extensions,
                              "cluster path extension");
            }
        }
    }
    for (size_t mask = 1U; mask <= full; ++mask) {
        size_t base = checked_product_size(mask, (size_t)maximum_n);
        size_t endpoints = mask;
        uint64_t total = 0U;
        while (endpoints != 0U) {
            size_t endpoint_bit = endpoints & (0U - endpoints);
            endpoints ^= endpoint_bit;
            unsigned endpoint = m2_bit_index(endpoint_bit);
            uint64_t addend = path[base + endpoint];
            if (total > UINT64_MAX - addend) {
                free(cluster_count);
                free(path);
                mpz_array_destroy(answer, (size_t)maximum_n + 1U);
                die("cluster count overflow");
            }
            total += addend;
        }
        cluster_count[mask] = total;
    }
    free(path);

    /* Ordered sequences of disjoint cluster blocks. */
    mpz_t *ordered = mpz_array_create(mask_count);
    mpz_set_ui(ordered[0], 1UL);
    for (size_t mask = 1U; mask <= full; ++mask) {
        mpz_set_ui(ordered[mask], 0UL);
        for (size_t block = mask; block != 0U;
             block = (block - 1U) & mask) {
            uint64_t weight = cluster_count[block];
            if (weight == 0U) {
                continue;
            }
            size_t remainder = mask ^ block;
            if ((m2_popcount(block) & 1U) != 0U) {
                /* (-1)^(|T|-1) is positive for odd |T|. */
                mpz_addmul_ui(ordered[mask], ordered[remainder],
                              (unsigned long)weight);
            } else {
                mpz_submul_ui(ordered[mask], ordered[remainder],
                              (unsigned long)weight);
            }
            increment_u64(&stats->cluster_convolution_pairs,
                          "cluster convolution");
        }
    }
    for (int n = 0; n <= maximum_n; ++n) {
        size_t prefix = n == 0 ? 0U
                               : ((size_t)1U << (unsigned)n) - 1U;
        mpz_set(answer[n], ordered[prefix]);
        if (mpz_sgn(answer[n]) < 0 ||
            (n > 1 && mpz_odd_p(answer[n]))) {
            mpz_array_destroy(ordered, mask_count);
            free(cluster_count);
            mpz_array_destroy(answer, (size_t)maximum_n + 1U);
            die("cluster expansion invariant failed");
        }
    }
    mpz_array_destroy(ordered, mask_count);
    free(cluster_count);
    stats->seconds = monotonic_seconds() - started;
    return answer;
}

static void m2_print_stats(int maximum_n, int k, const M2Stats *stats)
{
    fprintf(stderr,
            "179957_02: k=%d, n=0..%d, independent cluster-block DP, "
            "masks=%zu, endpoint slots=%zu, path extensions=%llu, "
            "cluster pairs=%llu, %.3f s\n",
            k, maximum_n, stats->masks, stats->endpoint_slots,
            (unsigned long long)stats->path_extensions,
            (unsigned long long)stats->cluster_convolution_pairs,
            stats->seconds);
}

static int m2_check(int maximum_n, int k)
{
    M2Stats stats;
    mpz_t *answer = m2_compute_sequence(maximum_n, k, &stats);
    if (verify_known(answer, maximum_n, k) != EXIT_SUCCESS) {
        mpz_array_destroy(answer, (size_t)maximum_n + 1U);
        return EXIT_FAILURE;
    }
    DpStats reference_stats;
    mpz_t *reference = compute_sequence(maximum_n, k, NULL,
                                        &reference_stats);
    for (int n = 0; n <= maximum_n; ++n) {
        if (mpz_cmp(answer[n], reference[n]) != 0) {
            gmp_fprintf(stderr,
                        "error: cluster/frontier mismatch at k=%d,n=%d: "
                        "%Zd versus %Zd\n", k, n, answer[n], reference[n]);
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
            fprintf(stderr, "error: cluster/direct mismatch at k=%d,n=%d\n",
                    k, n);
            mpz_array_destroy(reference, (size_t)maximum_n + 1U);
            mpz_array_destroy(answer, (size_t)maximum_n + 1U);
            return EXIT_FAILURE;
        }
    }
    m2_print_stats(maximum_n, k, &stats);
    printf("ok: k=%d, independent cluster expansion agrees with 04 "
           "through n=%d and full permutations through n=%d%s\n",
           k, maximum_n, direct_max,
           k == 4 ? "; A179957 verified" : "");
    mpz_array_destroy(reference, (size_t)maximum_n + 1U);
    mpz_array_destroy(answer, (size_t)maximum_n + 1U);
    return EXIT_SUCCESS;
}

static void m2_write_file(const char *argv0, int maximum_n, int k,
                          const mpz_t *answer)
{
    char final_name[64];
    char part_name[64];
    int a = snprintf(final_name, sizeof(final_name),
                     "b179957_02_k%d.txt", k);
    int b = snprintf(part_name, sizeof(part_name),
                     "b179957_02_k%d_part.txt", k);
    if (a < 0 || b < 0 || (size_t)a >= sizeof(final_name) ||
        (size_t)b >= sizeof(part_name)) {
        die("cluster output filename is too long");
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
    for (int n = 0; n <= maximum_n; ++n) {
        if (gmp_fprintf(stream, "%d %Zd\n", n, answer[n]) < 0 ||
            fflush(stream) != 0) {
            (void)fclose(stream);
            free(final_path);
            free(part_path);
            die("could not write cluster output file");
        }
    }
    if (fclose(stream) != 0 || rename(part_path, final_path) != 0) {
        free(final_path);
        free(part_path);
        die("could not finish cluster output file");
    }
    printf("wrote %s (n=0..%d)\n", final_path, maximum_n);
    free(final_path);
    free(part_path);
}

static void m2_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K]\n"
            "       %s --upto MAX_N [--k K]\n"
            "       %s --term N [--k K]\n"
            "       %s --check [MAX_N] [--k K]\n"
            "K is 1..10 (default 4); N is 0..%d.\n"
            "A range run writes b179957_02_kK.txt.\n",
            program, program, program, program, M2_MAX_N);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        m2_usage(argv[0]);
        return EXIT_SUCCESS;
    }
    RunMode mode = RUN_UPTO;
    int n = -1;
    int k = DEFAULT_K;
    bool have_n = false;
    bool have_k = false;
    bool have_mode = false;
    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--k") == 0) {
            if (have_k || argument + 1 >= argc) {
                m2_usage(argv[0]);
                return EXIT_FAILURE;
            }
            k = parse_bounded_integer(argv[++argument], "K", 1,
                                      MAX_SUPPORTED_K);
            have_k = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                m2_usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = strcmp(text, "--term") == 0 ? RUN_TERM : RUN_UPTO;
            have_mode = true;
            n = parse_bounded_integer(argv[++argument],
                        mode == RUN_TERM ? "N" : "MAX_N", 0, M2_MAX_N);
            have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode || have_n) {
                m2_usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = RUN_CHECK;
            have_mode = true;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                n = parse_bounded_integer(argv[++argument], "CHECK_N", 0,
                                          M2_MAX_N);
                have_n = true;
            }
        } else if (text[0] == '-' || have_n) {
            m2_usage(argv[0]);
            return EXIT_FAILURE;
        } else {
            n = parse_bounded_integer(text,
                        mode == RUN_CHECK ? "CHECK_N" : "MAX_N",
                        0, M2_MAX_N);
            have_n = true;
        }
    }
    if (!have_n) {
        n = mode == RUN_CHECK
                ? (k == 4 ? M2_MAX_N : DEFAULT_GENERAL_CHECK_N)
                : M2_DEFAULT_N;
    }
    if (mode == RUN_CHECK) {
        return m2_check(n, k);
    }
    M2Stats stats;
    mpz_t *answer = m2_compute_sequence(n, k, &stats);
    if (verify_known(answer, n, k) != EXIT_SUCCESS) {
        mpz_array_destroy(answer, (size_t)n + 1U);
        return EXIT_FAILURE;
    }
    m2_print_stats(n, k, &stats);
    if (mode == RUN_TERM) {
        gmp_printf("%d %Zd\n", n, answer[n]);
    } else {
        m2_write_file(argv[0], n, k, (const mpz_t *)answer);
    }
    mpz_array_destroy(answer, (size_t)n + 1U);
    return EXIT_SUCCESS;
}
