/*
 * Threshold-k minimum-jump permutations -- explicit sparse transfer matrix.
 *
 * This is the matrix form of the linear-forest inclusion-exclusion method.
 * It deliberately differs from 179957_04.c in when transitions are built:
 *
 *   1. enumerate every reachable saturated frontier state;
 *   2. build the complete sparse transfer matrix M_k once;
 *   3. apply that immutable matrix to polynomial state vectors.
 *
 * The mathematical invariant and exact GMP evaluation are shared with 04.
 * The source includes 04 with its main renamed so that the carefully checked
 * state canonicalization and transition rules have a single definition; the
 * range driver below does not call 04's compute_sequence.
 * In the standalone 03 executable, an additional transposed copy of the
 * sparse matrix lets worker threads own disjoint destination-state ranges.
 * Every GMP output coefficient therefore has exactly one writer; source
 * coefficients are read-only until all workers have joined.  --threads T
 * selects 1..64 workers, while the default uses the online CPU count (capped
 * at 16, and one worker for small matrices).  When this file is embedded by
 * 05 for its proof certificate, the original single-thread source-row kernel
 * is retained, so 05 does not acquire a pthread build dependency.
 *
 * For fixed k, the matrix has S(k) states.  Its entries are small monomials
 * c*x^e (e=0,1,2).  Computing all answers through N takes
 * O(E(k)*N^2) exact polynomial operations and O(S(k)*N) GMP objects.
 * As in 04, k is accepted in 1..10 but large k can hit an explicit state or
 * coefficient-object guard before a requested large n is reached.  The
 * matrix-specific guard permits 10000000 coefficient objects per vector,
 * enough for k=7 through n=500 (19248*501 = 9643248 objects); the two rolling
 * vectors then occupy about 294 MiB in GMP headers before integer limbs.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     179957_03.c -lgmp -o 179957_03
 *
 * Usage:
 *   ./179957_03 --k 4 --upto 100
 *   ./179957_03 --k 4 --term 100
 *   ./179957_03 --k 7 --upto 500 --threads 8
 *   ./179957_03 --k 4 --check
 */

#if defined(__APPLE__) && !defined(A179957_03_NO_MAIN)
#define _DARWIN_C_SOURCE
#endif

#define main a179957_04_embedded_main
#include "179957_04.c"
#undef main

#ifndef A179957_03_NO_MAIN
#include <pthread.h>
#endif

#define M3_MAX_MATRIX_EDGES 5000000U
#define M3_MAX_VECTOR_COEFFICIENT_SLOTS 10000000U
#define M3_MAX_THREADS 64
#define M3_AUTO_THREAD_CAP 16
#define M3_PARALLEL_STATE_THRESHOLD 1024U

typedef struct {
    size_t destination;
    uint8_t edges;
    unsigned long weight;
} M3Edge;

typedef struct {
    M3Edge *edge;
    size_t count;
} M3Row;

#ifndef A179957_03_NO_MAIN
typedef struct {
    size_t source;
    uint8_t edges;
    unsigned long weight;
} M3IncomingEdge;

typedef struct {
    M3IncomingEdge *edge;
    size_t count;
} M3IncomingRow;
#endif

typedef struct {
    PolynomialMap registry; /* coefficient_count=1; used as an exact key hash */
    M3Row *row;
#ifndef A179957_03_NO_MAIN
    M3IncomingRow *incoming;
#endif
    unsigned long *orientation;
    size_t row_capacity;
    size_t edge_count;
    int frontier_width;
} M3Matrix;

typedef struct {
    size_t matrix_states;
    size_t matrix_edges;
    size_t peak_vector_coefficient_slots;
    uint64_t matrix_applications;
    uint64_t polynomial_addmuls;
    size_t worker_threads;
    double seconds;
} M3Stats;

#ifndef A179957_03_NO_MAIN
/* Zero means automatic selection from the online processor count. */
static int m3_requested_threads = 0;
static size_t m3_select_worker_threads(size_t state_count);
#endif

