#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

typedef unsigned long long u64;

#define VALUE_RANGE        100000
#define STACK_FRAME        32
#define ROTATION_MOVE_COST 0

static u64 g_cmp, g_move;
static size_t g_cur_mem, g_peak_mem;
static int g_depth, g_maxdepth;

#define ENTER() do { if (++g_depth > g_maxdepth) g_maxdepth = g_depth; } while (0)
#define LEAVE() (g_depth--)

static void* tmalloc(size_t sz)
{
    void* p = malloc(sz);
    if (!p) { fprintf(stderr, "memory allocation failed\n"); exit(1); }
    g_cur_mem += sz;
    if (g_cur_mem > g_peak_mem) g_peak_mem = g_cur_mem;
    return p;
}

static void tfree(void* p, size_t sz)
{
    free(p);
    g_cur_mem -= sz;
}

static void reset_counters(void)
{
    g_cmp = g_move = 0;
    g_cur_mem = g_peak_mem = 0;
    g_depth = g_maxdepth = 0;
}

static double now_ms(void)
{
#ifdef _WIN32
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1000.0 / (double)f.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
#endif
}

static int rand_val(void)
{
    unsigned r = ((unsigned)rand() << 15) ^ (unsigned)rand();
    return (int)(r % VALUE_RANGE);
}

static void swap_int(int* a, int* b)
{
    int t = *a; *a = *b; *b = t;
    g_move += 3;
}

static void quick_sort(int* a, int lo, int hi)
{
    ENTER();
    if (hi - lo == 1) {
        g_cmp++;
        if (a[hi] < a[lo]) swap_int(&a[hi], &a[lo]);
    }
    else if (hi - lo >= 2) {
        int mid = lo + (hi - lo) / 2;

        g_cmp++; if (a[mid] < a[lo])  swap_int(&a[mid], &a[lo]);
        g_cmp++; if (a[hi] < a[lo])  swap_int(&a[hi], &a[lo]);
        g_cmp++; if (a[hi] < a[mid]) swap_int(&a[hi], &a[mid]);

        if (hi - lo >= 3) {
            int pivot = a[mid];
            int i = lo, j = hi - 1;
            swap_int(&a[mid], &a[hi - 1]);
            for (;;) {
                do { i++; g_cmp++; } while (a[i] < pivot);
                do { j--; g_cmp++; } while (a[j] > pivot);
                if (i < j) swap_int(&a[i], &a[j]);
                else break;
            }
            swap_int(&a[i], &a[hi - 1]);
            quick_sort(a, lo, i - 1);
            quick_sort(a, i + 1, hi);
        }
    }
    LEAVE();
}

typedef struct Node {
    int key;
    int h;
    struct Node* l, * r;
} Node;

static int g_nodes;

static int height(Node* n) { return n ? n->h : 0; }

static void upd(Node* n)
{
    int a = height(n->l), b = height(n->r);
    n->h = (a > b ? a : b) + 1;
}

static Node* rot_right(Node* y)
{
    Node* x = y->l;
    y->l = x->r;
    x->r = y;
    g_move += ROTATION_MOVE_COST;
    upd(y); upd(x);
    return x;
}

static Node* rot_left(Node* x)
{
    Node* y = x->r;
    x->r = y->l;
    y->l = x;
    g_move += ROTATION_MOVE_COST;
    upd(x); upd(y);
    return y;
}

static Node* balance(Node* n)
{
    int bf;
    upd(n);
    bf = height(n->l) - height(n->r);
    if (bf > 1) {
        if (height(n->l->l) < height(n->l->r)) n->l = rot_left(n->l);
        return rot_right(n);
    }
    if (bf < -1) {
        if (height(n->r->r) < height(n->r->l)) n->r = rot_right(n->r);
        return rot_left(n);
    }
    return n;
}

static Node* avl_insert(Node* n, int key)
{
    ENTER();
    if (!n) {
        n = (Node*)tmalloc(sizeof(Node));
        n->key = key; n->h = 1; n->l = n->r = NULL;
        g_move++;
        g_nodes++;
    }
    else {
        g_cmp++;
        if (key < n->key) n->l = avl_insert(n->l, key);
        else              n->r = avl_insert(n->r, key);
        n = balance(n);
    }
    LEAVE();
    return n;
}

static Node* remove_min(Node* n, int* minkey)
{
    Node* res;
    ENTER();
    if (!n->l) {
        res = n->r;
        *minkey = n->key;
        tfree(n, sizeof(Node));
        g_nodes--;
    }
    else {
        n->l = remove_min(n->l, minkey);
        res = balance(n);
    }
    LEAVE();
    return res;
}

static Node* avl_delete(Node* n, int key)
{
    Node* res;
    ENTER();
    if (!n) {
        res = NULL;
    }
    else {
        int c;
        g_cmp++;
        c = (key > n->key) - (key < n->key);
        if (c < 0) {
            n->l = avl_delete(n->l, key);
            res = balance(n);
        }
        else if (c > 0) {
            n->r = avl_delete(n->r, key);
            res = balance(n);
        }
        else if (!n->l || !n->r) {
            res = n->l ? n->l : n->r;
            tfree(n, sizeof(Node));
            g_nodes--;
        }
        else {
            int m;
            n->r = remove_min(n->r, &m);
            n->key = m;
            g_move++;
            res = balance(n);
        }
    }
    LEAVE();
    return res;
}

static void inorder(Node* n, int* out, int* idx)
{
    ENTER();
    if (n) {
        inorder(n->l, out, idx);
        out[(*idx)++] = n->key;
        g_move++;
        inorder(n->r, out, idx);
    }
    LEAVE();
}

