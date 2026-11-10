#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static long long g_cmp = 0;
static size_t g_cur_stack = 0;
static size_t g_max_stack = 0;
static size_t g_extra_heap = 0;

#define RET_ADDR sizeof(void *)

#define FRAME_SIFT_DOWN  (sizeof(int *) + 2 * sizeof(int) + 3 * sizeof(int) + RET_ADDR)
#define FRAME_HEAP_SORT  (sizeof(int *) + sizeof(int) + 2 * sizeof(int) + RET_ADDR)

#define FRAME_MERGE_SORT (2 * sizeof(int *) + 2 * sizeof(int) + sizeof(int) + RET_ADDR)
#define FRAME_MERGE      (2 * sizeof(int *) + 3 * sizeof(int) + 3 * sizeof(int) + RET_ADDR)

#define FRAME_QUICK      (sizeof(int *) + 3 * sizeof(int) + sizeof(int) + RET_ADDR)
#define FRAME_PARTITION  (sizeof(int *) + 3 * sizeof(int) + 5 * sizeof(int) + RET_ADDR)
#define FRAME_CHOOSE     (sizeof(int *) + 3 * sizeof(int) + 5 * sizeof(int) + RET_ADDR)

#define STACK_ENTER(sz) do { g_cur_stack += (sz); \
                             if (g_cur_stack > g_max_stack) g_max_stack = g_cur_stack; } while (0)
#define STACK_EXIT(sz)  do { g_cur_stack -= (sz); } while (0)

#define SWAP(a, b) do { int _t = (a); (a) = (b); (b) = _t; } while (0)

static void sift_down(int* arr, int i, int n)
{
    int left, largest;
    STACK_ENTER(FRAME_SIFT_DOWN);
    while (1) {
        left = 2 * i + 1;
        if (left >= n) break;
        largest = left;
        if (left + 1 < n) {
            g_cmp++;
            if (arr[left + 1] > arr[left]) largest = left + 1;
        }
        g_cmp++;
        if (arr[largest] > arr[i]) {
            SWAP(arr[largest], arr[i]);
            i = largest;
        }
        else {
            break;
        }
    }
    STACK_EXIT(FRAME_SIFT_DOWN);
}

static void heap_sort(int* arr, int n)
{
    int i;
    STACK_ENTER(FRAME_HEAP_SORT);
    for (i = n / 2 - 1; i >= 0; i--)
        sift_down(arr, i, n);
    for (i = n - 1; i > 0; i--) {
        SWAP(arr[0], arr[i]);
        sift_down(arr, 0, i);
    }
    STACK_EXIT(FRAME_HEAP_SORT);
}

static void merge(int* arr, int* tmp, int left, int mid, int right)
{
    int i = left, j = mid + 1, k = left;
    STACK_ENTER(FRAME_MERGE);
    while (i <= mid && j <= right) {
        g_cmp++;
        if (arr[i] <= arr[j]) tmp[k++] = arr[i++];
        else                  tmp[k++] = arr[j++];
    }
    while (i <= mid)   tmp[k++] = arr[i++];
    while (j <= right) tmp[k++] = arr[j++];
    for (k = left; k <= right; k++) arr[k] = tmp[k];
    STACK_EXIT(FRAME_MERGE);
}

static void merge_sort_rec(int* arr, int* tmp, int left, int right)
{
    int mid;
    STACK_ENTER(FRAME_MERGE_SORT);
    if (left < right) {
        mid = left + (right - left) / 2;
        merge_sort_rec(arr, tmp, left, mid);
        merge_sort_rec(arr, tmp, mid + 1, right);
        merge(arr, tmp, left, mid, right);
    }
    STACK_EXIT(FRAME_MERGE_SORT);
}

static void merge_sort(int* arr, int n)
{
    int* tmp = (int*)malloc(sizeof(int) * n);
    if (!tmp) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    g_extra_heap += sizeof(int) * (size_t)n;
    merge_sort_rec(arr, tmp, 0, n - 1);
    free(tmp);
}

enum { PIVOT_FIRST = 0, PIVOT_RANDOM = 1, PIVOT_MEDIAN3 = 2 };

static unsigned int rand_u(unsigned int range)
{
    unsigned int r = ((unsigned int)(rand() % 32768) * 32768u) +
        (unsigned int)(rand() % 32768);
    return r % range;
}

