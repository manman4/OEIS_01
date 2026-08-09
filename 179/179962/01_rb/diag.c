/*
 * diag.c -- A_k(n) = #{permutations p of [2n+k] : |p(i+1)-p(i)| > n}
 *           by the insertion dynamic programming of proved_results.md (Lemma C2),
 *           in exact GMP arithmetic, optionally parallelised with OpenMP.
 *
 * NO RECURRENCE IS USED TO PRODUCE VALUES.  Every A_k(n) printed by this
 * program is counted from the combinatorial definition via the certified
 * finite-state representation, independently of whether C_k has been proved
 * for that k.  The recurrence appears only in --certify, where it is the
 * object being verified, never a source of values.
 *
 * The transition tables are exported from the Ruby reference implementation
 * (export_machine.rb), which is the one checked against brute-force
 * enumeration and against the independent inclusion-exclusion DP.  This file
 * only performs the sparse matrix-vector products over Z.
 *
 * Build (Linux):
 *   cc -O3 -fopenmp diag.c -o diag -lgmp
 * Build (macOS, with the alias from the project notes):
 *   gcc-omp diag.c -o diag -lgmp
 * Build without OpenMP:
 *   cc -O3 diag.c -o diag -lgmp
 *
 * Usage:
 *   ruby export_machine.rb K            # writes machine_kK.txt (once per k)
 *   ./diag machine_k8.txt --upto 1000   # print A_8(n), n = k-2 .. 1000
 *   ./diag machine_k8.txt --term 500    # print a single term
 *   ./diag machine_k8.txt --certify     # prove the C_k recurrence (Lemma 10.1)
 *
 * Options:
 *   --upto N     print all terms up to n = N (default 100)
 *   --term N     print only n = N
 *   --certify    run the finite-window certificate for C_k
 *   --quiet      suppress progress on stderr
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* ------------------------------------------------------------------ */
/* sparse layer, stored by target for race-free parallel accumulation   */

typedef struct {
    int rows;      /* source dimension */
    int cols;      /* target dimension */
    int nnz;
    int *start;    /* cols+1 offsets into src/mul */
    int *src;      /* nnz source indices */
    unsigned long *mul;
} Layer;

static void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static void layer_build(Layer *layer, int rows, int cols, int nnz,
                        const int *from, const int *to,
                        const unsigned long *mult)
{
    layer->rows = rows;
    layer->cols = cols;
    layer->nnz = nnz;
    layer->start = calloc((size_t)cols + 1, sizeof(int));
    layer->src = malloc((size_t)nnz * sizeof(int));
    layer->mul = malloc((size_t)nnz * sizeof(unsigned long));
    if (!layer->start || !layer->src || !layer->mul) {
        die("out of memory building a layer");
    }
    for (int e = 0; e < nnz; ++e) {
        layer->start[to[e] + 1]++;
    }
    for (int c = 0; c < cols; ++c) {
        layer->start[c + 1] += layer->start[c];
    }
    int *fill = malloc((size_t)cols * sizeof(int));
    if (!fill) {
        die("out of memory building a layer");
    }
    memcpy(fill, layer->start, (size_t)cols * sizeof(int));
    for (int e = 0; e < nnz; ++e) {
        int slot = fill[to[e]]++;
        layer->src[slot] = from[e];
        layer->mul[slot] = mult[e];
    }
    free(fill);
}

/* dst = layer * source */
static void layer_apply(const Layer *layer, mpz_t *source, mpz_t *dst)
{
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 64)
#endif
    for (int c = 0; c < layer->cols; ++c) {
        mpz_set_ui(dst[c], 0UL);
        for (int e = layer->start[c]; e < layer->start[c + 1]; ++e) {
            mpz_addmul_ui(dst[c], source[layer->src[e]], layer->mul[e]);
        }
    }
}

/* ------------------------------------------------------------------ */
/* the machine as exported by export_machine.rb                         */

typedef struct {
    int k;
    int nstates;
    int head_len;
    int *head_idx;
    unsigned long *head_coef;
    Layer bulk;
    int tail_count;
    Layer *tail;
    int accept_len;
    int *accept;
    int max_dim;
} Machine;

static void read_token(FILE *stream, const char *want)
{
    char buffer[64];
    if (fscanf(stream, "%63s", buffer) != 1 || strcmp(buffer, want) != 0) {
        fprintf(stderr, "error: expected '%s' in the machine file\n", want);
        exit(EXIT_FAILURE);
    }
}