static size_t m3_registry_find(const PolynomialMap *registry, StateKey key)
{
    if (registry->bucket_count == 0U) {
        die("empty matrix state registry");
    }
    size_t slot = state_hash(key) & (registry->bucket_count - 1U);
    while (registry->bucket[slot] != 0U) {
        size_t index = registry->bucket[slot] - 1U;
        if (index >= registry->count) {
            die("corrupt matrix state registry");
        }
        if (state_key_equal(registry->item[index].key, key)) {
            return index;
        }
        slot = (slot + 1U) & (registry->bucket_count - 1U);
    }
    die("matrix destination state was not registered");
}

static void m3_ensure_rows(M3Matrix *matrix)
{
    if (matrix->registry.count <= matrix->row_capacity) {
        return;
    }
    size_t old_capacity = matrix->row_capacity;
    size_t new_capacity = old_capacity == 0U ? 16U : old_capacity * 2U;
    while (new_capacity < matrix->registry.count) {
        if (new_capacity > SIZE_MAX / 2U) {
            die("matrix row capacity overflow");
        }
        new_capacity *= 2U;
    }
    if (new_capacity > MAX_ACTIVE_STATES) {
        new_capacity = MAX_ACTIVE_STATES;
    }
    if (new_capacity < matrix->registry.count) {
        die("matrix state limit reached");
    }
    matrix->row = xreallocarray(matrix->row, new_capacity,
                                sizeof(*matrix->row));
    memset(matrix->row + old_capacity, 0,
           (new_capacity - old_capacity) * sizeof(*matrix->row));
    matrix->row_capacity = new_capacity;
}

static size_t m3_registry_add(M3Matrix *matrix, StateKey key)
{
    size_t old_count = matrix->registry.count;
    PolynomialState *state = polynomial_map_get(&matrix->registry, key);
    size_t index = (size_t)(state - matrix->registry.item);
    if (index >= matrix->registry.count) {
        die("matrix registry returned an invalid index");
    }
    if (matrix->registry.count != old_count &&
        matrix->registry.count != old_count + 1U) {
        die("matrix registry grew by more than one state");
    }
    m3_ensure_rows(matrix);
    return index;
}

static void m3_keyset_step(PolynomialMap *current, PolynomialMap *next,
                           int frontier_size, int frontier_width)
{
    for (size_t index = 0U; index < current->count; ++index) {
        Transition transition[MAX_RAW_TRANSITIONS];
        size_t count = build_transitions(current->item[index].key,
                                         frontier_size, frontier_width,
                                         transition);
        for (size_t t = 0U; t < count; ++t) {
            (void)polynomial_map_get(next, transition[t].key);
        }
    }
}

#ifndef A179957_03_NO_MAIN
static void m3_matrix_build_incoming(M3Matrix *matrix)
{
    size_t state_count = matrix->registry.count;
    matrix->incoming = xcalloc(state_count, sizeof(*matrix->incoming));

    for (size_t source = 0U; source < state_count; ++source) {
        const M3Row *row = &matrix->row[source];
        for (size_t e = 0U; e < row->count; ++e) {
            size_t destination = row->edge[e].destination;
            if (destination >= state_count) {
                die("matrix transpose destination is out of range");
            }
            matrix->incoming[destination].count = checked_add_size(
                matrix->incoming[destination].count, 1U);
        }
    }

    size_t incoming_edges = 0U;
    for (size_t destination = 0U; destination < state_count; ++destination) {
        M3IncomingRow *row = &matrix->incoming[destination];
        incoming_edges = checked_add_size(incoming_edges, row->count);
        row->edge = xcalloc(row->count, sizeof(*row->edge));
    }
    if (incoming_edges != matrix->edge_count) {
        die("matrix transpose edge count mismatch");
    }

    size_t *used = xcalloc(state_count, sizeof(*used));
    for (size_t source = 0U; source < state_count; ++source) {
        const M3Row *outgoing = &matrix->row[source];
        for (size_t e = 0U; e < outgoing->count; ++e) {
            const M3Edge *edge = &outgoing->edge[e];
            M3IncomingRow *incoming = &matrix->incoming[edge->destination];
            size_t position = used[edge->destination]++;
            if (position >= incoming->count) {
                free(used);
                die("matrix transpose row overflow");
            }
            incoming->edge[position].source = source;
            incoming->edge[position].edges = edge->edges;
            incoming->edge[position].weight = edge->weight;
        }
    }
    for (size_t destination = 0U; destination < state_count; ++destination) {
        if (used[destination] != matrix->incoming[destination].count) {
            free(used);
            die("matrix transpose row count mismatch");
        }
    }
    free(used);
}
#endif

