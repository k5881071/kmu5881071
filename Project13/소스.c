#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_COUNT   10000
#define MAX_KEY      1000000
#define TRIALS       100
#define SEED         (unsigned)time(NULL)
#define PARTIAL_SWAPS 500

typedef struct {
    int key1;
    int key2;
    int order;
} Data;

typedef struct {
    long long cmp;
    long long mov;
} Counter;

static void swap_data(Data* a, Data* b, Counter* c)
{
    Data temp = *a;  c->mov++;
    *a = *b;         c->mov++;
    *b = temp;       c->mov++;
}

static void selection_sort(Data* a, int n, Counter* c)
{
    for (int i = 0; i < n - 1; i++) {
        int min = i;
        for (int j = i + 1; j < n; j++) {
            c->cmp++;
            if (a[j].key1 < a[min].key1)
                min = j;
        }
        if (min != i)
            swap_data(&a[i], &a[min], c);
    }
}

static void bubble_sort(Data* a, int n, Counter* c, int early_stop)
{
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            c->cmp++;
            if (a[j].key1 > a[j + 1].key1) {
                swap_data(&a[j], &a[j + 1], c);
                swapped = 1;
            }
        }
        if (early_stop && !swapped)
            break;
    }
}

static void insertion_sort(Data* a, int n, Counter* c, int use_ge)
{
    for (int i = 1; i < n; i++) {
        Data current = a[i];  c->mov++;
        int j = i - 1;
        while (j >= 0) {
            c->cmp++;
            int shift = use_ge ? (a[j].key1 >= current.key1)
                : (a[j].key1 > current.key1);
            if (!shift) break;
            a[j + 1] = a[j];  c->mov++;
            j--;
        }
        a[j + 1] = current;   c->mov++;
    }
}

enum { SEL, BUB, BUB_ES, INS_GT, INS_GE, ALGO_COUNT };

static const char* ALGO_NAME[ALGO_COUNT] = {
    "Selection Sort",
    "Bubble Sort",
    "Bubble Sort (Early Termination)",
    "Insertion Sort (>)",
    "Insertion Sort (>=)"
};

static void run_sort(int algo, Data* a, int n, Counter* c)
{
    c->cmp = 0; c->mov = 0;
    switch (algo) {
    case SEL:    selection_sort(a, n, c);        break;
    case BUB:    bubble_sort(a, n, c, 0);        break;
    case BUB_ES: bubble_sort(a, n, c, 1);        break;
    case INS_GT: insertion_sort(a, n, c, 0);     break;
    case INS_GE: insertion_sort(a, n, c, 1);     break;
    }
}

static int is_sorted(const Data* a, int n)
{
    for (int i = 0; i < n - 1; i++)
        if (a[i].key1 > a[i + 1].key1) return 0;
    return 1;
}

static int stability_violations(const Data* a, int n)
{
    int v = 0;
    for (int i = 0; i < n - 1; i++)
        if (a[i].key1 == a[i + 1].key1 && a[i].order > a[i + 1].order)
            v++;
    return v;
}

static int duplicate_adjacent_pairs(const Data* a, int n)
{
    int d = 0;
    for (int i = 0; i < n - 1; i++)
        if (a[i].key1 == a[i + 1].key1) d++;
    return d;
}

static int rand_range(int max)
{
    return rand() % (max + 1);
}

static void generate_random(Data* a, int n, int max_key1)
{
    for (int i = 0; i < n; i++) {
        a[i].key1 = rand_range(max_key1);
        a[i].key2 = rand_range(MAX_KEY);
        a[i].order = i;
    }
}

static int cmp_prep(const void* x, const void* y)
{
    const Data* p = x, * q = y;
    if (p->key1 != q->key1) return (p->key1 > q->key1) - (p->key1 < q->key1);
    return (p->order > q->order) - (p->order < q->order);
}

static void make_sorted(Data* dst, const Data* src, int n)
{
    memcpy(dst, src, sizeof(Data) * n);
    qsort(dst, n, sizeof(Data), cmp_prep);
}

static void make_reverse(Data* dst, const Data* sorted, int n)
{
    for (int i = 0; i < n; i++)
        dst[i] = sorted[n - 1 - i];
}

static void make_partial(Data* dst, const Data* sorted, int n, int swaps)
{
    memcpy(dst, sorted, sizeof(Data) * n);
    for (int k = 0; k < swaps; k++) {
        int i = rand() % n, j = rand() % n;
        Data t = dst[i]; dst[i] = dst[j]; dst[j] = t;
    }
}

static void print_line(void)
{
    printf("\n------------------------------------\n\n");
}

static void run_state(const char* title, const Data* input, int n)
{
    static const int algos[4] = { SEL, BUB, BUB_ES, INS_GT };
    Data* work = malloc(sizeof(Data) * n);
    if (!work) { perror("malloc"); exit(1); }

    printf("%s\n", title);
    for (int k = 0; k < 4; k++) {
        Counter c;
        memcpy(work, input, sizeof(Data) * n);
        run_sort(algos[k], work, n, &c);
        printf("\n%s\n", ALGO_NAME[algos[k]]);
        printf("Comparisons : %lld\n", c.cmp);
        printf("Movements   : %lld\n", c.mov);
        printf("Sorted Check: %s\n", is_sorted(work, n) ? "OK" : "FAIL");
    }
    free(work);
}

