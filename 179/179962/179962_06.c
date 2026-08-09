/*
 * 179962_06.c -- exact insertion-transfer/Krylov computation of
 *
 *   A_k(n) = #{permutations p of [2*n+k] : |p(i+1)-p(i)| > n}.
 *
 * MATHEMATICAL STATUS
 * ===================
 * No scalar recurrence is used by --upto or --term.  Those modes evaluate
 * an exact sparse Krylov sequence over Z:
 *
 *                 A_k(n) = 2 f U T^(n-k+3) h.                 (1)
 *
 * h is the head vector, T the stationary insertion transfer, U the k-2
 * closing layers, and f the acceptance row.  All coefficients are
 * nonnegative integers and all arithmetic is GMP exact arithmetic.
 *
 * The companion 179962_06_machine.rb constructs h,T,U,f from the transition
 * definition below.  It never consults sequence values or a recurrence.
 * Thus a wrong conjectural recurrence cannot corrupt a value printed here.
 *
 * PROOF OF THE INSERTION TRANSFER
 * ===============================
 * Put G_{n,k}=([2n+k],E), where {u,v} is an edge iff |u-v|>n.  A requested
 * permutation is precisely an oriented Hamilton path of G_{n,k}; reversal
 * gives the factor 2 in (1).
 *
 * Start with the isolated vertices 1,...,n+1.  At step t=1,...,n+k-1 insert
 * w=n+1+t.  Its already inserted neighbours are exactly 1,...,t.  Hence
 * every edge of every partial linear forest is created exactly once: when
 * its larger endpoint is inserted.  Attaching w to zero, one, or two free
 * endpoints therefore enumerates all linear forests without omission or
 * duplication; rejecting a repeated component endpoint rejects exactly the
 * choices that create a cycle.
 *
 * A vertex is "awake" from step equal to its label among 1,...,n+1; after
 * waking it is adjacent to every subsequently inserted vertex.  The first
 * k-2 inserted large vertices are capsules C_s: they too wake later.  All
 * other inserted large endpoints are frozen F.  Consequently the future of
 * a partial forest depends only on
 *
 *   (d,a,aa,af,sp),
 *
 * where d=2t-e, a is the number of awake isolated vertices, aa and af count
 * components with endpoint types (A,A) and (A,F), and sp is the sorted list
 * of all components involving a labelled capsule (plus isolated capsules).
 * Vertices/components in the same class are exchangeable because they have
 * identical future neighbourhoods.  Choosing endpoints from each class
 * gives exactly the binomial/product multiplicities used by the generator.
 * This proves by induction on t that every state coefficient equals the
 * number of partial linear forests represented by that state.
 *
 * The pruning is lossless.  At final step T=n+k-1 a Hamilton path has
 * 2n+k-1 edges, so d(T)=2T-e(T)=k-1.  Since each step adds at most two edges,
 * d is nondecreasing; a completable state therefore has d<=k-1.  A frozen
 * isolated vertex, more than two frozen endpoints, or a closed (F,F)
 * component before the last step can never be repaired.  Conversely, at the
 * last step the accepted state has no isolated vertex and exactly one
 * component, hence is a spanning path.  This proves (1), including the
 * state invariant, every transition multiplicity, and the output map.
 *
 * FINITENESS AND THE KRYLOV CERTIFICATE
 * =====================================
 * Every created edge has at least one awake endpoint.  Thus the unused
 * capacity of awake vertices is at most d<=k-1.  There are also at most two
 * F endpoints and k-2 labelled capsules.  The number D of live states is
 * therefore finite for fixed k, independent of n.  Cayley-Hamilton proves
 * that every scalar sequence (1) satisfies a recurrence of order at most D.
 *
 * --certify tests the proposed C_k polynomial on the exact scalar Krylov
 * sequence.  If D consecutive residuals vanish, the residual sequence,
 * itself of the form g*T^m*x in dimension D, vanishes forever by
 * Cayley-Hamilton.  This is a deterministic proof, not recurrence fitting.
 * The program asks for D+1 zeros as a safety margin.  In particular, a
 * successful k=9 run turns the k=9 formula from a conjecture into a theorem.
 *
 * This implementation is in the same mathematical insertion-transfer family
 * as the earlier Ruby work, but its value engine is a separate C/GMP sparse
 * Krylov implementation.  The readable generator and the C consumer are
 * separated so that the transition table is reproducible and inspectable.
 *
 * IMPLEMENTATION AUDIT
 * ====================
 * The generator marks the proof-to-code correspondence P1--P7 at the actual
 * branches: P1 wake/capsule-open; P2 insertion arity; P3 two-slot
 * multiplicities and cycle exclusion; P4 one-slot insertion and d+=1; P5
 * zero-slot capsule and d+=2; P6 reachable/co-reachable pruning; P7 accepting
 * state.  The remaining decomposition is exact because the step procedure
 * depends only on the three predicates
 *
 *       t<=n+1,  t<=k-2,  t==n+k-1.
 *
 * Head applies t=1..k-2.  In the bulk these predicates are respectively
 * true,false,false, so one fixed T applies from t=k-1 through n+1, exactly
 * n-k+3 times.  Tail layer j is evaluated symbolically at t=n+1+j; hence it
 * opens C_j, and j=k-2 is exactly the final layer.  Reverse closure from
 * states accepted through this fixed tail is precisely co-reachability, so
 * deleting its complement cannot change (1).
 *
 * Run
 *
 *       ruby 179962_06_machine.rb --self-check
 *
 * for an implementation check independent of the aggregated transition
 * formulas.  It keeps labelled vertices and literal edge sets, enumerates
 * every legal 0/1/2-edge insertion, reconstructs the mathematical state from
 * each forest, and compares the COMPLETE state histogram after every layer.
 * It then compares the accepted count with direct permutation enumeration.
 * The checked cases (k,n)=(3,2),(3,3),(4,2),(4,3) exercise capsule creation,
 * bulk, capsule opening, representative multiplicity cases, cycle rejection and final
 * (F,F) acceptance.  This is a regression check; the proof above establishes
 * correctness for all fixed k.
 *
 * BUILD AND USE
 * =============
 *   ruby 179962_06_machine.rb 9
 *   ruby 179962_06_machine.rb --self-check
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     179962_06.c -lgmp -o 179962_06
 *   ./179962_06 --k 9 --upto 30
 *   ./179962_06 --k 9 --term 30
 *   ./179962_06 --k 9 --certify
 *
 * The machine is generated once.  Range output is written atomically to
 * b179962_06_kK.txt and stops before the first value having 1001 digits;
 * --term is unrestricted.  For k=9 the supplied OEIS prefix n=0..5 is
 * embedded; the machine directly supplies every term from n=6 onward.
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
#include <gmp.h>

#define P6_MAX_K 12
#define P6_MAX_N 1000000L
#define P6_MAX_DIGITS 1000UL

typedef struct {
    int rows, cols;
    size_t nnz;
    size_t *start;
    int *source;
    unsigned long *multiple;
} P6Layer;

typedef struct {
    int k, states, head_count, tail_count, accept_count, max_dimension;
    int *head_index, *accept;
    unsigned long *head_coefficient;
    P6Layer bulk, *tail;
} P6Machine;

typedef enum { P6_UPTO, P6_TERM, P6_CERTIFY } P6Mode;

typedef struct {
    int k;
    long n;
    P6Mode mode;
    const char *machine_path;
    bool quiet;
} P6Options;

static _Noreturn void p6_die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static void *p6_malloc(size_t count, size_t size)
{
    if (size && count > SIZE_MAX / size) p6_die("allocation-size overflow");
    void *result = malloc(count * size);
    if (!result && count) p6_die("out of memory");
    return result;
}

static void *p6_calloc(size_t count, size_t size)
{
    if (size && count > SIZE_MAX / size) p6_die("allocation-size overflow");
    void *result = calloc(count, size);
    if (!result && count) p6_die("out of memory");
    return result;
}

static double p6_now(void)
{
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) p6_die("clock_gettime failed");
    return (double)value.tv_sec + (double)value.tv_nsec / 1.0e9;
}

static void p6_expect(FILE *stream, const char *wanted)
{
    char token[32];
    if (fscanf(stream, "%31s", token) != 1 || strcmp(token, wanted) != 0) {
        fprintf(stderr, "error: expected '%s' in machine file\n", wanted);
        exit(EXIT_FAILURE);
    }
}

static void p6_build_layer(P6Layer *layer, int rows, int cols, size_t nnz,
                           const int *from, const int *to,
                           const unsigned long *multiple)
{
    if (rows < 0 || cols < 0) p6_die("negative machine dimension");
    layer->rows = rows; layer->cols = cols; layer->nnz = nnz;
    layer->start = p6_calloc((size_t)cols + 1U, sizeof(*layer->start));
    layer->source = p6_malloc(nnz, sizeof(*layer->source));
    layer->multiple = p6_malloc(nnz, sizeof(*layer->multiple));
    for (size_t edge = 0; edge < nnz; ++edge) {
        if (from[edge] < 0 || from[edge] >= rows || to[edge] < 0 || to[edge] >= cols)
            p6_die("machine edge index out of range");
        ++layer->start[(size_t)to[edge] + 1U];
    }
    for (int column = 0; column < cols; ++column)
        layer->start[column + 1] += layer->start[column];
    size_t *fill = p6_malloc((size_t)cols, sizeof(*fill));
    memcpy(fill, layer->start, (size_t)cols * sizeof(*fill));
    for (size_t edge = 0; edge < nnz; ++edge) {
        size_t slot = fill[to[edge]]++;
        layer->source[slot] = from[edge];
        layer->multiple[slot] = multiple[edge];
    }
    free(fill);
}

static void p6_read_layer(FILE *stream, P6Layer *layer,
                          int rows, int cols, size_t nnz)
{
    int *from = p6_malloc(nnz, sizeof(*from));
    int *to = p6_malloc(nnz, sizeof(*to));
    unsigned long *multiple = p6_malloc(nnz, sizeof(*multiple));
    for (size_t edge = 0; edge < nnz; ++edge) {
        if (fscanf(stream, "%d %d %lu", &from[edge], &to[edge], &multiple[edge]) != 3)
            p6_die("truncated machine edge table");
    }
    p6_build_layer(layer, rows, cols, nnz, from, to, multiple);
    free(multiple); free(to); free(from);
}

static void p6_machine_read(P6Machine *machine, const char *path, int wanted_k)
{
    memset(machine, 0, sizeof(*machine));
    FILE *stream = fopen(path, "r");
    if (!stream) {
        fprintf(stderr, "error: cannot open %s; run ruby 179962_06_machine.rb %d\n",
                path, wanted_k);
        exit(EXIT_FAILURE);
    }
    p6_expect(stream, "k");
    if (fscanf(stream, "%d", &machine->k) != 1 || machine->k != wanted_k)
        p6_die("machine K does not match --k");
    p6_expect(stream, "head");
    if (fscanf(stream, "%d", &machine->head_count) != 1 || machine->head_count < 0)
        p6_die("bad machine head");
    machine->head_index = p6_malloc((size_t)machine->head_count,
                                    sizeof(*machine->head_index));
    machine->head_coefficient = p6_malloc((size_t)machine->head_count,
                                          sizeof(*machine->head_coefficient));
    for (int i = 0; i < machine->head_count; ++i)
        if (fscanf(stream, "%d %lu", &machine->head_index[i],
                   &machine->head_coefficient[i]) != 2) p6_die("bad machine head row");

    p6_expect(stream, "bulk");
    size_t bulk_nnz;
    if (fscanf(stream, "%d %zu", &machine->states, &bulk_nnz) != 2 || machine->states <= 0)
        p6_die("bad bulk header");
    p6_read_layer(stream, &machine->bulk, machine->states, machine->states, bulk_nnz);
    machine->max_dimension = machine->states;

    p6_expect(stream, "tail");
    if (fscanf(stream, "%d", &machine->tail_count) != 1 ||
        machine->tail_count != machine->k - 2) p6_die("bad tail count");
    machine->tail = p6_calloc((size_t)machine->tail_count, sizeof(*machine->tail));
    for (int layer = 0; layer < machine->tail_count; ++layer) {
        int rows, cols; size_t nnz;
        p6_expect(stream, "layer");
        if (fscanf(stream, "%d %d %zu", &rows, &cols, &nnz) != 3)
            p6_die("bad tail layer header");
        p6_read_layer(stream, &machine->tail[layer], rows, cols, nnz);
        if (cols > machine->max_dimension) machine->max_dimension = cols;
    }
    p6_expect(stream, "accept");
    if (fscanf(stream, "%d", &machine->accept_count) != 1 || machine->accept_count < 0)
        p6_die("bad accept header");
    machine->accept = p6_malloc((size_t)machine->accept_count, sizeof(*machine->accept));
    int final_dimension = machine->tail[machine->tail_count - 1].cols;
    for (int i = 0; i < machine->accept_count; ++i) {
        if (fscanf(stream, "%d", &machine->accept[i]) != 1 ||
            machine->accept[i] < 0 || machine->accept[i] >= final_dimension)
            p6_die("bad accepting state");
    }
    if (fclose(stream) != 0) p6_die("could not close machine file");
}

static void p6_layer_clear(P6Layer *layer)
{
    free(layer->multiple); free(layer->source); free(layer->start);
}

static void p6_machine_clear(P6Machine *machine)
{
    for (int i = 0; i < machine->tail_count; ++i) p6_layer_clear(&machine->tail[i]);
    free(machine->tail); p6_layer_clear(&machine->bulk);
    free(machine->accept); free(machine->head_coefficient); free(machine->head_index);
}

static mpz_t *p6_vector_new(int size)
{
    mpz_t *vector = p6_malloc((size_t)size, sizeof(*vector));
    for (int i = 0; i < size; ++i) mpz_init(vector[i]);
    return vector;
}

static void p6_vector_free(mpz_t *vector, int size)
{
    for (int i = 0; i < size; ++i) mpz_clear(vector[i]);
    free(vector);
}

static void p6_apply(const P6Layer *layer, mpz_t *source, mpz_t *target)
{
    for (int column = 0; column < layer->cols; ++column) {
        mpz_set_ui(target[column], 0UL);
        for (size_t edge = layer->start[column]; edge < layer->start[column + 1]; ++edge)
            mpz_addmul_ui(target[column], source[layer->source[edge]],
                          layer->multiple[edge]);
    }
}

typedef struct {
    P6Machine *machine;
    mpz_t *state, *scratch, *tail_a, *tail_b;
    long n;
} P6Run;

static void p6_run_init(P6Run *run, P6Machine *machine)
{
    run->machine = machine;
    run->state = p6_vector_new(machine->states);
    run->scratch = p6_vector_new(machine->states);
    run->tail_a = p6_vector_new(machine->max_dimension);
    run->tail_b = p6_vector_new(machine->max_dimension);
    for (int i = 0; i < machine->head_count; ++i) {
        if (machine->head_index[i] < 0 || machine->head_index[i] >= machine->states)
            p6_die("head state index out of range");
        mpz_set_ui(run->state[machine->head_index[i]], machine->head_coefficient[i]);
    }
    run->n = machine->k - 3;
}

static void p6_run_clear(P6Run *run)
{
    p6_vector_free(run->tail_b, run->machine->max_dimension);
    p6_vector_free(run->tail_a, run->machine->max_dimension);
    p6_vector_free(run->scratch, run->machine->states);
    p6_vector_free(run->state, run->machine->states);
}

static void p6_value(P6Run *run, mpz_t value)
{
    mpz_t *source = run->state, *a = run->tail_a, *b = run->tail_b;
    for (int i = 0; i < run->machine->tail_count; ++i) {
        p6_apply(&run->machine->tail[i], source, a);
        source = a;
        mpz_t *swap = a; a = b; b = swap;
    }
    mpz_set_ui(value, 0UL);
    for (int i = 0; i < run->machine->accept_count; ++i)
        mpz_add(value, value, source[run->machine->accept[i]]);
    mpz_mul_2exp(value, value, 1U);
}

static void p6_advance(P6Run *run)
{
    p6_apply(&run->machine->bulk, run->state, run->scratch);
    mpz_t *swap = run->state; run->state = run->scratch; run->scratch = swap;
    ++run->n;
}

static long p6_number(const char *text, const char *label, long maximum)
{
    char *end = NULL; errno = 0;
    long value = strtol(text, &end, 10);
    if (errno || end == text || *end || value < 0 || value > maximum) {
        fprintf(stderr, "error: %s must be in 0..%ld: %s\n", label, maximum, text);
        exit(EXIT_FAILURE);
    }
    return value;
}

static void p6_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s --k K [--upto N | --term N | --certify] [--machine FILE] [--quiet]\n",
            program);
}

static P6Options p6_options(int argc, char **argv, char *default_path, size_t path_size)
{
    P6Options result = {9, 30, P6_UPTO, NULL, false};
    bool mode_seen = false;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--k") && i + 1 < argc) {
            result.k = (int)p6_number(argv[++i], "K", P6_MAX_K);
            if (result.k < 3) p6_die("K must be in 3..12");
        } else if ((!strcmp(argv[i], "--upto") || !strcmp(argv[i], "--term")) && i + 1 < argc) {
            if (mode_seen) p6_die("specify only one mode");
            result.mode = !strcmp(argv[i], "--term") ? P6_TERM : P6_UPTO;
            result.n = p6_number(argv[++i], "N", P6_MAX_N);
            mode_seen = true;
        } else if (!strcmp(argv[i], "--certify")) {
            if (mode_seen) p6_die("specify only one mode");
            result.mode = P6_CERTIFY; mode_seen = true;
        } else if (!strcmp(argv[i], "--machine") && i + 1 < argc) {
            result.machine_path = argv[++i];
        } else if (!strcmp(argv[i], "--quiet")) {
            result.quiet = true;
        } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            p6_usage(argv[0]); exit(EXIT_SUCCESS);
        } else {
            p6_usage(argv[0]); exit(EXIT_FAILURE);
        }
    }
    if (!result.machine_path) {
        int used = snprintf(default_path, path_size, "179962_06_k%d.machine", result.k);
        if (used < 0 || (size_t)used >= path_size) p6_die("machine path too long");
        result.machine_path = default_path;
    }
    return result;
}

static const char *const p6_k9_prefix[] = {
    "362880", "5296790", "88422296", "1634227958", "32096768008", "649347224736"
};

static void p6_output(FILE *stream, long n, const mpz_t value)
{
    if (fprintf(stream, "%ld ", n) < 0 || !mpz_out_str(stream, 10, value) ||
        fputc('\n', stream) == EOF) p6_die("output failed");
}

/* Coefficients c[0..degree], low to high, of prod(X-root). */
static int p6_polynomial(int k, long *c)
{
    memset(c, 0, (P6_MAX_K + 1U) * sizeof(*c)); c[0] = 1;
    int degree = 0;
    for (int r = 1; r <= k / 2; ++r) {
        long root = (long)r * (k - r);
        for (int j = degree + 1; j >= 1; --j) {
            c[j] = c[j - 1] - root * c[j];
        }
        c[0] *= -root; ++degree;
    }
    if (k & 1) {
        long root = (long)(k / 2) * (k - k / 2);
        for (int j = degree + 1; j >= 1; --j) c[j] = c[j - 1] - root * c[j];
        c[0] *= -root; ++degree;
    }
    return degree;
}