static void m3_matrix_build(int k, M3Matrix *matrix)
{
    memset(matrix, 0, sizeof(*matrix));
    matrix->frontier_width = k - 1;

    PolynomialMap keysets[2];
    polynomial_map_init(&keysets[0], 1U);
    polynomial_map_init(&keysets[1], 1U);
    StateKey empty = {UINT64_C(0), UINT64_C(0)};
    (void)polynomial_map_get(&keysets[0], empty);
    int current_index = 0;
    for (int size = 0; size < matrix->frontier_width; ++size) {
        int next_index = 1 - current_index;
        m3_keyset_step(&keysets[current_index], &keysets[next_index],
                       size, matrix->frontier_width);
        polynomial_map_destroy(&keysets[current_index]);
        polynomial_map_init(&keysets[current_index], 1U);
        current_index = next_index;
    }

    polynomial_map_init(&matrix->registry, 1U);
    PolynomialMap *seeds = &keysets[current_index];
    for (size_t index = 0U; index < seeds->count; ++index) {
        (void)m3_registry_add(matrix, seeds->item[index].key);
    }
    polynomial_map_destroy(&keysets[0]);
    polynomial_map_destroy(&keysets[1]);

    /* Breadth-first closure under the saturated transition relation. */
    for (size_t source = 0U; source < matrix->registry.count; ++source) {
        StateKey key = matrix->registry.item[source].key;
        Transition transition[MAX_RAW_TRANSITIONS];
        size_t count = build_transitions(key, matrix->frontier_width,
                                         matrix->frontier_width,
                                         transition);
        matrix->row[source].edge = xcalloc(count,
                                            sizeof(*matrix->row[source].edge));
        matrix->row[source].count = count;
        matrix->edge_count = checked_add_size(matrix->edge_count, count);
        if (matrix->edge_count > M3_MAX_MATRIX_EDGES) {
            die("sparse transfer-matrix edge limit reached");
        }
        for (size_t t = 0U; t < count; ++t) {
            size_t destination = m3_registry_add(matrix, transition[t].key);
            matrix->row[source].edge[t].destination = destination;
            matrix->row[source].edge[t].edges = transition[t].edges;
            matrix->row[source].edge[t].weight = transition[t].weight;
        }
    }
    matrix->orientation = xcalloc(matrix->registry.count,
                                  sizeof(*matrix->orientation));
    for (size_t state = 0U; state < matrix->registry.count; ++state) {
        matrix->orientation[state] = final_orientation_weight(
            matrix->registry.item[state].key, matrix->frontier_width);
    }
#ifndef A179957_03_NO_MAIN
    m3_matrix_build_incoming(matrix);
#endif
}

static void m3_matrix_destroy(M3Matrix *matrix)
{
    for (size_t index = 0U; index < matrix->registry.count; ++index) {
        free(matrix->row[index].edge);
#ifndef A179957_03_NO_MAIN
        free(matrix->incoming[index].edge);
#endif
    }
    free(matrix->row);
    free(matrix->orientation);
#ifndef A179957_03_NO_MAIN
    free(matrix->incoming);
#endif
    polynomial_map_destroy(&matrix->registry);
    memset(matrix, 0, sizeof(*matrix));
}

static void m3_evaluate_q(const mpz_t *q, int n,
                          const mpz_t *factorial, mpz_t answer)
{
    int maximum_edges = n == 0 ? 0 : n - 1;
    mpz_set_ui(answer, 0UL);
    for (int edges = 0; edges <= maximum_edges; ++edges) {
        if ((edges & 1) == 0) {
            mpz_addmul(answer, q[edges], factorial[n - edges]);
        } else {
            mpz_submul(answer, q[edges], factorial[n - edges]);
        }
    }
    if (mpz_sgn(answer) < 0 || (n > 1 && mpz_odd_p(answer))) {
        die("matrix evaluation invariant failed");
    }
}

