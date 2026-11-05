#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

static long long g_cmp = 0;
static uintptr_t g_stack_base = 0;
static size_t    g_stack_max = 0;

static void track_stack(uintptr_t cur)
{
    size_t used = (g_stack_base > cur) ? (size_t)(g_stack_base - cur)
        : (size_t)(cur - g_stack_base);
    if (used > g_stack_max) g_stack_max = used;
}
#define TRACK_STACK() do { volatile char _probe = 0; track_stack((uintptr_t)&_probe); } while (0)

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

static int* g_tmp = NULL;

static void merge(int a[], int lo, int mid, int hi)
{
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) {
        g_cmp++;
        if (a[i] <= a[j]) g_tmp[k++] = a[i++];
        else              g_tmp[k++] = a[j++];
    }
    while (i <= mid) g_tmp[k++] = a[i++];
    while (j <= hi)  g_tmp[k++] = a[j++];
    for (k = lo; k <= hi; k++) a[k] = g_tmp[k];
}

static void merge_sort_rec(int a[], int lo, int hi)
{
    TRACK_STACK();
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort_rec(a, lo, mid);
    merge_sort_rec(a, mid + 1, hi);
    merge(a, lo, mid, hi);
}

static size_t merge_sort(int a[], int n)
{
    g_tmp = (int*)malloc(sizeof(int) * (size_t)n);
    if (!g_tmp) { fprintf(stderr, "malloc failed\n"); exit(1); }
    merge_sort_rec(a, 0, n - 1);
    free(g_tmp);
    g_tmp = NULL;
    return sizeof(int) * (size_t)n;
}

enum { PIVOT_FIRST, PIVOT_RANDOM, PIVOT_MEDIAN3 };

static void swap_int(int* x, int* y) { int t = *x; *x = *y; *y = t; }

static int rand_range(int lo, int hi)
{
    unsigned r = ((unsigned)rand() << 15) ^ (unsigned)rand();
    return lo + (int)(r % (unsigned)(hi - lo + 1));
}

static int median_of_three(int a[], int lo, int hi)
{
    int mid = lo + (hi - lo) / 2;
    int x = a[lo], y = a[mid], z = a[hi];
    g_cmp++;
    if (x < y) {
        g_cmp++;
        if (y < z) return mid;
        g_cmp++;
        return (x < z) ? hi : lo;
    }
    else {
        g_cmp++;
        if (x < z) return lo;
        g_cmp++;
        return (y < z) ? hi : mid;
    }
}

static int choose_pivot(int a[], int lo, int hi, int method)
{
    switch (method) {
    case PIVOT_FIRST:   return lo;
    case PIVOT_RANDOM:  return rand_range(lo, hi);
    default:            return median_of_three(a, lo, hi);
    }
}

static int partition(int a[], int lo, int hi, int method)
{
    int p = choose_pivot(a, lo, hi, method);
    swap_int(&a[p], &a[hi]);
    int pivot = a[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        g_cmp++;
        if (a[j] <= pivot) {
            i++;
            swap_int(&a[i], &a[j]);
        }
    }
    swap_int(&a[i + 1], &a[hi]);
    return i + 1;
}

static void quick_sort_rec(int a[], int lo, int hi, int method)
{
    TRACK_STACK();
    if (lo >= hi) return;
    int p = partition(a, lo, hi, method);
    quick_sort_rec(a, lo, p - 1, method);
    quick_sort_rec(a, p + 1, hi, method);
}

static int is_sorted(const int a[], int n)
{
    for (int i = 1; i < n; i++)
        if (a[i - 1] > a[i]) return 0;
    return 1;
}

typedef struct {
    long long cmp;
    double    ms;
    size_t    mem;
    int       ok;
} Result;

static Result run(int alg, const int original[], int work[], int n)
{
    Result r;
    memcpy(work, original, sizeof(int) * (size_t)n);

    g_cmp = 0;
    g_stack_max = 0;
    { volatile char base = 0; g_stack_base = (uintptr_t)&base; }

    size_t extra = 0;
    double t0 = now_ms();
    if (alg == 0) extra = merge_sort(work, n);
    else          quick_sort_rec(work, 0, n - 1, alg - 1);
    double t1 = now_ms();

    r.cmp = g_cmp;
    r.ms = t1 - t0;
    r.mem = extra + g_stack_max;
    r.ok = is_sorted(work, n);
    return r;
}

int main(void)
{
    const int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    const int num_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));
    const char* names[] = { "Merge Sort", "Quick (First)", "Quick (Random)", "Quick (Median3)" };

    srand(time(NULL));

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        int* original = (int*)malloc(sizeof(int) * (size_t)n);
        int* work = (int*)malloc(sizeof(int) * (size_t)n);
        if (!original || !work) { fprintf(stderr, "malloc failed\n"); return 1; }

        for (int i = 0; i < n; i++)
            original[i] = rand() % 100000;

        printf("Data Size : %d\n\n", n);
        printf("%-18s%-15s%-12s%s\n", "Algorithm", "Comparisons", "Time(ms)", "Memory(Bytes)");
        printf("--------------------------------------------------------------\n");

        for (int alg = 0; alg < 4; alg++) {
            Result r = run(alg, original, work, n);
            printf("%-18s%-15lld%-12.3f%zu%s\n", names[alg], r.cmp, r.ms, r.mem,
                r.ok ? "" : "   [정렬 실패!]");
        }
        printf("\n");

        free(original);
        free(work);
    }
    return 0;
}