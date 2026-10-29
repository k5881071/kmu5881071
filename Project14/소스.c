#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long long cmp_count = 0;

int is_sorted(const int* a, int n)
{
    for (int i = 0; i < n - 1; i++) {
        if (a[i] > a[i + 1])
            return 0;
    }
    return 1;
}

long long insertion_sort(int* a, int n)
{
    cmp_count = 0;
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0) {
            cmp_count++;
            if (a[j] > key) {
                a[j + 1] = a[j];
                j--;
            }
            else {
                break;
            }
        }
        a[j + 1] = key;
    }
    return cmp_count;
}

static void gap_insertion(int* a, int n, int h)
{
    for (int i = h; i < n; i++) {
        int key = a[i];
        int j = i - h;
        while (j >= 0) {
            cmp_count++;
            if (a[j] > key) {
                a[j + h] = a[j];
                j -= h;
            }
            else {
                break;
            }
        }
        a[j + h] = key;
    }
}

long long shell_sort_original(int* a, int n)
{
    cmp_count = 0;
    for (int h = n / 2; h >= 1; h /= 2)
        gap_insertion(a, n, h);
    return cmp_count;
}

long long shell_sort_knuth(int* a, int n)
{
    cmp_count = 0;

    int h = 1;
    while (3 * h + 1 < n)
        h = 3 * h + 1;

    for (; h >= 1; h = (h - 1) / 3) {
        gap_insertion(a, n, h);
        if (h == 1)
            break;
    }
    return cmp_count;
}

int main(void)
{
    int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    srand((unsigned int)time(NULL));

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];

        int* original = (int*)malloc(sizeof(int) * n);
        int* work = (int*)malloc(sizeof(int) * n);
        if (original == NULL || work == NULL) {
            printf("메모리 할당 실패\n");
            return 1;
        }

        for (int i = 0; i < n; i++)
            original[i] = rand();

        for (int i = 0; i < n; i++) work[i] = original[i];
        long long c1 = insertion_sort(work, n);
        int ok1 = is_sorted(work, n);

        for (int i = 0; i < n; i++) work[i] = original[i];
        long long c2 = shell_sort_original(work, n);
        int ok2 = is_sorted(work, n);

        for (int i = 0; i < n; i++) work[i] = original[i];
        long long c3 = shell_sort_knuth(work, n);
        int ok3 = is_sorted(work, n);

        printf("Data Size : %d\n\n", n);
        printf("Insertion Sort       : %lld comparisons\n", c1);
        printf("Shell Sort (Original): %lld comparisons\n", c2);
        printf("Shell Sort (Knuth)   : %lld comparisons\n", c3);

        if (!(ok1 && ok2 && ok3))
            printf("[경고] 정렬 결과가 올바르지 않습니다! (%d, %d, %d)\n", ok1, ok2, ok3);
        printf("\n");

        free(original);
        free(work);
    }

    return 0;
}