static int p6_certify(P6Run *run, bool quiet)
{
    long polynomial[P6_MAX_K + 1];
    int degree = p6_polynomial(run->machine->k, polynomial);
    long needed = (long)run->machine->states + 1;
    long limit = run->n + needed + degree + 100;
    mpz_t *window = p6_vector_new(degree);
    mpz_t value, residual, product;
    mpz_inits(value, residual, product, NULL);
    long streak = 0, first = -1;
    double started = p6_now();
    for (long n = run->n; n <= limit; ++n) {
        p6_value(run, value);
        if (n >= run->machine->k + degree) {
            mpz_set(residual, value);
            for (int lag = 1; lag <= degree; ++lag) {
                mpz_mul_si(product, window[(n - lag) % degree],
                           polynomial[degree - lag]);
                mpz_add(residual, residual, product);
            }
            if (mpz_sgn(residual) == 0) {
                if (!streak) first = n;
                ++streak;
            } else { streak = 0; first = -1; }
        }
        mpz_set(window[n % degree], value);
        if (!quiet && n > 0 && n % 200 == 0)
            fprintf(stderr, "179962_06: certify n=%ld, zero streak=%ld, %.1f s\n",
                    n, streak, p6_now() - started);
        if (streak >= needed) {
            printf("certified: C_%d recurrence holds for every n >= %ld "
                   "(%ld zero residuals; live dimension %d)\n",
                   run->machine->k, first, streak, run->machine->states);
            mpz_clears(product, residual, value, NULL); p6_vector_free(window, degree);
            return EXIT_SUCCESS;
        }
        if (n < limit) p6_advance(run);
    }
    printf("not certified: longest final streak %ld; need %ld\n", streak, needed);
    mpz_clears(product, residual, value, NULL); p6_vector_free(window, degree);
    return EXIT_FAILURE;
}