static void machine_read(Machine *machine, const char *path)
{
    FILE *stream = fopen(path, "r");
    if (!stream) {
        die("cannot open the machine file (run export_machine.rb first)");
    }
    read_token(stream, "k");
    if (fscanf(stream, "%d", &machine->k) != 1) die("bad machine file");

    read_token(stream, "head");
    if (fscanf(stream, "%d", &machine->head_len) != 1) die("bad machine file");
    machine->head_idx = malloc((size_t)machine->head_len * sizeof(int));
    machine->head_coef =
        malloc((size_t)machine->head_len * sizeof(unsigned long));
    if (!machine->head_idx || !machine->head_coef) die("out of memory");
    for (int i = 0; i < machine->head_len; ++i) {
        if (fscanf(stream, "%d %lu", &machine->head_idx[i],
                   &machine->head_coef[i]) != 2) die("bad machine file");
    }

    read_token(stream, "bulk");
    int rows, nnz;
    if (fscanf(stream, "%d %d", &rows, &nnz) != 2) die("bad machine file");
    machine->nstates = rows;
    machine->max_dim = rows;
    {
        int *from = malloc((size_t)nnz * sizeof(int));
        int *to = malloc((size_t)nnz * sizeof(int));
        unsigned long *mult = malloc((size_t)nnz * sizeof(unsigned long));
        if (!from || !to || !mult) die("out of memory");
        for (int e = 0; e < nnz; ++e) {
            if (fscanf(stream, "%d %d %lu", &from[e], &to[e], &mult[e]) != 3) {
                die("bad machine file");
            }
        }
        layer_build(&machine->bulk, rows, rows, nnz, from, to, mult);
        free(from); free(to); free(mult);
    }

    read_token(stream, "tail");
    if (fscanf(stream, "%d", &machine->tail_count) != 1) die("bad machine file");
    machine->tail = calloc((size_t)machine->tail_count, sizeof(Layer));
    if (!machine->tail) die("out of memory");
    for (int j = 0; j < machine->tail_count; ++j) {
        read_token(stream, "layer");
        int lr, lc, ln;
        if (fscanf(stream, "%d %d %d", &lr, &lc, &ln) != 3) die("bad machine file");
        int *from = malloc((size_t)ln * sizeof(int));
        int *to = malloc((size_t)ln * sizeof(int));
        unsigned long *mult = malloc((size_t)ln * sizeof(unsigned long));
        if (!from || !to || !mult) die("out of memory");
        for (int e = 0; e < ln; ++e) {
            if (fscanf(stream, "%d %d %lu", &from[e], &to[e], &mult[e]) != 3) {
                die("bad machine file");
            }
        }
        layer_build(&machine->tail[j], lr, lc, ln, from, to, mult);
        free(from); free(to); free(mult);
        if (lc > machine->max_dim) machine->max_dim = lc;
    }

    read_token(stream, "accept");
    if (fscanf(stream, "%d", &machine->accept_len) != 1) die("bad machine file");
    machine->accept = malloc((size_t)machine->accept_len * sizeof(int));
    if (!machine->accept) die("out of memory");
    for (int i = 0; i < machine->accept_len; ++i) {
        if (fscanf(stream, "%d", &machine->accept[i]) != 1) die("bad machine file");
    }
    fclose(stream);
}

/* ------------------------------------------------------------------ */
/* C_k, used only by --certify                                          */

typedef struct {
    int degree;
    long coef[8];   /* a(n) = sum_i coef[i-1] * a(n-i) */
} Recurrence;

static Recurrence build_recurrence(int k)
{
    long roots[8];
    int count = 0;
    for (int r = 1; r <= k / 2; ++r) {
        roots[count++] = (long)r * (k - r);
    }
    if (k % 2 == 1) {
        roots[count++] = (long)(k / 2) * (k - k / 2);   /* middle root doubled */
    }
    /* expand prod (X - root) */
    long poly[9];
    memset(poly, 0, sizeof(poly));
    poly[0] = 1;
    int degree = 0;
    for (int i = 0; i < count; ++i) {
        for (int j = degree + 1; j > 0; --j) {
            poly[j] = poly[j - 1] - roots[i] * poly[j];
        }
        poly[0] = -roots[i] * poly[0];
        ++degree;
    }
    /* poly[degree] = 1; a(n) = -sum_{i=1..degree} poly[degree-i] a(n-i) */
    Recurrence recurrence;
    recurrence.degree = degree;
    for (int i = 1; i <= degree; ++i) {
        recurrence.coef[i - 1] = -poly[degree - i];
    }
    return recurrence;
}

/* ------------------------------------------------------------------ */

typedef struct {
    Machine *machine;
    mpz_t *state;       /* current bulk vector */
    mpz_t *scratch;     /* bulk scratch */
    mpz_t *tail_a;
    mpz_t *tail_b;
    int n;              /* n of the current state vector */
} Run;

static mpz_t *vector_new(int size)
{
    mpz_t *vector = malloc((size_t)size * sizeof(mpz_t));
    if (!vector) die("out of memory");
    for (int i = 0; i < size; ++i) {
        mpz_init(vector[i]);
    }
    return vector;
}

static void run_init(Run *run, Machine *machine)
{
    run->machine = machine;
    run->state = vector_new(machine->nstates);
    run->scratch = vector_new(machine->nstates);
    run->tail_a = vector_new(machine->max_dim);
    run->tail_b = vector_new(machine->max_dim);
    for (int i = 0; i < machine->head_len; ++i) {
        mpz_set_ui(run->state[machine->head_idx[i]], machine->head_coef[i]);
    }
    run->n = machine->k - 3;    /* the head leaves us at this n */
}