static void free_tree(Node* n)
{
    if (!n) return;
    free_tree(n->l);
    free_tree(n->r);
    free(n);
}

typedef struct {
    double ms;
    u64 cmp, move;
    size_t mem;
    int final_n;
} Result;

static Result run_quick(const int* orig, int n, const int* ops, int nops, int* result)
{
    Result r;
    int cap = n + nops;
    int cnt = n;
    int* a;
    int i, k;
    double t0, t1;

    reset_counters();
    t0 = now_ms();

    a = (int*)tmalloc((size_t)cap * sizeof(int));
    for (i = 0; i < n; i++) { a[i] = orig[i]; g_move++; }

    for (k = 0; k < nops; k++) {
        int v = ops[k];
        if (v % 2 == 0) {
            a[cnt++] = v;
            g_move++;
        }
        else {
            int idx = -1;
            for (i = 0; i < cnt; i++) {
                g_cmp++;
                if (a[i] == v) { idx = i; break; }
            }
            if (idx >= 0) {
                for (i = idx; i < cnt - 1; i++) { a[i] = a[i + 1]; g_move++; }
                cnt--;
            }
        }
    }

    if (cnt > 1) quick_sort(a, 0, cnt - 1);

    t1 = now_ms();

    r.ms = t1 - t0;
    r.cmp = g_cmp;
    r.move = g_move;
    r.mem = g_peak_mem + (size_t)g_maxdepth * STACK_FRAME;
    r.final_n = cnt;
    memcpy(result, a, (size_t)cnt * sizeof(int));
    free(a);
    return r;
}

static Result run_avl(const int* orig, int n, const int* ops, int nops, int* result)
{
    Result r;
    Node* root = NULL;
    int* out;
    int idx = 0;
    int i, k;
    double t0, t1;

    reset_counters();
    g_nodes = 0;
    t0 = now_ms();

    for (i = 0; i < n; i++) root = avl_insert(root, orig[i]);

    for (k = 0; k < nops; k++) {
        int v = ops[k];
        if (v % 2 == 0) root = avl_insert(root, v);
        else            root = avl_delete(root, v);
    }

    out = (int*)tmalloc((size_t)g_nodes * sizeof(int));
    inorder(root, out, &idx);

    t1 = now_ms();

    r.ms = t1 - t0;
    r.cmp = g_cmp;
    r.move = g_move;
    r.mem = g_peak_mem + (size_t)g_maxdepth * STACK_FRAME;
    r.final_n = idx;
    memcpy(result, out, (size_t)idx * sizeof(int));
    free(out);
    free_tree(root);
    return r;
}

static void print_report(int n, int nops, const Result* q, const Result* a)
{
    char c1[64], c2[64];

    printf("Data Size : %d\n", n);
    printf("Operations: %d\n\n", nops);
    printf("%-18s%-16s%s\n", "", "Quick Sort", "AVL Tree");
    printf("------------------------------------------------\n");

    snprintf(c1, sizeof c1, "%.3f ms", q->ms);
    snprintf(c2, sizeof c2, "%.3f ms", a->ms);
    printf("%-18s%-16s%s\n", "Total Time", c1, c2);

    snprintf(c1, sizeof c1, "%llu", q->cmp);
    snprintf(c2, sizeof c2, "%llu", a->cmp);
    printf("%-18s%-16s%s\n", "Comparisons", c1, c2);

    snprintf(c1, sizeof c1, "%llu", q->move);
    snprintf(c2, sizeof c2, "%llu", a->move);
    printf("%-18s%-16s%s\n", "Data Movements", c1, c2);

    snprintf(c1, sizeof c1, "%zu Bytes", q->mem);
    snprintf(c2, sizeof c2, "%zu Bytes", a->mem);
    printf("%-18s%-16s%s\n", "Memory Usage", c1, c2);

    printf("\n");
    snprintf(c1, sizeof c1, "%d", q->final_n);
    snprintf(c2, sizeof c2, "%d", a->final_n);
    printf("%-18s%-16s%s\n", "Final Data Count", c1, c2);
    printf("\n");
    printf("------------------------------------------------\n");
}

int main(int argc, char* argv[])
{
    const int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    const int nsizes = (int)(sizeof(sizes) / sizeof(sizes[0]));
    unsigned seed = (argc > 1) ? (unsigned)strtoul(argv[1], NULL, 10)
        : (unsigned)time(NULL);
    int s, i;

    srand(seed);

    for (s = 0; s < nsizes; s++) {
        int n = sizes[s];
        int nops = n / 5;
        int ok;
        Result rq, ra;
        int* orig = (int*)malloc((size_t)n * sizeof(int));
        int* ops = (int*)malloc((size_t)nops * sizeof(int));
        int* res_q = (int*)malloc((size_t)(n + nops) * sizeof(int));
        int* res_a = (int*)malloc((size_t)(n + nops) * sizeof(int));
        if (!orig || !ops || !res_q || !res_a) {
            fprintf(stderr, "out of memory\n");
            return 1;
        }

        for (i = 0; i < n; i++)    orig[i] = rand_val();
        for (i = 0; i < nops; i++) ops[i] = rand_val();

        rq = run_quick(orig, n, ops, nops, res_q);
        ra = run_avl(orig, n, ops, nops, res_a);

        ok = (rq.final_n == ra.final_n) &&
            (memcmp(res_q, res_a, (size_t)rq.final_n * sizeof(int)) == 0);
        for (i = 1; ok && i < rq.final_n; i++)
            if (res_q[i - 1] > res_q[i]) ok = 0;
        if (!ok) fprintf(stderr, "ERROR: result mismatch (size %d)\n", n);

        print_report(n, nops, &rq, &ra);

        free(orig); free(ops); free(res_q); free(res_a);
    }
    return 0;
}