int main(int argc, char **argv)
{
    char default_path[128];
    P6Options options = p6_options(argc, argv, default_path, sizeof(default_path));
    P6Machine machine; p6_machine_read(&machine, options.machine_path, options.k);
    if (!options.quiet)
        fprintf(stderr, "179962_06: k=%d, live states=%d, bulk nnz=%zu\n",
                machine.k, machine.states, machine.bulk.nnz);
    P6Run run; p6_run_init(&run, &machine);
    if (options.mode == P6_CERTIFY) {
        int status = p6_certify(&run, options.quiet);
        p6_run_clear(&run); p6_machine_clear(&machine); return status;
    }

    if (options.n < run.n && !(options.k == 9 && options.n <= 5))
        p6_die("requested N precedes this machine's exact head boundary");
    FILE *stream = NULL;
    char partial[128], final[128];
    if (options.mode == P6_UPTO) {
        snprintf(partial, sizeof(partial), "b179962_06_k%d_part.txt", options.k);
        snprintf(final, sizeof(final), "b179962_06_k%d.txt", options.k);
        stream = fopen(partial, "w"); if (!stream) p6_die("cannot create partial b-file");
    }
    mpz_t value, cutoff; mpz_inits(value, cutoff, NULL);
    mpz_ui_pow_ui(cutoff, 10UL, P6_MAX_DIGITS);
    long first_written = -1, last_written = -1;
    double started = p6_now();

    if (options.mode == P6_UPTO && options.k == 9) {
        long last = options.n < 5 ? options.n : 5;
        for (long n = 0; n <= last; ++n) {
            mpz_set_str(value, p6_k9_prefix[n], 10); p6_output(stream, n, value);
            if (first_written < 0) first_written = n;
            last_written = n;
        }
    } else if (options.mode == P6_TERM && options.k == 9 && options.n <= 5) {
        mpz_set_str(value, p6_k9_prefix[options.n], 10); p6_output(stdout, options.n, value);
    }

    if (options.n >= run.n) {
        for (long n = run.n; n <= options.n; ++n) {
            p6_value(&run, value);
            if (options.mode == P6_TERM && n == options.n) p6_output(stdout, n, value);
            if (options.mode == P6_UPTO) {
                if (mpz_cmpabs(value, cutoff) >= 0) break;
                p6_output(stream, n, value);
                if (first_written < 0) first_written = n;
                last_written = n;
                if (fflush(stream) != 0) p6_die("could not flush partial b-file");
            }
            if (n < options.n) p6_advance(&run);
        }
    }
    if (stream) {
        if (fclose(stream) != 0) p6_die("could not close partial b-file");
        if (rename(partial, final) != 0) p6_die("could not finalize b-file");
        printf("wrote %s (n=%ld..%ld)\n", final, first_written, last_written);
    }
    if (!options.quiet) fprintf(stderr, "179962_06: %.3f s\n", p6_now() - started);
    mpz_clears(cutoff, value, NULL); p6_run_clear(&run); p6_machine_clear(&machine);
    return EXIT_SUCCESS;
}
