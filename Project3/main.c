#include <stdio.h>

#define MAXLEN    1024
#define MAXSTACK   128
#define MAXQUEUE    64

#define TARGET 'C'

int Upper(char c) { return (c >= 'A' && c <= 'Z');}

int Space(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' ||
        c == '\r' || c == '\v' || c == '\f');
}

typedef struct {
    char item[MAXSTACK];
    int  top;
} NodeStack;

void initNodeStack(NodeStack* s) { s->top = -1; }
int  isEmptyNodeStack(NodeStack* s) { return s->top == -1; }
int  isFullNodeStack(NodeStack* s) { return s->top == MAXSTACK - 1; }
void pushNode(NodeStack* s, char x) { s->item[++(s->top)] = x; }
char popNode(NodeStack* s) { return s->item[(s->top)--]; }
char peekNode(NodeStack* s) { return s->item[s->top]; }

typedef struct {
    int item[MAXSTACK];
    int top;
} CounterStack;

void initCounterStack(CounterStack* s) { s->top = -1; }
int  isEmptyCounterStack(CounterStack* s) { return s->top == -1; }
void pushCounter(CounterStack* s, int x) { s->item[++(s->top)] = x; }
int  popCounter(CounterStack* s) { return s->item[(s->top)--]; }
void incCounter(CounterStack* s) { s->item[s->top]++; }

typedef struct {
    char item[MAXQUEUE];
    int  front;
    int  rear;
} Queue;

void initQueue(Queue* q) { q->front = q->rear = -1; }
int  isEmptyQueue(Queue* q) { return q->front == q->rear; }
int  isFullQueue(Queue* q) { return q->rear == MAXQUEUE - 1; }
void enqueue(Queue* q, char x)
{
    if (!isFullQueue(q)) q->item[++(q->rear)] = x;
}
char dequeue(Queue* q) { return q->item[++(q->front)]; }

void drawTree(const char* e)
{
    int depth = 0, i, k;

    for (i = 0; e[i] != '\0'; i++) {
        if (Upper(e[i])) {
            for (k = 0; k < 4 * (depth - 1); k++) printf(" ");
            if (depth > 0) printf("+---");
            printf("%c\n", e[i]);
        }
        else if (e[i] == '(') depth++;
        else if (e[i] == ')') depth--;
    }
}

int main(void)
{
    NodeStack    nstack;
    CounterStack cstack;
    Queue        cqueue;

    char buf[MAXLEN], expr[MAXLEN];
    int  i, len = 0;

    int  total = 0;
    int  nonleaf = 0;
    int  maxLevel = 1;
    int  degree = 0;
    int  depth = 0;
    int  used = 0;

    char parent = 0;
    int  targetFound = 0;
    int  expectNode = 0;
    int  justClosed = 0;

    initNodeStack(&nstack);
    initCounterStack(&cstack);
    initQueue(&cqueue);

    printf("트리를 괄호 표기법으로 입력하세요 : ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        printf("잘못된 입력입니다.\n");
        return 1;
    }
    for (i = 0; buf[i] != '\0'; i++)
        if (!Space(buf[i]))
            expr[len++] = buf[i];
    expr[len] = '\0';

    if (len == 0) {
        printf("잘못된 입력입니다.\n\n");
        return 1;
    }

    for (i = 0; i < len; i++) {
        char c = expr[i];

        if (expectNode == 0) {
            if (!Upper(c)) {
                printf("잘못된 입력입니다.\n\n",
                    i + 1, c);
                return 1;
            }
            if (used & (1 << (c - 'A'))) {
                printf("잘못된 입력입니다.\n");
                return 1;
            }
            used |= (1 << (c - 'A'));
            total++;
            if (depth + 1 > maxLevel) maxLevel = depth + 1;

            if (!isEmptyNodeStack(&nstack)) {
                incCounter(&cstack);
                if (peekNode(&nstack) == TARGET)
                    enqueue(&cqueue, c);
            }
            if (c == TARGET) {
                targetFound = 1;
                parent = isEmptyNodeStack(&nstack) ? 0 : peekNode(&nstack);
            }
            expectNode = 1;
            justClosed = 0;
        }
        else {
            if (c == '(') {
                if (justClosed) {
                    printf("잘못된 입력입니다.\n",
                        i + 1);
                    return 1;
                }
                if (isFullNodeStack(&nstack)) {
                    printf("잘못된 입력입니다.\n\n");
                    return 1;
                }
                pushNode(&nstack, expr[i - 1]);
                pushCounter(&cstack, 0);
                nonleaf++;
                depth++;
                expectNode = 0;
            }
            else if (c == ',') {
                if (depth == 0) {
                    printf("잘못된 입력입니다.\n");
                    return 1;
                }
                expectNode = 0;
            }
            else if (c == ')') {
                int children;
                if (depth == 0) {
                    printf("잘못된 입력입니다.\n");
                    return 1;
                }
                popNode(&nstack);
                children = popCounter(&cstack);
                if (children > degree) degree = children;
                depth--;
                justClosed = 1;
            }
            else {
                printf("잘못된 입력입니다.\n");
                return 1;
            }
        }
    }

    if (expectNode == 0) {
        printf("잘못된 입력입니다.\n");
        return 1;
    }
    if (depth != 0) {
        printf("잘못된 입력입니다.\n");
        return 1;
    }
    if (used != ((1 << total) - 1)) {
        printf("잘못된 입력입니다.\n");
        return 1;
    }

    printf("\n입력된 트리 : %s\n", expr);
    printf("------------------------------------\n");
    printf("전체 노드의 수   : %d\n", total);
    printf("단말 노드의 수   : %d\n", total - nonleaf);
    printf("비단말 노드의 수 : %d\n", nonleaf);
    printf("트리의 높이      : %d\n", maxLevel);
    printf("트리의 차수      : %d\n", degree);

    if (!targetFound) {
        printf("노드 %c 의 부모   : 노드 %c 가 트리에 없습니다.\n", TARGET, TARGET);
        printf("노드 %c 의 자식   : 노드 %c 가 트리에 없습니다.\n", TARGET, TARGET);
    }
    else {
        if (parent == 0) printf("노드 %c 의 부모   : 없음 (루트 노드)\n", TARGET);
        else             printf("노드 %c 의 부모   : %c\n", TARGET, parent);

        printf("노드 %c 의 자식   : ", TARGET);
        if (isEmptyQueue(&cqueue)) printf("없음 (단말 노드)\n");
        else {
            while (!isEmptyQueue(&cqueue)) {
                printf("%c", dequeue(&cqueue));
                if (!isEmptyQueue(&cqueue)) printf(", ");
            }
            printf("\n");
        }
    }

    printf("------------------------------------\n");
    printf("트리 구조\n");
    drawTree(expr);
    return 0;
}