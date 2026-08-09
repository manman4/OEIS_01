/*
 * 179962_05.c -- inclusion-exclusion + compact frontier DP.
 *
 * A_k(n) is the number of permutations p of [2*n+k] such that
 *
 *                    |p(i+1)-p(i)| > n.
 *
 * This is a C/GMP implementation of the algorithm in perm_gap.rb.  It uses
 * the proved identity
 *
 *   a_d(N) = Sum_F (-1)^e(F) 2^c(F) (N-e(F))!,
 *
 * where F ranges over linear forests of the graph with edges
 * 0 < |u-v| < d.  Here A_k(n)=a_{n+1}(2*n+k).  No recurrence for A_k is
 * used, either to generate or to check a value.
 *
 * Vertices of the forbidden graph are exposed in increasing order.  Its
 * bandwidth is d-1=n, so only the last n vertices form the frontier.  As in
 * perm_gap.rb, a slot stores its degree and, only for degree-one vertices,
 * the label of its path endpoint.  Labels occurring twice are the two open
 * ends of one path; a label occurring once has its other endpoint outside
 * the frontier.  Degree-zero and degree-two vertices carry no component
 * label.  This is smaller than the more redundant state used by
 * 179957_03.c.
 *
 * The DP marks selected edges by x.  Whenever a covered vertex leaves the
 * frontier its weight is multiplied by 2; remaining covered frontier
 * vertices receive the same factor at read-off.  Thus coefficient q_j is
 * exactly 2^v times the number of j-edge forests, and since c=v-j,
 * q_j/2^j is the required 2^c-weight.  The final inclusion-exclusion sum is
 * evaluated with exact GMP integers and checked for exact divisibility.
 *
 * Important complexity distinction: for fixed d this is polynomial in N,
 * but on the diagonal d=n+1 the frontier width grows with n.  Consequently
 * 05 is still exponential in n; it is an independent exact checker, not a
 * large-n replacement for the proved insertion DP used by 02/diag.c.
 * Explicit state, transition and GMP-object guards stop before an unsafe
 * allocation.  The packed state supports n<=15, normally far beyond the
 * practical state-space limit.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     179962_05.c -lgmp -o 179962_05
 *
 * Usage:
 *   ./179962_05 --k 4 --term 8
 *   ./179962_05 --k 4 --upto 8
 *   ./179962_05 --k 4 --check 7
 *
 * Range output is written to b179962_05_kK_part.txt and atomically renamed
 * after completion.  --term writes only to standard output.
 */

#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <gmp.h>

#define P5_DEFAULT_K 4
#define P5_DEFAULT_N 7
#define P5_MAX_N 15
#define P5_MAX_TOTAL 10000
#define P5_MAX_SLOTS 15
#define P5_MAX_BAG (P5_MAX_SLOTS + 1)
#define P5_MAX_RAW (1 + P5_MAX_SLOTS + P5_MAX_SLOTS*(P5_MAX_SLOTS-1)/2)
#define P5_MAX_STATES ((size_t)2000000)
#define P5_MAX_TRANSITIONS ((size_t)100000000)
#define P5_MAX_MPZ_OBJECTS ((size_t)40000000)

typedef struct {
    uint64_t low;
    uint64_t high;
    uint8_t size;
} P5Key;

typedef struct {
    size_t destination;
    uint8_t edges;
    uint8_t factor;
} P5Transition;

typedef struct {
    P5Transition *item;
    size_t count;
} P5Row;

typedef struct {
    P5Key key;
    unsigned long readoff;
} P5State;

typedef struct {
    int width;
    P5State *state;
    P5Row *row;
    size_t count;
    size_t capacity;
    size_t *bucket;                 /* 0=empty, otherwise state index + 1 */
    size_t bucket_count;
    size_t transition_count;
} P5Machine;

typedef struct {
    size_t states;
    size_t transitions;
    uint64_t addmuls;
    size_t coefficient_objects;
    uint64_t peak_rss_bytes;
    double seconds;
} P5Stats;

