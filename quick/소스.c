#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_N 20000
#define VALUE_RANGE 100000

typedef enum { PIVOT_FIRST, PIVOT_RANDOM, PIVOT_MEDIAN3 } PivotType;

static long long comparisons = 0;

static void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

static int rand_range(int lo, int hi)
{
    unsigned int r = ((unsigned int)rand() << 15) ^ (unsigned int)rand();
    return lo + (int)(r % (unsigned int)(hi - lo + 1));
}

static int median_of_three(int a[], int lo, int hi)
{
    int mid = lo + (hi - lo) / 2;
    int x = a[lo], y = a[mid], z = a[hi];

    comparisons++;
    if (x < y) {
        comparisons++;
        if (y < z) return mid;
        comparisons++;
        if (x < z) return hi;
        return lo;
    }
    else {
        comparisons++;
        if (x < z) return lo;
        comparisons++;
        if (y < z) return hi;
        return mid;
    }
}

static int choose_pivot(int a[], int lo, int hi, PivotType type)
{
    switch (type) {
    case PIVOT_FIRST:   return lo;
    case PIVOT_RANDOM:  return rand_range(lo, hi);
    case PIVOT_MEDIAN3: return median_of_three(a, lo, hi);
    }
    return lo;
}

static int partition(int a[], int lo, int hi, PivotType type)
{
    int p = choose_pivot(a, lo, hi, type);
    swap(&a[lo], &a[p]);
    int pivot = a[lo];
    int i = lo;

    for (int j = lo + 1; j <= hi; j++) {
        comparisons++;
        if (a[j] < pivot) {
            i++;
            swap(&a[i], &a[j]);
        }
    }
    swap(&a[lo], &a[i]);
    return i;
}

static void quick_sort(int a[], int lo, int hi, PivotType type)
{
    while (lo < hi) {
        int p = partition(a, lo, hi, type);
        if (p - lo < hi - p) {
            quick_sort(a, lo, p - 1, type);
            lo = p + 1;
        }
        else {
            quick_sort(a, p + 1, hi, type);
            hi = p - 1;
        }
    }
}

static int is_sorted(const int a[], int n)
{
    for (int i = 1; i < n; i++)
        if (a[i - 1] > a[i]) return 0;
    return 1;
}

static long long run_sort(const int original[], int n, PivotType type, const char* name)
{
    int* copy = (int*)malloc(sizeof(int) * n);
    if (copy == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        exit(1);
    }
    memcpy(copy, original, sizeof(int) * n);

    comparisons = 0;
    quick_sort(copy, 0, n - 1, type);
    long long result = comparisons;

    if (!is_sorted(copy, n))
        fprintf(stderr, "[오류] %s 정렬 결과가 올바르지 않습니다. (n=%d)\n", name, n);

    free(copy);
    return result;
}

int main(void)
{
    int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    int count = sizeof(sizes) / sizeof(sizes[0]);
    int* data = (int*)malloc(sizeof(int) * MAX_N);
    if (data == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        return 1;
    }

    srand((unsigned int)time(NULL));

    for (int k = 0; k < count; k++) {
        int n = sizes[k];

        for (int i = 0; i < n; i++)
            data[i] = rand_range(0, VALUE_RANGE - 1);

        long long c_first = run_sort(data, n, PIVOT_FIRST, "First");
        long long c_random = run_sort(data, n, PIVOT_RANDOM, "Random");
        long long c_median = run_sort(data, n, PIVOT_MEDIAN3, "Median-3");

        printf("Data Size : %d\n\n", n);
        printf("Quick Sort (First)    : %lld comparisons\n", c_first);
        printf("Quick Sort (Random)   : %lld comparisons\n", c_random);
        printf("Quick Sort (Median-3) : %lld comparisons\n\n", c_median);
    }

    free(data);
    return 0;
}