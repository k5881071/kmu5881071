#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define V        50
#define MAX_W    100
#define REPEAT   2000
#define E_SPARSE 100
#define E_DENSE  600

typedef struct { int u, v, w; } Edge;


static int randRange(int lo, int hi) {
    return lo + rand() % (hi - lo + 1);
}

static void generateGraph(Edge* edges, int numEdges) {
    static bool used[V][V];
    int perm[V], cnt = 0;

    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++) used[i][j] = false;

    for (int i = 0; i < V; i++) perm[i] = i;
    for (int i = V - 1; i > 0; i--) {
        int j = randRange(0, i);
        int t = perm[i]; perm[i] = perm[j]; perm[j] = t;
    }

    for (int i = 1; i < V; i++) {
        int a = perm[i];
        int b = perm[randRange(0, i - 1)];
        edges[cnt++] = (Edge){ a, b, randRange(1, MAX_W) };
        used[a][b] = used[b][a] = true;
    }

    while (cnt < numEdges) {
        int a = randRange(0, V - 1);
        int b = randRange(0, V - 1);
        if (a == b || used[a][b]) continue;
        edges[cnt++] = (Edge){ a, b, randRange(1, MAX_W) };
        used[a][b] = used[b][a] = true;
    }
}

static bool isConnected(const Edge* edges, int numEdges) {
    static bool adj[V][V];
    bool vis[V] = { false };
    int stack[V], top = 0, count = 1;

    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++) adj[i][j] = false;
    for (int i = 0; i < numEdges; i++)
        adj[edges[i].u][edges[i].v] = adj[edges[i].v][edges[i].u] = true;

    stack[top++] = 0;
    vis[0] = true;
    while (top > 0) {
        int x = stack[--top];
        for (int y = 0; y < V; y++)
            if (adj[x][y] && !vis[y]) { vis[y] = true; count++; stack[top++] = y; }
    }
    return count == V;
}

typedef struct Node {
    int to, w;
    struct Node* next;
} Node;

typedef struct {
    Node* head[V];
    int nodeCount;
} AdjList;

static void pushNode(AdjList* g, int from, int to, int w) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->to = to; n->w = w; n->next = g->head[from];
    g->head[from] = n;
    g->nodeCount++;
}

static void buildAdjList(AdjList* g, const Edge* edges, int numEdges) {
    for (int i = 0; i < V; i++) g->head[i] = NULL;
    g->nodeCount = 0;
    for (int i = 0; i < numEdges; i++) {
        pushNode(g, edges[i].u, edges[i].v, edges[i].w);
        pushNode(g, edges[i].v, edges[i].u, edges[i].w);
    }
}

static void freeAdjList(AdjList* g) {
    for (int i = 0; i < V; i++) {
        Node* p = g->head[i];
        while (p) { Node* t = p->next; free(p); p = t; }
        g->head[i] = NULL;
    }
}

typedef struct {
    int parent[V];
    int rnk[V];
} DSU;

static void dsuInit(DSU* d) {
    for (int i = 0; i < V; i++) { d->parent[i] = i; d->rnk[i] = 0; }
}

static int dsuFind(DSU* d, int x) {
    if (d->parent[x] != x) d->parent[x] = dsuFind(d, d->parent[x]);
    return d->parent[x];
}

static bool dsuUnion(DSU* d, int a, int b) {
    a = dsuFind(d, a); b = dsuFind(d, b);
    if (a == b) return false;
    if (d->rnk[a] < d->rnk[b]) { int t = a; a = b; b = t; }
    d->parent[b] = a;
    if (d->rnk[a] == d->rnk[b]) d->rnk[a]++;
    return true;
}

static int cmpEdge(const void* a, const void* b) {
    return ((const Edge*)a)->w - ((const Edge*)b)->w;
}

static long long kruskal(const AdjList* g, Edge* edgeBuf, Edge* mst, int* mstCnt) {
    int m = 0;

    for (int u = 0; u < V; u++)
        for (Node* p = g->head[u]; p; p = p->next)
            if (u < p->to) edgeBuf[m++] = (Edge){ u, p->to, p->w };

    qsort(edgeBuf, m, sizeof(Edge), cmpEdge);

    DSU dsu;
    dsuInit(&dsu);
    long long total = 0;
    *mstCnt = 0;
    for (int i = 0; i < m && *mstCnt < V - 1; i++) {
        if (dsuUnion(&dsu, edgeBuf[i].u, edgeBuf[i].v)) {
            mst[(*mstCnt)++] = edgeBuf[i];
            total += edgeBuf[i].w;
        }
    }
    return total;
}

typedef struct { int w[V][V]; } AdjMatrix;

static void buildAdjMatrix(AdjMatrix* g, const Edge* edges, int numEdges) {
    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++) g->w[i][j] = 0;
    for (int i = 0; i < numEdges; i++)
        g->w[edges[i].u][edges[i].v] = g->w[edges[i].v][edges[i].u] = edges[i].w;
}