/* Active coefficients occupy the half-open interval [low,high).  high=0
 * denotes an inactive state.  offset addresses one compact layer arena. */
typedef struct {
    unsigned low;
    unsigned high;
    size_t offset;
} P5Span;

typedef enum { P5_UPTO, P5_TERM, P5_CHECK } P5Mode;

typedef struct {
    int k;
    int n;
    P5Mode mode;
} P5Options;

static _Noreturn void p5_die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static size_t p5_add_size(size_t a, size_t b)
{
    if (b > SIZE_MAX-a) p5_die("size addition overflow");
    return a+b;
}

static size_t p5_mul_size(size_t a, size_t b)
{
    if (a != 0U && b > SIZE_MAX/a) p5_die("size multiplication overflow");
    return a*b;
}

static void *p5_calloc(size_t count, size_t size)
{
    (void)p5_mul_size(count, size);
    void *p = calloc(count == 0U ? 1U : count, size == 0U ? 1U : size);
    if (p == NULL) p5_die("out of memory");
    return p;
}

static void *p5_realloc(void *old, size_t count, size_t size)
{
    size_t bytes = p5_mul_size(count, size);
    void *p = realloc(old, bytes == 0U ? 1U : bytes);
    if (p == NULL) p5_die("out of memory");
    return p;
}

static double p5_now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) p5_die("clock_gettime failed");
    return (double)t.tv_sec + (double)t.tv_nsec/1000000000.0;
}

static uint64_t p5_peak_rss_bytes(void)
{
    struct rusage usage;
    if (getrusage(RUSAGE_SELF,&usage)!=0) p5_die("getrusage failed");
    if (usage.ru_maxrss < 0) p5_die("negative peak RSS");
#if defined(__APPLE__)
    return (uint64_t)usage.ru_maxrss;          /* bytes on macOS */
#else
    if ((uint64_t)usage.ru_maxrss>UINT64_MAX/UINT64_C(1024))
        p5_die("peak RSS conversion overflow");
    return (uint64_t)usage.ru_maxrss*UINT64_C(1024); /* KiB on Linux */
#endif
}

static uint8_t p5_byte(P5Key key, int slot)
{
    if (slot < 0 || slot >= P5_MAX_SLOTS) p5_die("state slot out of range");
    if (slot < 8) return (uint8_t)(key.low >> (8U*(unsigned)slot));
    return (uint8_t)(key.high >> (8U*(unsigned)(slot-8)));
}

static void p5_set_byte(P5Key *key, int slot, uint8_t byte)
{
    if (slot < 0 || slot >= P5_MAX_SLOTS) p5_die("state slot encoding overflow");
    if (slot < 8) key->low |= (uint64_t)byte << (8U*(unsigned)slot);
    else key->high |= (uint64_t)byte << (8U*(unsigned)(slot-8));
}

static bool p5_key_equal(P5Key a, P5Key b)
{
    return a.low == b.low && a.high == b.high && a.size == b.size;
}

static uint64_t p5_mix(uint64_t x)
{
    x ^= x >> 30U; x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27U; x *= UINT64_C(0x94d049bb133111eb);
    return x ^ (x >> 31U);
}

static size_t p5_hash(P5Key key)
{
    uint64_t r = (key.high << 23U) | (key.high >> 41U);
    return (size_t)p5_mix(key.low ^ r ^ ((uint64_t)key.size << 56U));
}

