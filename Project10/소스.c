#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define V 100
#define MAX_E 4000

typedef struct {
    int m[V][V];
} AdjMatrix;

typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

typedef struct {
    Node* head[V];
    size_t nodeCount;
} AdjList;

typedef struct { int u, v; } Edge;

static void mat_init(AdjMatrix* g, const Edge* e, int cnt) {
    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++) g->m[i][j] = 0;
    for (int i = 0; i < cnt; i++) {
        g->m[e[i].u][e[i].v] = 1;
        g->m[e[i].v][e[i].u] = 1;
    }
}

static size_t mat_memory(const AdjMatrix* g) { (void)g; return sizeof(AdjMatrix); }

static int mat_has(const AdjMatrix* g, int u, int v, long* cmp) {
    (*cmp)++;
    return g->m[u][v];
}

static long mat_insert(AdjMatrix* g, int u, int v) {
    long cmp = 0;
    if (mat_has(g, u, v, &cmp)) return cmp;
    g->m[u][v] = 1;
    g->m[v][u] = 1;
    return cmp;
}

static long mat_delete(AdjMatrix* g, int u, int v) {
    long cmp = 0;
    if (!mat_has(g, u, v, &cmp)) return cmp;
    g->m[u][v] = 0;
    g->m[v][u] = 0;
    return cmp;
}

static long mat_neighbors(const AdjMatrix* g, int u, int* out, int* n) {
    long cmp = 0;
    *n = 0;
    for (int j = 0; j < V; j++) {
        cmp++;
        if (g->m[u][j]) out[(*n)++] = j;
    }
    return cmp;
}

static void list_push_head(AdjList* g, int u, int v) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) { perror("malloc"); exit(1); }
    n->vertex = v;
    n->next = g->head[u];
    g->head[u] = n;
    g->nodeCount++;
}

static void list_init(AdjList* g, const Edge* e, int cnt) {
    for (int i = 0; i < V; i++) g->head[i] = NULL;
    g->nodeCount = 0;
    for (int i = 0; i < cnt; i++) {
        list_push_head(g, e[i].u, e[i].v);
        list_push_head(g, e[i].v, e[i].u);
    }
}

static void list_free(AdjList* g) {
    for (int i = 0; i < V; i++) {
        Node* p = g->head[i];
        while (p) { Node* t = p->next; free(p); p = t; }
        g->head[i] = NULL;
    }
    g->nodeCount = 0;
}

static size_t list_memory(const AdjList* g) {
    return sizeof(AdjList) + g->nodeCount * sizeof(Node);
}

static int list_find(const AdjList* g, int u, int v, long* cmp) {
    for (Node* p = g->head[u]; p; p = p->next) {
        (*cmp)++;
        if (p->vertex == v) return 1;
    }
    return 0;
}

static long list_insert(AdjList* g, int u, int v) {
    long cmp = 0;
    if (list_find(g, u, v, &cmp)) return cmp;
    list_push_head(g, u, v);
    list_push_head(g, v, u);
    return cmp;
}

static int list_remove_node(AdjList* g, int u, int v, long* cmp) {
    Node* prev = NULL;
    for (Node* p = g->head[u]; p; prev = p, p = p->next) {
        (*cmp)++;
        if (p->vertex == v) {
            if (prev) prev->next = p->next;
            else g->head[u] = p->next;
            free(p);
            g->nodeCount--;
            return 1;
        }
    }
    return 0;
}

static long list_delete(AdjList* g, int u, int v) {
    long cmp = 0;
    list_remove_node(g, u, v, &cmp);
    list_remove_node(g, v, u, &cmp);
    return cmp;
}

static long list_neighbors(const AdjList* g, int u, int* out, int* n) {
    long cmp = 0;
    *n = 0;
    for (Node* p = g->head[u]; p; p = p->next) {
        cmp++;
        out[(*n)++] = p->vertex;
    }
    return cmp;
}

static int exists_tmp[V][V];

