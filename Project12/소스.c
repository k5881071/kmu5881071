#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define V      10
#define E      20
#define MAX_W  100
#define INF    1000000000

typedef struct { int u, v, w; } Edge;

static int randRange(int lo, int hi) {
    return lo + rand() % (hi - lo + 1);
}

static void generateGraph(Edge edges[E]) {
    bool used[V][V] = { {false} };
    int perm[V], cnt = 0;

    for (int i = 0; i < V; i++) perm[i] = i;
    for (int i = V - 1; i > 0; i--) {
        int j = randRange(0, i);
        int t = perm[i]; perm[i] = perm[j]; perm[j] = t;
    }

    for (int i = 1; i < V; i++) {
        int a = perm[i];
        int b = perm[randRange(0, i - 1)];
        edges[cnt].u = a < b ? a : b;
        edges[cnt].v = a < b ? b : a;
        edges[cnt].w = randRange(1, MAX_W);
        used[a][b] = used[b][a] = true;
        cnt++;
    }

    while (cnt < E) {
        int a = randRange(0, V - 1);
        int b = randRange(0, V - 1);
        if (a == b || used[a][b]) continue;
        edges[cnt].u = a < b ? a : b;
        edges[cnt].v = a < b ? b : a;
        edges[cnt].w = randRange(1, MAX_W);
        used[a][b] = used[b][a] = true;
        cnt++;
    }
}

static bool isConnected(const Edge edges[E]) {
    bool adj[V][V] = { {false} };
    bool vis[V] = { false };
    int stack[V], top = 0, count = 1;

    for (int i = 0; i < E; i++)
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

static int cmpEdge(const void* a, const void* b) {
    const Edge* x = a, * y = b;
    if (x->u != y->u) return x->u - y->u;
    return x->v - y->v;
}

static void floydWarshall(const Edge edges[E], int dist[V][V], int next[V][V]) {
    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++) {
            dist[i][j] = (i == j) ? 0 : INF;
            next[i][j] = -1;
        }

    for (int i = 0; i < E; i++) {
        int u = edges[i].u, v = edges[i].v, w = edges[i].w;
        dist[u][v] = dist[v][u] = w;
        next[u][v] = v;
        next[v][u] = u;
    }

    for (int k = 0; k < V; k++)
        for (int i = 0; i < V; i++) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < V; j++) {
                if (dist[k][j] == INF) continue;
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
}

static int reconstructPath(int next[V][V], int s, int t, int path[V]) {
    int n = 0, cur = s;
    path[n++] = cur;
    while (cur != t) {
        cur = next[cur][t];
        path[n++] = cur;
    }
    return n;
}

static int edgeWeight(const Edge edges[E], int a, int b) {
    for (int i = 0; i < E; i++)
        if ((edges[i].u == a && edges[i].v == b) ||
            (edges[i].u == b && edges[i].v == a))
            return edges[i].w;
    return -1;
}

int main(void) {
    Edge edges[E];
    int dist[V][V], next[V][V];

    srand((unsigned)time(NULL));

    do { generateGraph(edges); } while (!isConnected(edges));
    qsort(edges, E, sizeof(Edge), cmpEdge);

    printf("Graph\n");
    printf("Vertices : %d\n", V);
    printf("Edges    : %d\n\n", E);
    printf("Edges\n");
    for (int i = 0; i < E; i++)
        printf("(%d, %d) Weight: %d\n", edges[i].u, edges[i].v, edges[i].w);
    printf("\n");

    floydWarshall(edges, dist, next);

    printf("Shortest Paths\n\n");
    int pairs = 0;
    bool allOk = true;
    for (int u = 0; u < V; u++) {
        for (int v = u + 1; v < V; v++) {
            int path[V];
            int n = reconstructPath(next, u, v, path);

            printf("%d -> %d\n", u, v);
            printf("Path     : ");
            for (int i = 0; i < n; i++)
                printf(i ? " -> %d" : "%d", path[i]);
            printf("\n");
            printf("Distance : %d\n\n", dist[u][v]);
            pairs++;

            int sum = 0;
            for (int i = 0; i + 1 < n; i++) sum += edgeWeight(edges, path[i], path[i + 1]);
            if (sum != dist[u][v]) allOk = false;
        }
    }

    printf("Total vertex pairs : %d\n", pairs);
    printf("[Check] path cost == distance for all pairs: %s\n", allOk ? "PASS" : "FAIL");
    return (pairs == 45 && allOk) ? 0 : 1;
}