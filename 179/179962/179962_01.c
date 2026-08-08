/*
 * A179962 and the general 2*n+k family -- exact reference implementation.
 *
 * Count permutations p of [2*n+k] satisfying
 *
 *             |p(i+1)-p(i)| > n.
 *
 * Equivalently, this is the number of directed Hamilton paths in the graph
 * on [2*n+k] in which {u,v} is an edge iff |u-v| >= n+1.
 *
 * This file deliberately implements the definition, not a guessed linear
 * recurrence.  It reuses the independently checked, layered Held--Karp
 * kernel in 179957_01.c.  If M=2*n+k and D=n+1, that kernel evaluates
 *
 *   H(S,v) = Sum_{u in S-{v}, |u-v|>=D} H(S-{v},u)
 *
 * exactly.  Only two subset-cardinality layers are stored.  For a fixed
 * subset S only its |S| possible endpoints are allocated.
 *
 * Complexity (M=2*n+k):
 *   time   O(M^2 * 2^M) in the present total-minus-local implementation;
 *   memory O(M * binomial(M,floor(M/2)) + 2^M).
 *
 * Thus this is a safe exponential reference/checking program, not yet the
 * desired fixed-k polynomial-time Ferrers/path-cover transfer.  It is still
 * enormously faster than checking all M! permutations and is useful for
 * proving initial terms and checking a future specialized implementation.
 *
 * Numeric and allocation safety:
 *   - M is at most 34 and every DP entry is at most M! < 2^128;
 *   - the imported kernel checks every addition, subtraction, shift, index,
 *     allocation-size product, and counter increment;
 *   - --memory-mib is a hard allocation budget;
 *   - output is written to a _part.txt file and atomically renamed only
 *     after all requested terms have completed.
 *
 * Independent checks:
 *   - OEIS terms for k=3,...,8 are built in (including A179962 for k=4);
 *   - definition-level permutation enumeration is used when 2*n+k <= 10;
 *   - a separately written forward subset DP is used when 2*n+k <= 16;
 *   - reversal parity is checked whenever 2*n+k > 1.
 *
 * Build (179957_01.c must be beside this file):
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     179962_01.c -o 179962_01
 *
 * Usage:
 *
 *   ./179962_01 --k 4 --upto 8
 *   ./179962_01 --k 5 --term 8 --memory-mib 2048
 *   ./179962_01 --k 4 --check 8
 *
 * The family parameter is called k here.  Do not confuse it with the
 * threshold option of 179957_01: internally that threshold is n+1.
 *
 * References:
 *   https://oeis.org/A179962
 *   R. Ehrenborg and S. van Willigenburg,
 *   Enumerative properties of Ferrers graphs,
 *   https://arxiv.org/abs/0706.2918
 */

/* Import the audited exact subset-DP kernel under a private main name. */
#define main a179957_imported_main
#include "179957_01.c"
#undef main

#define FAMILY_DEFAULT_K 4
#define FAMILY_DEFAULT_MAX_N 8
#define FAMILY_MAX_TOTAL 34
#define FAMILY_MAX_K 34

typedef enum {
    FAMILY_RUN_UPTO,
    FAMILY_RUN_TERM,
    FAMILY_RUN_CHECK
} FamilyRunMode;

typedef struct {
    int k;
    int maximum_n;
    FamilyRunMode mode;
    uint64_t memory_mib;
} FamilyOptions;

typedef struct {
    int k;
    int last_n;
    const char *const *term;
} KnownFamily;

static const char *const known_k3[] = {
    "6", "14", "32", "72", "160", "352", "768", "1664",
    "3584", "7680"
};

static const char *const known_k4[] = {
    "24", "90", "368", "1496", "6056", "24440", "98408",
    "395576", "1588136", "6370040", "25532648", "102288056"
};

static const char *const known_k5[] = {
    "120", "646", "3984", "25384", "161136", "1019616",
    "6433728", "40495488", "254319360", "1593945600"
};

static const char *const known_k6[] = {
    "720", "5242", "44304", "399848", "3661824", "33461568",
    "305193408", "2779045440"
};

static const char *const known_k7[] = {
    "5040", "47622", "521606", "6231544", "76972416",
    "957913824", "11902581120"
};

static const char *const known_k8[] = {
    "40320", "479306", "6564318", "99133496", "1572313392",
    "25415753280"
};