/* A_k(run->n) into value */
static void run_value(Run *run, mpz_t value)
{
    Machine *machine = run->machine;
    mpz_t *source = run->state;
    mpz_t *a = run->tail_a;
    mpz_t *b = run->tail_b;
    for (int j = 0; j < machine->tail_count; ++j) {
        layer_apply(&machine->tail[j], source, a);
        source = a;
        mpz_t *swap = a; a = b; b = swap;
    }
    mpz_set_ui(value, 0UL);
    for (int i = 0; i < machine->accept_len; ++i) {
        mpz_add(value, value, source[machine->accept[i]]);
    }
    mpz_mul_2exp(value, value, 1);   /* orient the path */
}

static void run_advance(Run *run)
{
    layer_apply(&run->machine->bulk, run->state, run->scratch);
    mpz_t *swap = run->state;
    run->state = run->scratch;
    run->scratch = swap;
    run->n += 1;
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s MACHINE_FILE [--upto N | --term N | --certify] [--quiet]\n"
            "  the machine file comes from:  ruby export_machine.rb K\n",
            program);
}

int main(int argc, char **argv)
{
    if (argc < 2) { usage(argv[0]); return EXIT_FAILURE; }
    const char *path = argv[1];
    int mode_certify = 0, quiet = 0;
    long target = 100, only = -1;
    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "--certify")) {
            mode_certify = 1;
        } else if (!strcmp(argv[i], "--quiet")) {
            quiet = 1;
        } else if (!strcmp(argv[i], "--upto") && i + 1 < argc) {
            target = strtol(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "--term") && i + 1 < argc) {
            only = strtol(argv[++i], NULL, 10);
            target = only;
        } else {
            usage(argv[0]); return EXIT_FAILURE;
        }
    }

    Machine machine;
    machine_read(&machine, path);
    Recurrence recurrence = build_recurrence(machine.k);

    if (!quiet) {
        fprintf(stderr, "diag: k=%d, %d live states", machine.k, machine.nstates);
#ifdef _OPENMP
        fprintf(stderr, ", OpenMP with %d threads", omp_get_max_threads());
#else
        fprintf(stderr, ", single threaded");
#endif
        fprintf(stderr, "\n");
    }

    Run run;
    run_init(&run, &machine);

    clock_t begin = clock();
#ifdef _OPENMP
    double wall_begin = omp_get_wtime();
#endif

    if (mode_certify) {
        int degree = recurrence.degree;
        int need = machine.nstates + 1;              /* safe side: D+1 residuals */
        long last_n = machine.k + need + degree + 8;
        if (!quiet) {
            fprintf(stderr,
                    "diag: certification needs %d consecutive zero residuals; "
                    "generating n up to %ld\n", need, last_n);
        }
        mpz_t *window = vector_new(degree);
        mpz_t value, rhs;
        mpz_init(value); mpz_init(rhs);
        long first_ok = -1, streak = 0;
        for (long n = run.n; n <= last_n; ++n) {
            run_value(&run, value);
            if (n >= machine.k + degree) {
                mpz_set_ui(rhs, 0UL);
                for (int i = 1; i <= degree; ++i) {
                    long c = recurrence.coef[i - 1];
                    mpz_t *prior = &window[(n - i) % degree];
                    if (c >= 0) {
                        mpz_addmul_ui(rhs, *prior, (unsigned long)c);
                    } else {
                        mpz_submul_ui(rhs, *prior, (unsigned long)(-c));
                    }
                }
                if (mpz_cmp(value, rhs) == 0) {
                    if (streak == 0) first_ok = n;
                    ++streak;
                } else {
                    streak = 0;
                    first_ok = -1;
                    if (!quiet) {
                        fprintf(stderr, "diag: residual nonzero at n=%ld\n", n);
                    }
                }
            }
            mpz_set(window[n % degree], value);
            if (!quiet && n % 200 == 0) {
                fprintf(stderr, "  n=%ld  streak=%ld  [%.1fs]\n", n, streak,
                        (double)(clock() - begin) / CLOCKS_PER_SEC);
            }
            if (streak >= need) {
                printf("certified: C_%d holds for all n >= %ld "
                       "(%ld consecutive zero residuals, needed %d)\n",
                       machine.k, first_ok, streak, need);
                return EXIT_SUCCESS;
            }
            if (n < last_n) run_advance(&run);
        }
        printf("not certified: longest zero-residual streak %ld, needed %d\n",
               streak, need);
        return EXIT_FAILURE;
    }

    mpz_t value;
    mpz_init(value);
    for (long n = run.n; n <= target; ++n) {
        if (only < 0 || n == only) {
            run_value(&run, value);
            gmp_printf("%ld %Zd\n", n, value);
        }
        if (n < target) run_advance(&run);
    }
    if (!quiet) {
        double seconds = (double)(clock() - begin) / CLOCKS_PER_SEC;
#ifdef _OPENMP
        seconds = omp_get_wtime() - wall_begin;
#endif
        fprintf(stderr, "diag: done in %.2f s\n", seconds);
    }
    return EXIT_SUCCESS;
}