/* Canonical encoding. label[i] is meaningful exactly when degree[i]==1. */
static P5Key p5_encode(int size, const uint8_t degree[P5_MAX_BAG],
                       const int label[P5_MAX_BAG])
{
    if (size < 0 || size > P5_MAX_SLOTS) p5_die("invalid state size");
    int old_label[P5_MAX_BAG];
    int new_label[P5_MAX_BAG];
    int labels = 0;
    P5Key key = {UINT64_C(0), UINT64_C(0), (uint8_t)size};
    for (int i = 0; i < size; ++i) {
        if (degree[i] > 2U) p5_die("frontier degree exceeds 2");
        uint8_t byte = degree[i];
        if (degree[i] == 1U) {
            int found = -1;
            for (int j = 0; j < labels; ++j) {
                if (old_label[j] == label[i]) { found = new_label[j]; break; }
            }
            if (found < 0) {
                if (labels >= P5_MAX_BAG) p5_die("too many endpoint labels");
                old_label[labels] = label[i];
                new_label[labels] = labels;
                found = labels++;
            }
            byte |= (uint8_t)((unsigned)(found+1) << 2U);
        }
        p5_set_byte(&key, i, byte);
    }
    return key;
}

static void p5_decode(P5Key key, uint8_t degree[P5_MAX_BAG],
                      int label[P5_MAX_BAG])
{
    if (key.size > P5_MAX_SLOTS) p5_die("corrupt state size");
    int largest = -1;
    int occurrences[P5_MAX_BAG] = {0};
    for (int i = 0; i < (int)key.size; ++i) {
        uint8_t byte = p5_byte(key, i);
        degree[i] = byte & 3U;
        int encoded = (int)(byte >> 2U);
        if (degree[i] > 2U) p5_die("corrupt state degree");
        if (degree[i] == 1U) {
            if (encoded < 1 || encoded > largest+2) p5_die("corrupt endpoint label");
            label[i] = encoded-1;
            if (label[i] == largest+1) ++largest;
            if (++occurrences[label[i]] > 2) p5_die("endpoint label occurs over twice");
        } else {
            if (encoded != 0) p5_die("label attached to a non-endpoint");
            label[i] = -1;
        }
    }
    for (int i = (int)key.size; i < P5_MAX_BAG; ++i) {
        degree[i] = 0U; label[i] = -1;
    }
}

static bool p5_link(P5Key source, const int *selected, int selected_count,
                    int width, P5Key *target, uint8_t *factor)
{
    uint8_t degree[P5_MAX_BAG];
    int label[P5_MAX_BAG];
    p5_decode(source, degree, label);
    int old_size = source.size;
    int me = old_size;
    degree[me] = 0U; label[me] = -1;
    int next_label = 0;
    for (int i = 0; i < old_size; ++i) {
        if (degree[i] == 1U && label[i] >= next_label) next_label=label[i]+1;
    }
    for (int q = 0; q < selected_count; ++q) {
        int i = selected[q];
        if (i < 0 || i >= old_size || degree[i] >= 2U || degree[me] >= 2U)
            return false;
        if (degree[i] == 0U) {
            if (degree[me] == 0U) {
                int l = next_label++;
                degree[i]=1U; label[i]=l;
                degree[me]=1U; label[me]=l;
            } else {
                degree[i]=1U; label[i]=label[me];
                degree[me]=2U; label[me]=-1;
            }
        } else {
            int li=label[i];
            if (degree[me] == 0U) {
                degree[i]=2U; label[i]=-1;
                degree[me]=1U; label[me]=li;
            } else {
                int lm=label[me];
                if (li == lm) return false;             /* closes a cycle */
                degree[i]=2U; label[i]=-1;
                degree[me]=2U; label[me]=-1;
                for (int t=0; t<=me; ++t) if (label[t] == li) label[t]=lm;
            }
        }
    }
    int bag_size=old_size+1;
    int first=bag_size > width ? 1 : 0;
    int kept=bag_size-first;
    *factor = first != 0 && degree[0] >= 1U ? 2U : 1U;
    if (first != 0) {
        for (int i=0; i<kept; ++i) {
            degree[i]=degree[i+1]; label[i]=label[i+1];
        }
    }
    *target=p5_encode(kept, degree, label);
    return true;
}