static const KnownFamily known_families[] = {
    {3, (int)(sizeof(known_k3) / sizeof(known_k3[0])) - 1, known_k3},
    {4, (int)(sizeof(known_k4) / sizeof(known_k4[0])) - 1, known_k4},
    {5, (int)(sizeof(known_k5) / sizeof(known_k5[0])) - 1, known_k5},
    {6, (int)(sizeof(known_k6) / sizeof(known_k6[0])) - 1, known_k6},
    {7, (int)(sizeof(known_k7) / sizeof(known_k7[0])) - 1, known_k7},
    {8, (int)(sizeof(known_k8) / sizeof(known_k8[0])) - 1, known_k8}
};

static int family_parse_nonnegative(const char *text, const char *label,
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

static int family_total_size(int n, int k)
{
    if (n < 0 || k < 0 || n > (INT_MAX - k) / 2) {
        die("2*n+k overflows int");
    }
    int total = 2 * n + k;
    if (total > FAMILY_MAX_TOTAL) {
        fprintf(stderr,
                "error: this exact reference DP requires 2*n+k <= %d; "
                "got n=%d, k=%d, total=%d\n",
                FAMILY_MAX_TOTAL, n, k, total);
        exit(EXIT_FAILURE);
    }
    return total;
}

static const char *family_known_term(int k, int n)
{
    for (size_t index = 0;
         index < sizeof(known_families) / sizeof(known_families[0]);
         ++index) {
        if (known_families[index].k == k &&
            n <= known_families[index].last_n) {
            return known_families[index].term[n];
        }
    }
    return NULL;
}

static void family_verify_value(Count128 value, int n, int k,
                                bool run_independent)
{
    int total = family_total_size(n, k);
    int threshold = n + 1;
    const char *known = family_known_term(k, n);
    if (known != NULL) {
        Count128 expected = parse_count(known);
        if (!count_equal(value, expected)) {
            fprintf(stderr,
                    "error: known family mismatch at n=%d, k=%d: got ",
                    n, k);
            (void)fprint_count(stderr, value);
            fprintf(stderr, ", expected %s\n", known);
            exit(EXIT_FAILURE);
        }
    }

    if (total > 1 && (value.low & UINT64_C(1)) != 0U) {
        fprintf(stderr,
                "error: reversal parity failed at n=%d, k=%d\n", n, k);
        exit(EXIT_FAILURE);
    }

    if (!run_independent) {
        return;
    }
    if (total <= DIRECT_CHECK_MAX_N) {
        Count128 direct = direct_count(total, threshold);
        if (!count_equal(value, direct)) {
            fprintf(stderr,
                    "error: definition scan mismatch at n=%d, k=%d\n",
                    n, k);
            exit(EXIT_FAILURE);
        }
    }
    if (total <= INDEPENDENT_CHECK_MAX_N) {
        Count128 forward =
            count_from_u64(independent_forward_dp(total, threshold));
        if (!count_equal(value, forward)) {
            fprintf(stderr,
                    "error: independent forward-DP mismatch at n=%d, "
                    "k=%d\n", n, k);
            exit(EXIT_FAILURE);
        }
    }
}

static Count128 family_compute_value(int n, int k, uint64_t memory_mib,
                                     bool verbose,
                                     bool run_independent)
{
    int total = family_total_size(n, k);
    int threshold = n + 1;
    DpStats stats;
    Count128 value = compute_dp(total, threshold, memory_mib, &stats);
    family_verify_value(value, n, k, run_independent);
    if (verbose) {
        fprintf(stderr,
                "179962_01: n=%d, k=%d, total=%d, threshold=%d, "
                "exact layered subset DP, subsets=%" PRIu64
                ", states=%" PRIu64 ", local subtractions=%" PRIu64
                ", peak memory=%.1f MiB, %.3f s\n",
                n, k, total, threshold, stats.subsets, stats.states,
                stats.local_subtractions,
                (double)stats.peak_bytes / 1048576.0, stats.seconds);
    }
    return value;
}

static void family_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K] [--memory-mib M]\n"
            "       %s --upto MAX_N [--k K] [--memory-mib M]\n"
            "       %s --term N [--k K] [--memory-mib M]\n"
            "       %s --check [MAX_N] [--k K] [--memory-mib M]\n"
            "\n"
            "Counts permutations of [2*n+k] whose adjacent absolute "
            "differences are > n.\n"
            "This reference implementation requires 2*n+k <= %d.\n"
            "Defaults: --k %d --upto %d --memory-mib %" PRIu64 ".\n",
            program, program, program, program, FAMILY_MAX_TOTAL,
            FAMILY_DEFAULT_K, FAMILY_DEFAULT_MAX_N, DEFAULT_MEMORY_MIB);
}