static void m3_finalize_map(const PolynomialMap *map, int n,
                            int frontier_size, mpz_t *q,
                            const mpz_t *factorial, mpz_t answer,
                            M3Stats *stats)
{
    int maximum_edges = n == 0 ? 0 : n - 1;
    for (int edges = 0; edges <= maximum_edges; ++edges) {
        mpz_set_ui(q[edges], 0UL);
    }
    for (size_t state_index = 0U; state_index < map->count; ++state_index) {
        unsigned long orientation =
            final_orientation_weight(map->item[state_index].key,
                                     frontier_size);
        for (int edges = 0; edges <= maximum_edges; ++edges) {
            if (mpz_sgn(map->item[state_index].coefficient[edges]) != 0) {
                mpz_addmul_ui(q[edges],
                              map->item[state_index].coefficient[edges],
                              orientation);
                increment_u64(&stats->polynomial_addmuls,
                              "matrix polynomial operation");
            }
        }
    }
    m3_evaluate_q((const mpz_t *)q, n, factorial, answer);
}

static void m3_polynomial_map_step(const PolynomialMap *current,
                                   PolynomialMap *next, int n,
                                   int frontier_size, int frontier_width,
                                   M3Stats *stats)
{
    int maximum_edges = n == 0 ? 0 : n - 1;
    for (size_t state_index = 0U;
         state_index < current->count; ++state_index) {
        const PolynomialState *source = &current->item[state_index];
        Transition transition[MAX_RAW_TRANSITIONS];
        size_t count = build_transitions(source->key, frontier_size,
                                         frontier_width, transition);
        for (size_t t = 0U; t < count; ++t) {
            PolynomialState *destination =
                polynomial_map_get(next, transition[t].key);
            for (int edges = 0; edges <= maximum_edges; ++edges) {
                if (mpz_sgn(source->coefficient[edges]) == 0) {
                    continue;
                }
                int new_edges = edges + transition[t].edges;
                if (new_edges > n) {
                    die("warm-up polynomial degree exceeds n");
                }
                mpz_addmul_ui(destination->coefficient[new_edges],
                              source->coefficient[edges],
                              transition[t].weight);
                increment_u64(&stats->polynomial_addmuls,
                              "matrix warm-up polynomial operation");
            }
        }
    }
}

static void m3_finalize_vector(const M3Matrix *matrix,
                               const mpz_t *vector, int n,
                               size_t coefficient_count, mpz_t *q,
                               const mpz_t *factorial, mpz_t answer,
                               M3Stats *stats)
{
    int maximum_edges = n == 0 ? 0 : n - 1;
    for (int edges = 0; edges <= maximum_edges; ++edges) {
        mpz_set_ui(q[edges], 0UL);
    }
    for (size_t state = 0U; state < matrix->registry.count; ++state) {
        unsigned long orientation = matrix->orientation[state];
        size_t base = checked_product_size(state, coefficient_count);
        for (int edges = 0; edges <= maximum_edges; ++edges) {
            if (mpz_sgn(vector[base + (size_t)edges]) != 0) {
                mpz_addmul_ui(q[edges], vector[base + (size_t)edges],
                              orientation);
                increment_u64(&stats->polynomial_addmuls,
                              "matrix polynomial operation");
            }
        }
    }
    m3_evaluate_q((const mpz_t *)q, n, factorial, answer);
}