static void p5_rehash(P5Machine *m, size_t new_count)
{
    size_t *bucket=p5_calloc(new_count, sizeof(*bucket));
    for (size_t i=0; i<m->count; ++i) {
        size_t slot=p5_hash(m->state[i].key)&(new_count-1U);
        while (bucket[slot] != 0U) slot=(slot+1U)&(new_count-1U);
        bucket[slot]=i+1U;
    }
    free(m->bucket); m->bucket=bucket; m->bucket_count=new_count;
}

static size_t p5_state_add(P5Machine *m, P5Key key)
{
    if (m->bucket_count == 0U) p5_rehash(m, 16U);
    size_t slot=p5_hash(key)&(m->bucket_count-1U);
    while (m->bucket[slot] != 0U) {
        size_t i=m->bucket[slot]-1U;
        if (m->state == NULL || i >= m->count) p5_die("corrupt state hash table");
        if (p5_key_equal(m->state[i].key,key)) return i;
        slot=(slot+1U)&(m->bucket_count-1U);
    }
    if (m->count >= P5_MAX_STATES) p5_die("frontier state limit reached");
    if (p5_mul_size(m->count+1U,10U) >= p5_mul_size(m->bucket_count,7U)) {
        if (m->bucket_count > SIZE_MAX/2U) p5_die("state hash size overflow");
        p5_rehash(m,m->bucket_count*2U);
        slot=p5_hash(key)&(m->bucket_count-1U);
        while (m->bucket[slot] != 0U) slot=(slot+1U)&(m->bucket_count-1U);
    }
    if (m->count == m->capacity) {
        size_t cap=m->capacity == 0U ? 16U : p5_mul_size(m->capacity,2U);
        if (cap > P5_MAX_STATES) cap=P5_MAX_STATES;
        m->state=p5_realloc(m->state,cap,sizeof(*m->state));
        m->row=p5_realloc(m->row,cap,sizeof(*m->row));
        memset(m->row+m->capacity,0,(cap-m->capacity)*sizeof(*m->row));
        m->capacity=cap;
    }
    size_t index=m->count++;
    m->state[index].key=key;
    unsigned covered=0;
    for (int i=0;i<(int)key.size;++i) if ((p5_byte(key,i)&3U)!=0U) ++covered;
    if (covered >= sizeof(unsigned long)*8U) p5_die("read-off weight overflow");
    m->state[index].readoff=1UL<<covered;
    m->bucket[slot]=index+1U;
    return index;
}

static void p5_machine_build(int width, P5Machine *m)
{
    memset(m,0,sizeof(*m)); m->width=width;
    P5Key empty={UINT64_C(0),UINT64_C(0),0U};
    (void)p5_state_add(m,empty);
    for (size_t source=0; source<m->count; ++source) {
        P5Key key=m->state[source].key;
        size_t maximum=1U+(size_t)key.size+
                       (size_t)key.size*((size_t)key.size-1U)/2U;
        P5Transition *row=p5_calloc(maximum,sizeof(*row));
        size_t used=0U;
        P5Key target; uint8_t factor;
        if (!p5_link(key,NULL,0,width,&target,&factor)) p5_die("zero-edge extension rejected");
        row[used++]=(P5Transition){p5_state_add(m,target),0U,factor};
        for (int i=0;i<(int)key.size;++i) {
            int one[1]={i};
            if (p5_link(key,one,1,width,&target,&factor))
                row[used++]=(P5Transition){p5_state_add(m,target),1U,factor};
            for (int j=i+1;j<(int)key.size;++j) {
                int two[2]={i,j};
                if (p5_link(key,two,2,width,&target,&factor))
                    row[used++]=(P5Transition){p5_state_add(m,target),2U,factor};
            }
        }
        m->transition_count=p5_add_size(m->transition_count,used);
        if (m->transition_count>P5_MAX_TRANSITIONS) p5_die("frontier transition limit reached");
        m->row[source].item=p5_realloc(row,used,sizeof(*row));
        m->row[source].count=used;
    }
}