static int generate_graph(Edge* edges, int target) {
    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++) exists_tmp[i][j] = 0;
    int cnt = 0;
    while (cnt < target) {
        int u = rand() % V, v = rand() % V;
        if (u == v || exists_tmp[u][v]) continue;
        exists_tmp[u][v] = exists_tmp[v][u] = 1;
        edges[cnt].u = u;
        edges[cnt].v = v;
        cnt++;
    }
    return cnt;
}

typedef struct {
    size_t mem;
    long ins, del, conn, adj;
    int nb[V], nbCnt;
} Result;

static void print_result(int caseNo, const char* graphName, const char* repName, const Result* r, int adjV) {
    printf("Case %d : %s Graph - %s\n", caseNo, graphName, repName);
    printf("Memory                         : %zu Bytes\n", r->mem);
    printf("Edge insertion comparisons     : %ld\n", r->ins);
    printf("Edge deletion comparisons      : %ld\n", r->del);
    printf("Connectivity check comparisons : %ld\n", r->conn);
    printf("Adjacent vertices comparisons  : %ld\n", r->adj);
    printf("  (Adjacent vertices of %d [%d]: ", adjV, r->nbCnt);
    for (int i = 0; i < r->nbCnt; i++) printf("%d%s", r->nb[i], i + 1 < r->nbCnt ? " " : "");
    printf(")\n\n");
}

static void run_graph(const char* name, int edgeCount, int caseBase) {
    static Edge edges[MAX_E];
    int cnt = generate_graph(edges, edgeCount);

    int insU, insV, delU, delV, conU, conV, adjV;
    do { insU = rand() % V; insV = rand() % V; } while (insU == insV || exists_tmp[insU][insV]);
    {
        int k = rand() % cnt;
        delU = edges[k].u; delV = edges[k].v;
    }
    do { conU = rand() % V; conV = rand() % V; } while (conU == conV);
    adjV = rand() % V;

    printf("=== %s Graph (V=%d, E=%d) ===\n", name, V, cnt);
    printf("Insert pair  : (%d, %d)\nDelete pair  : (%d, %d)\n"
        "Connect pair : (%d, %d)\nAdjacent vtx : %d\n\n",
        insU, insV, delU, delV, conU, conV, adjV);

    AdjMatrix* mg = (AdjMatrix*)malloc(sizeof(AdjMatrix));
    if (!mg) { perror("malloc"); exit(1); }
    mat_init(mg, edges, cnt);
    Result mr = { 0 };
    mr.mem = mat_memory(mg);
    {
        long c = 0;
        mat_has(mg, conU, conV, &c);
        mr.conn = c;
    }
    mr.adj = mat_neighbors(mg, adjV, mr.nb, &mr.nbCnt);
    mr.ins = mat_insert(mg, insU, insV);
    mr.del = mat_delete(mg, delU, delV);

    AdjList* lg = (AdjList*)malloc(sizeof(AdjList));
    if (!lg) { perror("malloc"); exit(1); }
    list_init(lg, edges, cnt);
    Result lr = { 0 };
    lr.mem = list_memory(lg);
    {
        long c = 0;
        list_find(lg, conU, conV, &c);
        lr.conn = c;
    }
    lr.adj = list_neighbors(lg, adjV, lr.nb, &lr.nbCnt);
    lr.ins = list_insert(lg, insU, insV);
    lr.del = list_delete(lg, delU, delV);

    print_result(caseBase, name, "Adjacency Matrix", &mr, adjV);
    print_result(caseBase + 1, name, "Adjacency List", &lr, adjV);

    free(mg);
    list_free(lg);
    free(lg);
}

int main(int argc, char* argv[]) {
    unsigned seed = (argc > 1) ? (unsigned)atoi(argv[1]) : (unsigned)time(NULL);
    srand(seed);
    printf("Random seed : %u\n\n", seed);

    run_graph("Sparse",V, 1);
    run_graph("Dense", MAX_E, 3);
    return 0;
}