#ifdef A179957_03_NO_MAIN
static void m3_apply_matrix(const M3Matrix *matrix,
                            const mpz_t *current, mpz_t *next,
                            int n, size_t coefficient_count,
                            M3Stats *stats)
{
    for (size_t state = 0U; state < matrix->registry.count; ++state) {
        size_t base = checked_product_size(state, coefficient_count);
        for (int edges = 0; edges <= n; ++edges) {
            mpz_set_ui(next[base + (size_t)edges], 0UL);
        }
    }
    int maximum_edges = n == 0 ? 0 : n - 1;
    for (size_t source = 0U; source < matrix->registry.count; ++source) {
        size_t source_base = checked_product_size(source, coefficient_count);
        const M3Row *row = &matrix->row[source];
        for (size_t e = 0U; e < row->count; ++e) {
            size_t destination_base = checked_product_size(
                row->edge[e].destination, coefficient_count);
            for (int edges = 0; edges <= maximum_edges; ++edges) {
                if (mpz_sgn(current[source_base + (size_t)edges]) == 0) {
                    continue;
                }
                int new_edges = edges + row->edge[e].edges;
                if (new_edges > n) {
                    die("matrix polynomial degree exceeds n");
                }
                mpz_addmul_ui(next[destination_base + (size_t)new_edges],
                              current[source_base + (size_t)edges],
                              row->edge[e].weight);
                increment_u64(&stats->polynomial_addmuls,
                              "matrix polynomial operation");
            }
        }
    }
    increment_u64(&stats->matrix_applications, "matrix application");
}
#else
typedef struct {
    const M3Matrix *matrix;
    const mpz_t *current;
    mpz_t *next;
    int n;
    size_t coefficient_count;
    size_t first_destination;
    size_t past_last_destination;
    uint64_t addmuls;
    bool degree_error;
} M3WorkerTask;

static size_t m3_select_worker_threads(size_t state_count)
{
    size_t threads;
    if (m3_requested_threads > 0) {
        threads = (size_t)m3_requested_threads;
    } else if (state_count < M3_PARALLEL_STATE_THRESHOLD) {
        threads = 1U;
    } else {
        long online = sysconf(_SC_NPROCESSORS_ONLN);
        threads = online > 0 ? (size_t)online : 1U;
        if (threads > M3_AUTO_THREAD_CAP) {
            threads = M3_AUTO_THREAD_CAP;
        }
    }
    if (threads > state_count) {
        threads = state_count;
    }
    return threads == 0U ? 1U : threads;
}

static void *m3_matrix_worker(void *argument)
{
    M3WorkerTask *task = argument;
    int maximum_edges = task->n == 0 ? 0 : task->n - 1;
    for (size_t destination = task->first_destination;
         destination < task->past_last_destination; ++destination) {
        size_t destination_base = checked_product_size(
            destination, task->coefficient_count);
        for (int edges = 0; edges <= task->n; ++edges) {
            mpz_set_ui(task->next[destination_base + (size_t)edges], 0UL);
        }
        const M3IncomingRow *row = &task->matrix->incoming[destination];
        for (size_t e = 0U; e < row->count; ++e) {
            const M3IncomingEdge *incoming = &row->edge[e];
            size_t source_base = checked_product_size(
                incoming->source, task->coefficient_count);
            for (int edges = 0; edges <= maximum_edges; ++edges) {
                if (mpz_sgn(task->current[source_base + (size_t)edges]) ==
                    0) {
                    continue;
                }
                int new_edges = edges + incoming->edges;
                if (new_edges > task->n) {
                    task->degree_error = true;
                    continue;
                }
                mpz_addmul_ui(
                    task->next[destination_base + (size_t)new_edges],
                    task->current[source_base + (size_t)edges],
                    incoming->weight);
                if (task->addmuls == UINT64_MAX) {
                    task->degree_error = true;
                } else {
                    ++task->addmuls;
                }
            }
        }
    }
    return NULL;
}