static void p5_machine_destroy(P5Machine *m)
{
    for (size_t i=0;i<m->count;++i) free(m->row[i].item);
    free(m->state); free(m->row); free(m->bucket); memset(m,0,sizeof(*m));
}

static mpz_t *p5_mpz_array(size_t count)
{
    mpz_t *a=p5_calloc(count,sizeof(*a));
    for (size_t i=0;i<count;++i) mpz_init(a[i]);
    return a;
}

static void p5_mpz_destroy(mpz_t *a,size_t count)
{
    if (a==NULL) return;
    for (size_t i=0;i<count;++i) mpz_clear(a[i]);
    free(a);
}

static void p5_value(int n,int family_k,mpz_t answer,P5Stats *stats)
{
    memset(stats,0,sizeof(*stats)); double started=p5_now();
    if (n<0 || n>P5_MAX_N || family_k<0 || family_k>P5_MAX_TOTAL-2*n)
        p5_die("invalid n or k");
    int total=2*n+family_k;
    P5Machine machine; p5_machine_build(n,&machine);
    size_t degree_count=(size_t)total+1U;
    P5Span *current_span=p5_calloc(machine.count,sizeof(*current_span));
    P5Span *next_span=p5_calloc(machine.count,sizeof(*next_span));
    current_span[0].low=0U; current_span[0].high=1U; current_span[0].offset=0U;
    size_t current_count=1U;
    mpz_t *current=p5_mpz_array(current_count);
    mpz_set_ui(current[0],1UL);
    mpz_t *q=p5_mpz_array(degree_count);
    mpz_t *factorial=p5_mpz_array(degree_count);
    mpz_t *work=p5_mpz_array(degree_count);
    size_t fixed_objects=p5_mul_size(degree_count,3U);
    stats->coefficient_objects=p5_add_size(current_count,fixed_objects);
    for (int placed=0;placed<total;++placed) {
        memset(next_span,0,p5_mul_size(machine.count,sizeof(*next_span)));
        /* First pass: find the exact coefficient interval needed by each
         * destination.  This avoids S*(total+1) initialized mpz_t objects. */
        for (size_t source=0;source<machine.count;++source) {
            P5Span source_span=current_span[source];
            if (source_span.high==0U) continue;
            P5Row *row=&machine.row[source];
            for (size_t r=0;r<row->count;++r) {
                P5Transition tr=row->item[r];
                unsigned low=source_span.low+(unsigned)tr.edges;
                unsigned high=source_span.high+(unsigned)tr.edges;
                if (high>degree_count) p5_die("polynomial degree overflow");
                P5Span *destination=&next_span[tr.destination];
                if (destination->high==0U) {
                    destination->low=low; destination->high=high;
                } else {
                    if (low<destination->low) destination->low=low;
                    if (high>destination->high) destination->high=high;
                }
            }
        }
        size_t next_count=0U;
        for (size_t state=0;state<machine.count;++state) {
            P5Span *span=&next_span[state];
            if (span->high==0U) continue;
            if (span->low>=span->high) p5_die("invalid coefficient interval");
            span->offset=next_count;
            next_count=p5_add_size(next_count,(size_t)(span->high-span->low));
        }
        size_t simultaneous=p5_add_size(p5_add_size(current_count,next_count),
                                        fixed_objects);
        if (simultaneous>P5_MAX_MPZ_OBJECTS) {
            fprintf(stderr,
                    "error: n=%d,k=%d, layer %d needs %zu simultaneous GMP "
                    "objects (at least %.1f MiB of mpz headers; guard=%zu)\n",
                    n,family_k,placed+1,simultaneous,
                    (double)p5_mul_size(simultaneous,sizeof(mpz_t))/1048576.0,
                    P5_MAX_MPZ_OBJECTS);
            p5_mpz_destroy(current,current_count);
            p5_mpz_destroy(q,degree_count); p5_mpz_destroy(factorial,degree_count);
            p5_mpz_destroy(work,degree_count); free(current_span); free(next_span);
            p5_machine_destroy(&machine);
            p5_die("GMP coefficient-object limit reached");
        }
        if (simultaneous>stats->coefficient_objects)
            stats->coefficient_objects=simultaneous;
        mpz_t *next=p5_mpz_array(next_count);
        /* Second pass: apply the transitions to the compact arenas. */
        for (size_t source=0;source<machine.count;++source) {
            P5Span source_span=current_span[source];
            if (source_span.high==0U) continue;
            P5Row *row=&machine.row[source];
            for (size_t r=0;r<row->count;++r) {
                P5Transition tr=row->item[r];
                P5Span destination=next_span[tr.destination];
                for (unsigned e=source_span.low;e<source_span.high;++e) {
                    size_t source_index=p5_add_size(source_span.offset,
                                                    (size_t)(e-source_span.low));
                    if (mpz_sgn(current[source_index])==0) continue;
                    unsigned ne=e+(unsigned)tr.edges;
                    size_t destination_index=p5_add_size(destination.offset,
                                                         (size_t)(ne-destination.low));
                    mpz_addmul_ui(next[destination_index],current[source_index],tr.factor);
                    if (stats->addmuls==UINT64_MAX) p5_die("operation counter overflow");
                    ++stats->addmuls;
                }
            }
        }
        p5_mpz_destroy(current,current_count);
        current=next; current_count=next_count;
        P5Span *temporary=current_span; current_span=next_span; next_span=temporary;
    }
    for (size_t state=0;state<machine.count;++state) {
        P5Span span=current_span[state];
        if (span.high==0U) continue;
        for (unsigned e=span.low;e<span.high;++e) {
            size_t index=p5_add_size(span.offset,(size_t)(e-span.low));
            if (mpz_sgn(current[index])!=0)
                mpz_addmul_ui(q[e],current[index],machine.state[state].readoff);
        }
    }
    mpz_set_ui(factorial[0],1UL);
    for (int i=1;i<=total;++i) mpz_mul_ui(factorial[i],factorial[i-1],(unsigned long)i);
    mpz_set_ui(answer,0UL);
    for (int e=0;e<=total;++e) {
        if (mpz_sgn(q[e])==0) continue;
        if (!mpz_divisible_2exp_p(q[e],(mp_bitcnt_t)e))
            p5_die("2^edge divisibility invariant failed");
        mpz_fdiv_q_2exp(work[e],q[e],(mp_bitcnt_t)e);
        mpz_mul(work[e],work[e],factorial[total-e]);
        if ((e&1)==0) mpz_add(answer,answer,work[e]);
        else mpz_sub(answer,answer,work[e]);
    }
    if (mpz_sgn(answer)<0 || (total>1 && mpz_odd_p(answer)))
        p5_die("final count invariant failed");
    stats->states=machine.count; stats->transitions=machine.transition_count;
    stats->peak_rss_bytes=p5_peak_rss_bytes();
    stats->seconds=p5_now()-started;
    p5_mpz_destroy(current,current_count);
    p5_mpz_destroy(q,degree_count); p5_mpz_destroy(factorial,degree_count);
    p5_mpz_destroy(work,degree_count); free(current_span); free(next_span);
    p5_machine_destroy(&machine);
}