static int choose_pivot(int* arr, int low, int high, int mode)
{
    int mid, a, b, c;
    int result;
    STACK_ENTER(FRAME_CHOOSE);

    if (mode == PIVOT_FIRST) {
        result = low;
    }
    else if (mode == PIVOT_RANDOM) {
        result = low + (int)rand_u((unsigned int)(high - low + 1));
    }
    else {
        mid = low + (high - low) / 2;
        a = arr[low]; b = arr[mid]; c = arr[high];
        g_cmp++;
        if (a < b) {
            g_cmp++;
            if (b < c) result = mid;
            else {
                g_cmp++;
                result = (a < c) ? high : low;
            }
        }
        else {
            g_cmp++;
            if (a < c) result = low;
            else {
                g_cmp++;
                result = (b < c) ? high : mid;
            }
        }
    }
    STACK_EXIT(FRAME_CHOOSE);
    return result;
}

static int partition(int* arr, int low, int high, int mode)
{
    int p, pivot, i, j;
    STACK_ENTER(FRAME_PARTITION);

    p = choose_pivot(arr, low, high, mode);
    SWAP(arr[low], arr[p]);
    pivot = arr[low];
    i = low;
    for (j = low + 1; j <= high; j++) {
        g_cmp++;
        if (arr[j] < pivot) {
            i++;
            SWAP(arr[i], arr[j]);
        }
    }
    SWAP(arr[low], arr[i]);

    STACK_EXIT(FRAME_PARTITION);
    return i;
}

static void quick_sort_rec(int* arr, int low, int high, int mode)
{
    int p;
    STACK_ENTER(FRAME_QUICK);
    if (low < high) {
        p = partition(arr, low, high, mode);
        quick_sort_rec(arr, low, p - 1, mode);
        quick_sort_rec(arr, p + 1, high, mode);
    }
    STACK_EXIT(FRAME_QUICK);
}

static void quick_first(int* arr, int n) { quick_sort_rec(arr, 0, n - 1, PIVOT_FIRST); }
static void quick_random(int* arr, int n) { quick_sort_rec(arr, 0, n - 1, PIVOT_RANDOM); }
static void quick_median3(int* arr, int n) { quick_sort_rec(arr, 0, n - 1, PIVOT_MEDIAN3); }

static double now_ms(void)
{
#ifdef _WIN32
    LARGE_INTEGER freq, cnt;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&cnt);
    return (double)cnt.QuadPart * 1000.0 / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1.0e6;
#endif
}

static int is_sorted(const int* arr, int n)
{
    int i;
    for (i = 1; i < n; i++)
        if (arr[i - 1] > arr[i]) return 0;
    return 1;
}

typedef void (*SortFn)(int*, int);

typedef struct {
    const char* name;
    SortFn fn;
} Algo;

int main(void)
{
    const int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    const int num_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));
    const Algo algos[] = {
        { "Heap Sort",       heap_sort     },
        { "Merge Sort",      merge_sort    },
        { "Quick (First)",   quick_first   },
        { "Quick (Random)",  quick_random  },
        { "Quick (Median3)", quick_median3 }
    };
    const int num_algos = (int)(sizeof(algos) / sizeof(algos[0]));
    int s, a, i;

    srand(time(NULL));

    for (s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        int* original = (int*)malloc(sizeof(int) * n);
        int* work = (int*)malloc(sizeof(int) * n);
        if (!original || !work) { fprintf(stderr, "메모리 할당 실패\n"); return 1; }

        for (i = 0; i < n; i++)
            original[i] = rand() % 1000 * 1000 + rand() % 1000;

        printf("Data Size : %d\n\n", n);
        printf("%-15s %-14s %-11s %s\n", "Algorithm", "Comparisons", "Time(ms)", "Memory(Bytes)");
        printf("--------------------------------------------------------\n");

        for (a = 0; a < num_algos; a++) {
            double t0, t1;

            memcpy(work, original, sizeof(int) * n);

            g_cmp = 0;
            g_cur_stack = g_max_stack = 0;
            g_extra_heap = 0;

            t0 = now_ms();
            algos[a].fn(work, n);
            t1 = now_ms();

            if (!is_sorted(work, n))
                fprintf(stderr, "[오류] %s: 정렬 결과가 올바르지 않습니다!\n", algos[a].name);

            printf("%-15s %-14lld %-11.3f %zu\n",
                algos[a].name, g_cmp, t1 - t0, g_max_stack + g_extra_heap);
        }
        printf("\n");

        free(original);
        free(work);
    }
    return 0;
}