#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define CSV_FILE   "dataset_id_ascending.csv"
#define MAX_LINE   1024
#define FIELD_LEN  64

typedef struct {
    char id[FIELD_LEN];
    char name[FIELD_LEN];
    char gender[FIELD_LEN];
    int  korean, english, math, total;
} Student;

static long long g_cmp = 0;

static int name_cmp(const char* a, const char* b)
{
    g_cmp++;
    return strcmp(a, b);
}

static void trim(char* s)
{
    char* p = s;
    size_t n;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    if (n >= 2 && s[0] == '"' && s[n - 1] == '"') {
        memmove(s, s + 1, n - 2);
        s[n - 2] = '\0';
    }
}

static int is_number(const char* s)
{
    if (!*s) return 0;
    if (*s == '-' || *s == '+') s++;
    if (!*s) return 0;
    for (; *s; s++) if (!isdigit((unsigned char)*s)) return 0;
    return 1;
}

static int split_csv(char* line, char* f[], int max)
{
    int n = 0;
    char* p = line;
    while (n < max) {
        f[n++] = p;
        p = strchr(p, ',');
        if (!p) break;
        *p++ = '\0';
    }
    return n;
}

static int parse_line(char* line, int is_first_line, Student* st)
{
    char* f[8];
    int i;

    if (is_first_line &&
        (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB &&
        (unsigned char)line[2] == 0xBF)
        line += 3;

    if (split_csv(line, f, 8) < 6) return 0;
    for (i = 0; i < 6; i++) trim(f[i]);
    if (!is_number(f[3]) || !is_number(f[4]) || !is_number(f[5]))
        return 0;

    if (st) {
        strncpy(st->id, f[0], FIELD_LEN - 1); st->id[FIELD_LEN - 1] = '\0';
        strncpy(st->name, f[1], FIELD_LEN - 1); st->name[FIELD_LEN - 1] = '\0';
        strncpy(st->gender, f[2], FIELD_LEN - 1); st->gender[FIELD_LEN - 1] = '\0';
        st->korean = atoi(f[3]);
        st->english = atoi(f[4]);
        st->math = atoi(f[5]);
        st->total = st->korean + st->english + st->math;
    }
    return 1;
}

static Student* load_csv(const char* path, int* count)
{
    FILE* fp = fopen(path, "r");
    char line[MAX_LINE];
    int n = 0, idx = 0, line_no;
    Student* arr;

    if (!fp) { fprintf(stderr, "CSV 파일을 열 수 없습니다: %s\n", path); return NULL; }

    for (line_no = 0; fgets(line, sizeof(line), fp); line_no++)
        if (parse_line(line, line_no == 0, NULL)) n++;
    if (n == 0) { fclose(fp); *count = 0; return NULL; }

    arr = (Student*)malloc(sizeof(Student) * n);
    if (!arr) { fclose(fp); fprintf(stderr, "메모리 할당 실패\n"); return NULL; }

    rewind(fp);
    for (line_no = 0; fgets(line, sizeof(line), fp) && idx < n; line_no++)
        if (parse_line(line, line_no == 0, &arr[idx])) idx++;

    fclose(fp);
    *count = idx;
    return arr;
}

static int sequential_search(const Student* a, int n, const char* key, int* out)
{
    int i, m = 0;
    for (i = 0; i < n; i++)
        if (name_cmp(a[i].name, key) == 0)
            out[m++] = i;
    return m;
}

static void swap_st(Student* x, Student* y)
{
    Student t = *x; *x = *y; *y = t;
}

static void quick_sort(Student* a, int lo, int hi)
{
    while (lo < hi) {
        char pivot[FIELD_LEN];
        int i = lo, j = hi;
        strcpy(pivot, a[lo + (hi - lo) / 2].name);

        while (i <= j) {
            while (name_cmp(a[i].name, pivot) < 0) i++;
            while (name_cmp(a[j].name, pivot) > 0) j--;
            if (i <= j) { swap_st(&a[i], &a[j]); i++; j--; }
        }
        if (j - lo < hi - i) { quick_sort(a, lo, j); lo = i; }
        else { quick_sort(a, i, hi); hi = j; }
    }
}

static int binary_search(const Student* a, int n, const char* key, int* out)
{
    int lo = 0, hi = n - 1, mid, found = -1, l, r, m = 0, i;

    while (lo <= hi) {
        int c;
        mid = lo + (hi - lo) / 2;
        c = name_cmp(a[mid].name, key);
        if (c == 0) { found = mid; break; }
        if (c < 0) lo = mid + 1;
        else       hi = mid - 1;
    }
    if (found < 0) return 0;

    l = found - 1;
    while (l >= 0 && name_cmp(a[l].name, key) == 0) l--;
    r = found + 1;
    while (r < n && name_cmp(a[r].name, key) == 0) r++;

    for (i = l + 1; i < r; i++) out[m++] = i;
    return m;
}

static void print_result(const Student* a, const int* idx, int m, const char* key)
{
    int i;
    printf("Search Result\n");
    if (m == 0) { printf("Name    : %s (검색 결과 없음)\n\n", key); return; }
    for (i = 0; i < m; i++) {
        const Student* s = &a[idx[i]];
        printf("ID      : %s\n", s->id);
        printf("Name    : %s\n", s->name);
        printf("Korean  : %d\n", s->korean);
        printf("English : %d\n", s->english);
        printf("Math    : %d\n", s->math);
        printf("Total   : %d\n", s->total);
        if (i < m - 1) printf("\n");
    }
    printf("\n");
}

static int same_result(const Student* a, const int* ia, int ma,
    const Student* b, const int* ib, int mb)
{
    int i, j;
    if (ma != mb) return 0;
    for (i = 0; i < ma; i++) {
        int ok = 0;
        for (j = 0; j < mb; j++)
            if (strcmp(a[ia[i]].id, b[ib[j]].id) == 0) { ok = 1; break; }
        if (!ok) return 0;
    }
    return 1;
}
static unsigned int rnd(void)
{
    return ((unsigned int)rand() << 15) | (unsigned int)rand();
}

int main(int argc, char* argv[])
{
    int n = 0;
    Student* orig = load_csv(CSV_FILE, &n);
    Student* sorted;
    int* idxS, * idxB;
    long long qs_cost;

    if (!orig || n == 0) { fprintf(stderr, "읽은 학생 데이터가 없습니다.\n"); return 1; }
    printf("학생 수: %d명\n", n);

    srand((unsigned int)time(NULL));

    printf("\n");

    idxS = (int*)malloc(sizeof(int) * n);
    idxB = (int*)malloc(sizeof(int) * n);

    sorted = (Student*)malloc(sizeof(Student) * n);
    memcpy(sorted, orig, sizeof(Student) * n);
    g_cmp = 0;
    quick_sort(sorted, 0, n - 1);
    qs_cost = g_cmp;

    {
        const char* demo = (argc > 1) ? argv[1] : orig[0].name;
        int ms, mb;
        g_cmp = 0; ms = sequential_search(orig, n, demo, idxS);
        g_cmp = 0; mb = binary_search(sorted, n, demo, idxB);

        printf("[순차 탐색 결과]\n");
        print_result(orig, idxS, ms, demo);
        printf("[이진 탐색 결과]\n");
        print_result(sorted, idxB, mb, demo);
        printf("두 방법의 결과 일치 여부: %s\n\n",
            same_result(orig, idxS, ms, sorted, idxB, mb) ? "일치" : "불일치!");
    }

    {
        const int Ks[] = { 1, 10,20,30,40,50,60,70,80,90,100,200,400,800,1600,2000};
        const int NK = (int)(sizeof(Ks) / sizeof(Ks[0]));
        const int KMAX = Ks[NK - 1];
        char (*list)[FIELD_LEN] = (char (*)[FIELD_LEN])malloc((size_t)KMAX * FIELD_LEN);
        long long seqTotal = 0, binTotal = 0;
        int k, ki = 0, breakeven = -1, mismatch = 0;

        for (k = 0; k < KMAX; k++) {
            if (rnd() % 10 < 8)
                strcpy(list[k], orig[rnd() % n].name);
            else
                sprintf(list[k], "NOTFOUND_%d", k);
        }

        printf("퀵 정렬 비교 횟수 (1회): %lld\n\n", qs_cost);
        printf("Comparison Results\n\n");
        printf("%-8s %-16s %-16s %s\n", "K", "Sequential", "Quick + Binary", "Better");
        printf("----------------------------------------------------\n");

        binTotal = qs_cost;
        for (k = 1; k <= KMAX; k++) {
            int ms, mb;

            g_cmp = 0; ms = sequential_search(orig, n, list[k - 1], idxS); seqTotal += g_cmp;
            g_cmp = 0; mb = binary_search(sorted, n, list[k - 1], idxB); binTotal += g_cmp;

            if (!same_result(orig, idxS, ms, sorted, idxB, mb)) mismatch++;
            if (breakeven < 0 && binTotal < seqTotal) breakeven = k;

            if (k == Ks[ki]) {
                printf("%-8d %-16lld %-16lld %s\n", k, seqTotal, binTotal,
                    binTotal < seqTotal ? "Quick + Binary" : "Sequential");
                if (++ki >= NK) break;
            }
        }
        printf("\n");
        if (breakeven > 0)
            printf("K = %d 부터 (정렬 비용 포함) 퀵 정렬 + 이진 탐색이 순차 탐색보다 유리해집니다.\n", breakeven);
        else
            printf("K <= %d 범위에서는 순차 탐색이 계속 유리합니다.\n", KMAX);
        if (mismatch) printf("경고: 두 방법의 검색 결과가 다른 경우가 %d회 있었습니다.\n", mismatch);
        else          printf("모든 검색에서 두 방법의 결과가 일치했습니다.\n");
        free(list);
    }

    free(orig); free(sorted); free(idxS); free(idxB);
    return 0;
}