static void print_small(const char* label, const Data* a, int n)
{
    printf("  %-26s:", label);
    for (int i = 0; i < n; i++)
        printf(" (%d,%d)", a[i].key1, a[i].key2);
    printf("\n");
}

static void stability_verification(void)
{
    Data ex[4] = {
        {365, 5, 0}, {365, 10, 1}, {103, 40, 2}, {726, 80, 3}
    };
    printf("Stability Verification\n\n");
    printf("[Small Example] Original: (365,5) (365,10) (103,40) (726,80)\n");
    for (int algo = 0; algo < ALGO_COUNT; algo++) {
        Data t[4]; Counter c;
        memcpy(t, ex, sizeof(ex));
        run_sort(algo, t, 4, &c);
        print_small(ALGO_NAME[algo], t, 4);
    }

    const int N = 1000;
    Data* base = malloc(sizeof(Data) * N);
    Data* work = malloc(sizeof(Data) * N);
    if (!base || !work) { perror("malloc"); exit(1); }
    generate_random(base, N, 99);

    printf("\n[Large Test] %d items, Key 1 in 0..99 (many duplicates)\n", N);
    printf("  Input duplicate adjacent pairs after sorting: ");
    {
        Data* s = malloc(sizeof(Data) * N);
        make_sorted(s, base, N);
        printf("%d\n\n", duplicate_adjacent_pairs(s, N));
        free(s);
    }

    static const int algos[5] = { SEL, BUB, BUB_ES, INS_GT, INS_GE };
    for (int k = 0; k < 5; k++) {
        Counter c;
        memcpy(work, base, sizeof(Data) * N);
        run_sort(algos[k], work, N, &c);
        int v = stability_violations(work, N);
        printf("%-34s: %s (sorted=%s, order violations=%d)\n",
            ALGO_NAME[algos[k]],
            v == 0 ? "Stable" : "Unstable",
            is_sorted(work, N) ? "OK" : "FAIL", v);
    }

    printf("\nStability Verification (Summary)\n");
    {
        static const int sel[4] = { SEL, BUB, INS_GT, INS_GE };
        static const char* lbl[4] = { "Selection Sort      ", "Bubble Sort         ",
                                      "Insertion Sort (>)  ", "Insertion Sort (>=) " };
        for (int k = 0; k < 4; k++) {
            Counter c;
            memcpy(work, base, sizeof(Data) * N);
            run_sort(sel[k], work, N, &c);
            printf("%s: %s\n", lbl[k],
                stability_violations(work, N) == 0 ? "Stable" : "Unstable");
        }
    }
    free(base);
    free(work);
}

int main(void)
{
    srand(SEED);

    Data* base = malloc(sizeof(Data) * DATA_COUNT);
    Data* work = malloc(sizeof(Data) * DATA_COUNT);
    if (!base || !work) { perror("malloc"); return 1; }
    
    long long tot_cmp[4] = { 0 }, tot_mov[4] = { 0 };
    static const int algos[4] = { SEL, BUB, BUB_ES, INS_GT };
    int all_sorted = 1;

    for (int t = 0; t < TRIALS; t++) {
        generate_random(base, DATA_COUNT, MAX_KEY);
        for (int k = 0; k < 4; k++) {
            Counter c;
            memcpy(work, base, sizeof(Data) * DATA_COUNT);
            run_sort(algos[k], work, DATA_COUNT, &c);
            tot_cmp[k] += c.cmp;
            tot_mov[k] += c.mov;
            if (!is_sorted(work, DATA_COUNT)) all_sorted = 0;
        }
    }

    printf("Data Count : %d\n\n", DATA_COUNT);
    printf("Random Data\n");
    printf("Number of Trials : %d\n", TRIALS);
    for (int k = 0; k < 4; k++) {
        printf("\n%s\n", ALGO_NAME[algos[k]]);
        printf("Total Comparisons   : %lld\n", tot_cmp[k]);
        printf("Average Comparisons : %.2f\n", (double)tot_cmp[k] / TRIALS);
        printf("Average Movements   : %.2f\n", (double)tot_mov[k] / TRIALS);
    }
    printf("\nSorted Check (all %d trials, all algorithms): %s\n",
        TRIALS, all_sorted ? "OK" : "FAIL");
    
    Data* orig = malloc(sizeof(Data) * DATA_COUNT);
    Data* sorted = malloc(sizeof(Data) * DATA_COUNT);
    Data* reverse = malloc(sizeof(Data) * DATA_COUNT);
    Data* partial = malloc(sizeof(Data) * DATA_COUNT);
    if (!orig || !sorted || !reverse || !partial) { perror("malloc"); return 1; }

    generate_random(orig, DATA_COUNT, MAX_KEY);
    make_sorted(sorted, orig, DATA_COUNT);
    make_reverse(reverse, sorted, DATA_COUNT);
    make_partial(partial, sorted, DATA_COUNT, PARTIAL_SWAPS);

    print_line();
    run_state("Sorted Data", sorted, DATA_COUNT);
    print_line();
    run_state("Reverse-Sorted Data", reverse, DATA_COUNT);
    print_line();
    printf("(Partially sorted: sorted data + %d random pair swaps)\n", PARTIAL_SWAPS);
    run_state("Partially Sorted Data", partial, DATA_COUNT);
    print_line();

    stability_verification();

    free(base); free(work);
    free(orig); free(sorted); free(reverse); free(partial);
    return 0;
}