static void m3_apply_matrix(const M3Matrix *matrix,
                            const mpz_t *current, mpz_t *next,
                            int n, size_t coefficient_count,
                            M3Stats *stats)
{
    size_t thread_count = m3_select_worker_threads(matrix->registry.count);
    if (thread_count > stats->worker_threads) {
        stats->worker_threads = thread_count;
    }
    pthread_t thread[M3_MAX_THREADS];
    M3WorkerTask task[M3_MAX_THREADS];
    memset(thread, 0, sizeof(thread));
    memset(task, 0, sizeof(task));

    size_t state_count = matrix->registry.count;
    size_t total_work = checked_add_size(matrix->edge_count, state_count);
    size_t boundary = 0U;
    size_t completed_work = 0U;
    for (size_t t = 0U; t < thread_count; ++t) {
        task[t].matrix = matrix;
        task[t].current = current;
        task[t].next = next;
        task[t].n = n;
        task[t].coefficient_count = coefficient_count;
        task[t].first_destination = boundary;
        if (t + 1U == thread_count) {
            boundary = state_count;
        } else {
            size_t target = total_work * (t + 1U) / thread_count;
            while (boundary < state_count) {
                size_t row_work = checked_add_size(
                    matrix->incoming[boundary].count, 1U);
                if (completed_work + row_work > target &&
                    boundary > task[t].first_destination) {
                    break;
                }
                completed_work = checked_add_size(completed_work, row_work);
                ++boundary;
            }
        }
        task[t].past_last_destination = boundary;
    }

    size_t created = 0U;
    for (size_t t = 1U; t < thread_count; ++t) {
        int error = pthread_create(&thread[t], NULL, m3_matrix_worker,
                                   &task[t]);
        if (error != 0) {
            for (size_t joined = 1U; joined <= created; ++joined) {
                (void)pthread_join(thread[joined], NULL);
            }
            fprintf(stderr, "error: cannot create matrix worker: %s\n",
                    strerror(error));
            exit(EXIT_FAILURE);
        }
        ++created;
    }
    (void)m3_matrix_worker(&task[0]);
    for (size_t t = 1U; t < thread_count; ++t) {
        int error = pthread_join(thread[t], NULL);
        if (error != 0) {
            fprintf(stderr, "error: cannot join matrix worker: %s\n",
                    strerror(error));
            exit(EXIT_FAILURE);
        }
    }

    for (size_t t = 0U; t < thread_count; ++t) {
        if (task[t].degree_error) {
            die("parallel matrix degree or operation-count invariant failed");
        }
        if (stats->polynomial_addmuls > UINT64_MAX - task[t].addmuls) {
            die("matrix polynomial operation counter overflow");
        }
        stats->polynomial_addmuls += task[t].addmuls;
    }
    increment_u64(&stats->matrix_applications, "matrix application");
}
#endif