static FamilyOptions family_parse_options(int argc, char **argv)
{
    FamilyOptions options;
    options.k = FAMILY_DEFAULT_K;
    options.maximum_n = FAMILY_DEFAULT_MAX_N;
    options.mode = FAMILY_RUN_UPTO;
    options.memory_mib = DEFAULT_MEMORY_MIB;
    bool have_k = false;
    bool have_n = false;
    bool have_mode = false;
    bool have_memory = false;

    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--help") == 0 || strcmp(text, "-h") == 0) {
            family_usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(text, "--k") == 0) {
            if (have_k || argument + 1 >= argc) {
                family_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.k = family_parse_nonnegative(
                argv[++argument], "K", FAMILY_MAX_K);
            have_k = true;
        } else if (strcmp(text, "--memory-mib") == 0) {
            if (have_memory || argument + 1 >= argc) {
                family_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.memory_mib = parse_memory_mib(argv[++argument]);
            have_memory = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                family_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.mode = strcmp(text, "--term") == 0
                               ? FAMILY_RUN_TERM
                               : FAMILY_RUN_UPTO;
            options.maximum_n = family_parse_nonnegative(
                argv[++argument],
                options.mode == FAMILY_RUN_TERM ? "N" : "MAX_N",
                FAMILY_MAX_TOTAL / 2);
            have_mode = true;
            have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode) {
                family_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.mode = FAMILY_RUN_CHECK;
            have_mode = true;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                options.maximum_n = family_parse_nonnegative(
                    argv[++argument], "CHECK_N", FAMILY_MAX_TOTAL / 2);
                have_n = true;
            }
        } else if (text[0] != '-') {
            if (have_n) {
                family_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            options.maximum_n = family_parse_nonnegative(
                text, "MAX_N", FAMILY_MAX_TOTAL / 2);
            have_n = true;
        } else {
            family_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    (void)family_total_size(options.maximum_n, options.k);
    return options;
}

static void family_write_file(const char *argv0, int maximum_n, int k,
                              uint64_t memory_mib)
{
    char final_name[80];
    char part_name[80];
    int final_length = snprintf(final_name, sizeof(final_name),
                                "b179962_01_k%d.txt", k);
    int part_length = snprintf(part_name, sizeof(part_name),
                               "b179962_01_k%d_part.txt", k);
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

    for (int n = 0; n <= maximum_n; ++n) {
        Count128 value = family_compute_value(
            n, k, memory_mib, true, false);
        if (fprintf(stream, "%d ", n) < 0 ||
            fprint_count(stream, value) != 0 ||
            fputc('\n', stream) == EOF || fflush(stream) != 0) {
            (void)fclose(stream);
            free(final_path);
            free(part_path);
            die("could not write the family output file");
        }
    }
    if (fclose(stream) != 0) {
        free(final_path);
        free(part_path);
        die("could not close the family output file");
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }
    printf("wrote %s (n=0..%d, k=%d)\n", final_path, maximum_n, k);
    free(final_path);
    free(part_path);
}

static int family_check(int maximum_n, int k, uint64_t memory_mib)
{
    for (int n = 0; n <= maximum_n; ++n) {
        (void)family_compute_value(n, k, memory_mib, false, true);
    }
    printf("ok: exact layered subset DP; OEIS values where available; "
           "definition scan for 2*n+k<=%d; independent forward DP for "
           "2*n+k<=%d; reversal parity (n=0..%d, k=%d)\n",
           DIRECT_CHECK_MAX_N, INDEPENDENT_CHECK_MAX_N, maximum_n, k);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    /* Mark the imported CLI entry point used; only its kernel is called. */
    (void)&a179957_imported_main;
    FamilyOptions options = family_parse_options(argc, argv);
    if (options.mode == FAMILY_RUN_CHECK) {
        return family_check(options.maximum_n, options.k,
                            options.memory_mib);
    }
    if (options.mode == FAMILY_RUN_TERM) {
        Count128 value = family_compute_value(
            options.maximum_n, options.k, options.memory_mib, true, false);
        if (printf("%d ", options.maximum_n) < 0 ||
            fprint_count(stdout, value) != 0 || putchar('\n') == EOF) {
            die("could not write the requested term");
        }
        return EXIT_SUCCESS;
    }
    family_write_file(argv[0], options.maximum_n, options.k,
                      options.memory_mib);
    return EXIT_SUCCESS;
}