static const char *const p5_k2[] = {
    "2","2","2","2","2","2","2","2"
};
static const char *const p5_k3[] = {
    "6","14","32","72","160","352","768","1664","3584","7680"
};
static const char *const p5_k4[] = {
    "24","90","368","1496","6056","24440","98408","395576","1588136",
    "6370040","25532648","102288056"
};
static const char *const p5_k5[] = {
    "120","646","3984","25384","161136","1019616","6433728","40495488",
    "254319360","1593945600"
};
static const char *const p5_k6[] = {
    "720","5242","44304","399848","3661824","33461568","305193408","2779045440"
};
static const char *const p5_k7[] = {
    "5040","47622","521606","6231544","76972416","957913824","11902581120",
    "147689145984","1830235823616"
};
static const char *const p5_k8[] = {
    "40320","479306","6564318","99133496","1572313392","25415753280",
    "412583686272","6690112284672","108370981896192","1753844521231872"
};
static const char *const p5_k9[] = {
    "362880","5296790","88422296","1634227958","32096768008","649347224736"
};

static const char *p5_known(int k,int n)
{
#define P5_KNOWN(array) \
    (n>=0 && (size_t)n<sizeof(array)/sizeof((array)[0]) ? (array)[n] : NULL)
    if (k==2) return P5_KNOWN(p5_k2);
    if (k==3) return P5_KNOWN(p5_k3);
    if (k==4 && n>=0 && (size_t)n<sizeof(p5_k4)/sizeof(p5_k4[0])) return p5_k4[n];
    if (k==5 && n>=0 && (size_t)n<sizeof(p5_k5)/sizeof(p5_k5[0])) return p5_k5[n];
    if (k==6) return P5_KNOWN(p5_k6);
    if (k==7) return P5_KNOWN(p5_k7);
    if (k==8) return P5_KNOWN(p5_k8);
    if (k==9) return P5_KNOWN(p5_k9);
#undef P5_KNOWN
    return NULL;
}