static long long prim(const AdjMatrix* g, int* key, int* par, bool* vis,
    Edge* mst, int* mstCnt) {
    const int INF = 1 << 30;
    for (int i = 0; i < V; i++) { key[i] = INF; par[i] = -1; vis[i] = false; }
    key[0] = 0;

    long long total = 0;
    *mstCnt = 0;
    for (int it = 0; it < V; it++) {
        int u = -1;
        for (int i = 0; i < V; i++)
            if (!vis[i] && (u == -1 || key[i] < key[u])) u = i;
        vis[u] = true;
        if (par[u] != -1) {
            mst[(*mstCnt)++] = (Edge){ par[u], u, key[u] };
            total += key[u];
        }
        for (int v = 0; v < V; v++)
            if (g->w[u][v] && !vis[v] && g->w[u][v] < key[v]) {
                key[v] = g->w[u][v];
                par[v] = u;
            }
    }
    return total;
}

typedef struct {
    long long cost;
    int cnt;
    size_t mem;
    double ms;
} Result;

static double elapsedMs(struct timespec a, struct timespec b) {
    return (double)(b.tv_sec - a.tv_sec) * 1000.0 +
        (double)(b.tv_nsec - a.tv_nsec) / 1e6;
}

static Result runKruskal(const Edge* edges, int numEdges, Edge* mst) {
    AdjList g;
    buildAdjList(&g, edges, numEdges);
    Edge* edgeBuf = (Edge*)malloc(sizeof(Edge) * numEdges);

    size_t mem = sizeof(g.head) + (size_t)g.nodeCount * sizeof(Node)
        + (size_t)numEdges * sizeof(Edge) + sizeof(DSU)
        + (size_t)(V - 1) * sizeof(Edge);

    long long cost = 0;
    int cnt = 0;
    struct timespec t0, t1;
    timespec_get(&t0, TIME_UTC);
    for (int r = 0; r < REPEAT; r++) cost = kruskal(&g, edgeBuf, mst, &cnt);
    timespec_get(&t1, TIME_UTC);

    free(edgeBuf);
    freeAdjList(&g);
    return (Result) { cost, cnt, mem, elapsedMs(t0, t1) / REPEAT };
}

static Result runPrim(const Edge* edges, int numEdges, Edge* mst) {
    static AdjMatrix g;
    int key[V], par[V];
    bool vis[V];
    buildAdjMatrix(&g, edges, numEdges);

    size_t mem = sizeof(g) + sizeof(key) + sizeof(par) + sizeof(vis)
        + (size_t)(V - 1) * sizeof(Edge);

    long long cost = 0;
    int cnt = 0;
    struct timespec t0, t1;
    timespec_get(&t0, TIME_UTC);
    for (int r = 0; r < REPEAT; r++) cost = prim(&g, key, par, vis, mst, &cnt);
    timespec_get(&t1, TIME_UTC);

    return (Result) { cost, cnt, mem, elapsedMs(t0, t1) / REPEAT };
}

static void printMST(const Edge* mst, int cnt) {
    printf("MST Edge List (u - v : w)\n");
    for (int i = 0; i < cnt; i++)
        printf("  (%2d - %2d : %3d)%s", mst[i].u, mst[i].v, mst[i].w,
            (i % 4 == 3) ? "\n" : "");
    if (cnt % 4) printf("\n");
}

static void report(int caseNo, const char* title, int E, const Result* r) {
    printf("Case %d : %s\n", caseNo, title);
    printf("Vertices      : %d\n", V);
    printf("Edges         : %d\n", E);
    printf("MST Edges     : %d\n", r->cnt);
    printf("MST Cost      : %lld\n", r->cost);
    printf("Memory        : %zu Bytes\n", r->mem);
    printf("Execution Time: %.6f ms\n\n", r->ms);
}

int main(void) {
    static Edge sparse[E_SPARSE], dense[E_DENSE];
    Edge mst[V - 1];

    srand((unsigned)time(NULL));

    do { generateGraph(sparse, E_SPARSE); } while (!isConnected(sparse, E_SPARSE));
    do { generateGraph(dense, E_DENSE); } while (!isConnected(dense, E_DENSE));

    Result r1 = runKruskal(sparse, E_SPARSE, mst);
    report(1, "Sparse Graph - Adjacency List - Kruskal", E_SPARSE, &r1);
    printMST(mst, r1.cnt); printf("\n");

    Result r2 = runKruskal(dense, E_DENSE, mst);
    report(2, "Dense Graph - Adjacency List - Kruskal", E_DENSE, &r2);
    printMST(mst, r2.cnt); printf("\n");

    Result r3 = runPrim(sparse, E_SPARSE, mst);
    report(3, "Sparse Graph - Adjacency Matrix - Prim", E_SPARSE, &r3);
    printMST(mst, r3.cnt); printf("\n");

    Result r4 = runPrim(dense, E_DENSE, mst);
    report(4, "Dense Graph - Adjacency Matrix - Prim", E_DENSE, &r4);
    printMST(mst, r4.cnt); printf("\n");

    bool ok = r1.cnt == V - 1 && r2.cnt == V - 1 && r3.cnt == V - 1 && r4.cnt == V - 1
        && r1.cost == r3.cost && r2.cost == r4.cost;
    printf("[Check] MST edge count = 49 and Kruskal/Prim costs match: %s\n",
        ok ? "PASS" : "FAIL");
    printf("(Execution time = average of %d runs per case)\n", REPEAT);
    return ok ? 0 : 1;
}