static mpz_t *m3_compute_sequence(int maximum_n, int k, FILE *stream,
                                  M3Stats *stats)
{
    memset(stats, 0, sizeof(*stats));
    stats->worker_threads = 1U;
    double started = monotonic_seconds();
    size_t coefficient_count = (size_t)maximum_n + 1U;
    mpz_t *answer = mpz_array_create(coefficient_count);
    if (maximum_n <= 2 * k) {
        for (int n = 0; n <= maximum_n; ++n) {
            mpz_set_ui(answer[n], n <= 1 ? 1UL
                                  : n < 2 * k ? 0UL : 2UL);
            write_complete_term(stream, n, answer[n]);
        }
        stats->seconds = monotonic_seconds() - started;
        return answer;
    }

    M3Matrix matrix;
    m3_matrix_build(k, &matrix);
    stats->matrix_states = matrix.registry.count;
    stats->matrix_edges = matrix.edge_count;

    size_t vector_slots = checked_product_size(matrix.registry.count,
                                                coefficient_count);
    if (vector_slots > M3_MAX_VECTOR_COEFFICIENT_SLOTS) {
        m3_matrix_destroy(&matrix);
        mpz_array_destroy(answer, coefficient_count);
        die("matrix coefficient-object limit (10000000 per vector) reached; "
            "use a smaller k or n");
    }
    stats->peak_vector_coefficient_slots =
        checked_product_size(vector_slots, 2U);
    mpz_t *factorial = mpz_array_create(coefficient_count);
    mpz_t *q = mpz_array_create(coefficient_count);
    mpz_set_ui(factorial[0], 1UL);
    for (int n = 1; n <= maximum_n; ++n) {
        mpz_mul_ui(factorial[n], factorial[n - 1], (unsigned long)n);
    }

    PolynomialMap warm[2];
    polynomial_map_init(&warm[0], coefficient_count);
    polynomial_map_init(&warm[1], coefficient_count);
    StateKey empty = {UINT64_C(0), UINT64_C(0)};
    PolynomialState *initial = polynomial_map_get(&warm[0], empty);
    mpz_set_ui(initial->coefficient[0], 1UL);
    int warm_index = 0;
    for (int n = 0; n < matrix.frontier_width; ++n) {
        m3_finalize_map(&warm[warm_index], n, n, q,
                        (const mpz_t *)factorial, answer[n], stats);
        write_complete_term(stream, n, answer[n]);
        int next_index = 1 - warm_index;
        m3_polynomial_map_step(&warm[warm_index], &warm[next_index], n,
                               n, matrix.frontier_width, stats);
        polynomial_map_destroy(&warm[warm_index]);
        polynomial_map_init(&warm[warm_index], coefficient_count);
        warm_index = next_index;
    }

    mpz_t *vectors[2] = {
        mpz_array_create(vector_slots), mpz_array_create(vector_slots)
    };
    for (size_t state = 0U; state < warm[warm_index].count; ++state) {
        size_t matrix_index = m3_registry_find(
            &matrix.registry, warm[warm_index].item[state].key);
        size_t base = checked_product_size(matrix_index,
                                            coefficient_count);
        int warm_maximum_edges = matrix.frontier_width == 0
                                     ? 0 : matrix.frontier_width - 1;
        for (int edges = 0; edges <= warm_maximum_edges; ++edges) {
            mpz_set(vectors[0][base + (size_t)edges],
                    warm[warm_index].item[state].coefficient[edges]);
        }
    }
    polynomial_map_destroy(&warm[0]);
    polynomial_map_destroy(&warm[1]);

    int vector_index = 0;
    for (int n = matrix.frontier_width; n <= maximum_n; ++n) {
        m3_finalize_vector(&matrix, vectors[vector_index], n,
                           coefficient_count, q,
                           (const mpz_t *)factorial, answer[n], stats);
        write_complete_term(stream, n, answer[n]);
        if (n < maximum_n) {
            int next_index = 1 - vector_index;
            m3_apply_matrix(&matrix, vectors[vector_index],
                            vectors[next_index], n,
                            coefficient_count, stats);
            vector_index = next_index;
        }
    }

    mpz_array_destroy(vectors[0], vector_slots);
    mpz_array_destroy(vectors[1], vector_slots);
    mpz_array_destroy(factorial, coefficient_count);
    mpz_array_destroy(q, coefficient_count);
    m3_matrix_destroy(&matrix);
    stats->seconds = monotonic_seconds() - started;
    return answer;
}

#ifndef A179957_03_NO_MAIN
static void m3_print_stats(int maximum_n, int k, const M3Stats *stats)
{
    fprintf(stderr,
            "179957_03: k=%d, n=0..%d, explicit sparse transfer matrix, "
            "states=%zu, edges=%zu, threads=%zu, matrix applications=%llu, "
            "peak vector coefficient slots=%zu, addmuls=%llu, %.3f s\n",
            k, maximum_n, stats->matrix_states, stats->matrix_edges,
            stats->worker_threads,
            (unsigned long long)stats->matrix_applications,
            stats->peak_vector_coefficient_slots,
            (unsigned long long)stats->polynomial_addmuls,
            stats->seconds);
}

static int m3_check(int maximum_n, int k)
{
    if (maximum_n > KNOWN_MAX_N) {
        fprintf(stderr, "error: CHECK_N must be in 0..%d: %d\n",
                KNOWN_MAX_N, maximum_n);
        return EXIT_FAILURE;
    }
    M3Stats stats;
    mpz_t *answer = m3_compute_sequence(maximum_n, k, NULL, &stats);
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
                        "error: matrix/frontier mismatch at k=%d,n=%d: "
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
            fprintf(stderr, "error: matrix/direct mismatch at k=%d,n=%d\n",
                    k, n);
            mpz_array_destroy(reference, (size_t)maximum_n + 1U);
            mpz_array_destroy(answer, (size_t)maximum_n + 1U);
            return EXIT_FAILURE;
        }
    }
    m3_print_stats(maximum_n, k, &stats);
    printf("ok: k=%d, explicit sparse matrix agrees with 04 through "
           "n=%d and with full permutations through n=%d%s\n",
           k, maximum_n, direct_max,
           k == 4 ? "; A179957 verified" : "");
    mpz_array_destroy(reference, (size_t)maximum_n + 1U);
    mpz_array_destroy(answer, (size_t)maximum_n + 1U);
    return EXIT_SUCCESS;
}