static void p5_verify_known(int k,int n,const mpz_t value)
{
    const char *expected=p5_known(k,n);
    if (expected==NULL) return;
    mpz_t e; mpz_init_set_str(e,expected,10);
    if (mpz_cmp(value,e)!=0) {
        gmp_fprintf(stderr,"error: known-value mismatch at k=%d,n=%d: %Zd != %Zd\n",
                    k,n,value,e);
        mpz_clear(e); exit(EXIT_FAILURE);
    }
    mpz_clear(e);
}

static int p5_parse(const char *text,const char *label,int minimum,int maximum)
{
    char *end=NULL; errno=0; long value=strtol(text,&end,10);
    if (errno!=0 || end==text || *end!='\0' || value<minimum || value>maximum) {
        fprintf(stderr,"error: %s must be in %d..%d: %s\n",label,minimum,maximum,text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static void p5_usage(const char *program)
{
    fprintf(stderr,
      "usage: %s [MAX_N] [--k K]\n"
      "       %s --upto MAX_N [--k K]\n"
      "       %s --term N [--k K]\n"
      "       %s --check [MAX_N] [--k K]\n"
      "K is 0..%d, N is 0..%d. No recurrence is used.\n",
      program,program,program,program,P5_MAX_TOTAL,P5_MAX_N);
}

static P5Options p5_options(int argc,char **argv)
{
    P5Options o={P5_DEFAULT_K,P5_DEFAULT_N,P5_UPTO};
    bool have_k=false,have_n=false,have_mode=false;
    for (int a=1;a<argc;++a) {
        const char *s=argv[a];
        if (strcmp(s,"--help")==0 || strcmp(s,"-h")==0) { p5_usage(argv[0]); exit(0); }
        else if (strcmp(s,"--k")==0) {
            if (have_k || ++a>=argc) { p5_usage(argv[0]); exit(1); }
            o.k=p5_parse(argv[a],"K",0,P5_MAX_TOTAL); have_k=true;
        } else if (strcmp(s,"--term")==0 || strcmp(s,"--upto")==0) {
            if (have_mode || have_n || ++a>=argc) { p5_usage(argv[0]); exit(1); }
            o.mode=strcmp(s,"--term")==0?P5_TERM:P5_UPTO;
            o.n=p5_parse(argv[a],o.mode==P5_TERM?"N":"MAX_N",0,P5_MAX_N);
            have_mode=true; have_n=true;
        } else if (strcmp(s,"--check")==0) {
            if (have_mode || have_n) { p5_usage(argv[0]); exit(1); }
            o.mode=P5_CHECK; have_mode=true;
            if (a+1<argc && argv[a+1][0]!='-') { o.n=p5_parse(argv[++a],"CHECK_N",0,P5_MAX_N); have_n=true; }
        } else if (s[0]=='-' || have_n) { p5_usage(argv[0]); exit(1); }
        else { o.n=p5_parse(s,"MAX_N",0,P5_MAX_N); have_n=true; }
    }
    if (o.k>P5_MAX_TOTAL-2*o.n) p5_die("2*N+K exceeds implementation limit");
    return o;
}

static void p5_report(int n,int k,const P5Stats *s)
{
    fprintf(stderr,"179962_05: n=%d,k=%d, perm_gap frontier DP, states=%zu, "
            "transitions=%zu, GMP objects=%zu, peak RSS=%.1f MiB, "
            "addmuls=%" PRIu64 ", %.3f s\n",
            n,k,s->states,s->transitions,s->coefficient_objects,
            (double)s->peak_rss_bytes/1048576.0,s->addmuls,s->seconds);
}

static char *p5_path(const char *argv0,const char *name)
{
    const char *slash=strrchr(argv0,'/');
    if (slash==NULL) {
        char *p=p5_calloc(strlen(name)+1U,1U); memcpy(p,name,strlen(name)+1U); return p;
    }
    size_t directory=(size_t)(slash-argv0)+1U;
    size_t bytes=p5_add_size(directory,strlen(name)+1U);
    char *p=p5_calloc(bytes,1U); memcpy(p,argv0,directory); memcpy(p+directory,name,strlen(name)+1U); return p;
}

int main(int argc,char **argv)
{
    P5Options o=p5_options(argc,argv);
    if (o.mode==P5_TERM) {
        mpz_t value; mpz_init(value); P5Stats stats;
        p5_value(o.n,o.k,value,&stats); p5_verify_known(o.k,o.n,value);
        p5_report(o.n,o.k,&stats); gmp_printf("%d %Zd\n",o.n,value);
        mpz_clear(value); return EXIT_SUCCESS;
    }
    if (o.mode==P5_CHECK) {
        for (int n=0;n<=o.n;++n) {
            mpz_t value; mpz_init(value); P5Stats stats;
            p5_value(n,o.k,value,&stats); p5_verify_known(o.k,n,value); mpz_clear(value);
        }
        printf("ok: perm_gap frontier DP invariants and all embedded exact values "
               "in n=0..%d,k=%d\n",o.n,o.k);
        return EXIT_SUCCESS;
    }
    char final_name[80],part_name[80];
    if (snprintf(final_name,sizeof(final_name),"b179962_05_k%d.txt",o.k)<0 ||
        snprintf(part_name,sizeof(part_name),"b179962_05_k%d_part.txt",o.k)<0)
        p5_die("output filename formatting failed");
    char *final_path=p5_path(argv[0],final_name),*part_path=p5_path(argv[0],part_name);
    FILE *file=fopen(part_path,"w");
    if (file==NULL) { fprintf(stderr,"error: cannot open %s: %s\n",part_path,strerror(errno)); exit(1); }
    for (int n=0;n<=o.n;++n) {
        mpz_t value; mpz_init(value); P5Stats stats;
        p5_value(n,o.k,value,&stats); p5_verify_known(o.k,n,value);
        p5_report(n,o.k,&stats);
        if (gmp_fprintf(file,"%d %Zd\n",n,value)<0 || fflush(file)!=0) p5_die("output write failed");
        mpz_clear(value);
    }
    if (fclose(file)!=0) p5_die("output close failed");
    if (rename(part_path,final_path)!=0) {
        fprintf(stderr,"error: cannot rename %s to %s: %s\n",part_path,final_path,strerror(errno)); exit(1);
    }
    printf("wrote %s (n=0..%d,k=%d)\n",final_path,o.n,o.k);
    free(final_path); free(part_path); return EXIT_SUCCESS;
}