static void m3_produce_file(const char *argv0, int maximum_n, int k)
{
    char final_name[64];
    char part_name[64];
    int final_length = snprintf(final_name, sizeof(final_name),
                                "b179957_03_k%d.txt", k);
    int part_length = snprintf(part_name, sizeof(part_name),
                               "b179957_03_k%d_part.txt", k);
    if (final_length < 0 || part_length < 0 ||
        (size_t)final_length >= sizeof(final_name) ||
        (size_t)part_length >= sizeof(part_name)) {
        die("could not form matrix output filename");
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
    M3Stats stats;
    mpz_t *answer = m3_compute_sequence(maximum_n, k, stream, &stats);
    if (verify_known(answer, maximum_n, k) != EXIT_SUCCESS ||
        fclose(stream) != 0) {
        mpz_array_destroy(answer, (size_t)maximum_n + 1U);
        free(final_path);
        free(part_path);
        die("could not complete matrix output file");
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        mpz_array_destroy(answer, (size_t)maximum_n + 1U);
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }
    m3_print_stats(maximum_n, k, &stats);
    printf("wrote %s (n=0..%d)\n", final_path, maximum_n);
    mpz_array_destroy(answer, (size_t)maximum_n + 1U);
    free(final_path);
    free(part_path);
}

static void m3_usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--k K] [--threads T]\n"
            "       %s --upto MAX_N [--k K] [--threads T]\n"
            "       %s --term N [--k K] [--threads T]\n"
            "       %s --check [MAX_N] [--k K] [--threads T]\n"
            "K is 1..10 (default 4); N is 0..10000; T is 1..64.\n"
            "Without --threads, the online CPU count is used for large "
            "matrices (at most 16 threads).\n"
            "A range run writes b179957_03_kK.txt.\n",
            program, program, program, program);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        m3_usage(argv[0]);
        return EXIT_SUCCESS;
    }
    RunMode mode = RUN_UPTO;
    int n = -1;
    int k = DEFAULT_K;
    bool have_n = false;
    bool have_k = false;
    bool have_threads = false;
    bool have_mode = false;
    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--k") == 0) {
            if (have_k || argument + 1 >= argc) {
                m3_usage(argv[0]);
                return EXIT_FAILURE;
            }
            k = parse_bounded_integer(argv[++argument], "K", 1,
                                      MAX_SUPPORTED_K);
            have_k = true;
        } else if (strcmp(text, "--threads") == 0) {
            if (have_threads || argument + 1 >= argc) {
                m3_usage(argv[0]);
                return EXIT_FAILURE;
            }
            m3_requested_threads = parse_bounded_integer(
                argv[++argument], "T", 1, M3_MAX_THREADS);
            have_threads = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                m3_usage(argv[0]);
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
                m3_usage(argv[0]);
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
            m3_usage(argv[0]);
            return EXIT_FAILURE;
        } else {
            n = parse_bounded_integer(text,
                        mode == RUN_CHECK ? "CHECK_N" : "MAX_N",
                        0, MAX_SUPPORTED_N);
            have_n = true;
        }
    }
    if (!have_n) {
        n = mode == RUN_CHECK
                ? (k == 4 ? DEFAULT_CHECK_N : DEFAULT_GENERAL_CHECK_N)
                : DEFAULT_MAX_N;
    }
    if (mode == RUN_CHECK) {
        return m3_check(n, k);
    }
    if (mode == RUN_TERM) {
        M3Stats stats;
        mpz_t *answer = m3_compute_sequence(n, k, NULL, &stats);
        if (verify_known(answer, n, k) != EXIT_SUCCESS) {
            mpz_array_destroy(answer, (size_t)n + 1U);
            return EXIT_FAILURE;
        }
        m3_print_stats(n, k, &stats);
        gmp_printf("%d %Zd\n", n, answer[n]);
        mpz_array_destroy(answer, (size_t)n + 1U);
        return EXIT_SUCCESS;
    }
    m3_produce_file(argv[0], n, k);
    return EXIT_SUCCESS;
}
#endif
