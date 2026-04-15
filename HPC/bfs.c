// Assignment 1: Parallel Breadth First Search using OpenMP
// Compile: gcc -fopenmp bfs.c -o bfs
// Run:     ./bfs

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_VERTICES 10000

struct Graph {
    int V;
    int** adj;
};

void initGraph(struct Graph* g, int vertices) {
    g->V = vertices;
    g->adj = (int**)malloc(vertices * sizeof(int*));
    for (int i = 0; i < vertices; i++)
        g->adj[i] = (int*)calloc(vertices, sizeof(int));
}

void freeGraph(struct Graph* g) {
    for (int i = 0; i < g->V; i++) free(g->adj[i]);
    free(g->adj);
}

void addEdge(struct Graph* g, int u, int v) {
    g->adj[u][v] = 1;
    g->adj[v][u] = 1;
}

void generateRandomEdges(struct Graph* g, int max_edges) {
    int edges = 0;
    while (edges < max_edges) {
        int u = rand() % g->V;
        int v = rand() % g->V;
        if (u != v && g->adj[u][v] == 0) {
            addEdge(g, u, v);
            edges++;
        }
    }
}

void sequentialBFS(struct Graph* g, int start) {
    int V = g->V;
    // malloc instead of VLA — avoids stack overflow for large graphs
    int* visited = (int*)calloc(V, sizeof(int));
    int* q = (int*)malloc(MAX_VERTICES * sizeof(int));
    int front = 0, rear = 0;

    visited[start] = 1;
    q[rear++] = start;

    while (front != rear) {
        int node = q[front++];
        for (int i = 0; i < V; i++) {
            if (g->adj[node][i] && !visited[i]) {
                visited[i] = 1;
                q[rear++] = i;
            }
        }
    }

    free(visited);
    free(q);
}

void parallelBFS(struct Graph* g, int start) {
    int V = g->V;
    int* visited = (int*)calloc(V, sizeof(int));
    int* q = (int*)malloc(MAX_VERTICES * sizeof(int));
    int front = 0, rear = 0;

    visited[start] = 1;
    q[rear++] = start;

    while (front != rear) {
        int localFront = front;
        int localRear = rear;

        #pragma omp parallel
        {
            int* tq = (int*)malloc(MAX_VERTICES * sizeof(int));
            int tq_rear = 0;

            #pragma omp for
            for (int i = localFront; i < localRear; i++) {
                int node = q[i];
                for (int j = 0; j < V; j++) {
                    if (g->adj[node][j] && !visited[j]) {
                        // double-check inside critical: prevents two threads from
                        // both passing !visited[j] before either sets it to 1
                        #pragma omp critical
                        {
                            if (!visited[j]) {
                                visited[j] = 1;
                                tq[tq_rear++] = j;
                            }
                        }
                    }
                }
            }

            #pragma omp critical
            {
                for (int i = 0; i < tq_rear; i++)
                    q[rear++] = tq[i];
            }
            free(tq);
        }

        front = localRear;
    }

    free(q);
    free(visited);
}

int main() {
    int num_threads = omp_get_max_threads();
    double seq_times[5], par_times[5], speedups[5], efficiencies[5];
    int verts[5], start;

    printf("Advait Shinde BE A 41058\n");
    printf("Threads available: %d\n\n", num_threads);

    for (int i = 0; i < 5; i++) {
        printf("Enter vertices for graph %d (1-10000): ", i + 1);
        scanf("%d", &verts[i]);
        if (verts[i] < 1 || verts[i] > 10000) {
            printf("Must be between 1 and 10000.\n");
            return 1;
        }

        int max_possible = verts[i] * (verts[i] - 1) / 2;
        int max_edges = (max_possible < verts[i] * 5) ? max_possible : verts[i] * 5;
        printf("Generating %d edges.\n", max_edges);

        struct Graph g;
        initGraph(&g, verts[i]);
        srand(42 + i);
        generateRandomEdges(&g, max_edges);

        printf("Enter starting vertex (0-%d): ", verts[i] - 1);
        scanf("%d", &start);
        if (start < 0 || start >= verts[i]) {
            printf("Invalid vertex.\n");
            return 1;
        }

        // Both use omp_get_wtime() for consistent wall-clock timing
        double t1 = omp_get_wtime();
        sequentialBFS(&g, start);
        seq_times[i] = omp_get_wtime() - t1;

        double t2 = omp_get_wtime();
        parallelBFS(&g, start);
        par_times[i] = omp_get_wtime() - t2;

        speedups[i]    = seq_times[i] / par_times[i];
        efficiencies[i] = speedups[i] / num_threads;

        freeGraph(&g);
        printf("\n");
    }

    printf("Advait Shinde BE A 41058\n");
    printf("\n| %-6s | %-8s | %-14s | %-14s | %-9s | %-10s |\n",
           "Graph", "Vertices", "Seq Time (s)", "Par Time (s)", "Speedup", "Efficiency");
    printf("|--------|----------|----------------|----------------|-----------|------------|\n");
    for (int i = 0; i < 5; i++) {
        printf("| %-6d | %-8d | %-14.6f | %-14.6f | %-9.2f | %-10.4f |\n",
               i + 1, verts[i], seq_times[i], par_times[i], speedups[i], efficiencies[i]);
    }
    printf("|--------|----------|----------------|----------------|-----------|------------|\n");

    FILE* csv = fopen("bfs_results.csv", "w");
    if (csv) {
        fprintf(csv, "graph,vertices,seq_time,par_time,speedup,efficiency\n");
        for (int i = 0; i < 5; i++)
            fprintf(csv, "%d,%d,%.6f,%.6f,%.4f,%.4f\n",
                    i + 1, verts[i], seq_times[i], par_times[i], speedups[i], efficiencies[i]);
        fclose(csv);
        printf("Results saved to bfs_results.csv\n");
    }

    return 0;
}
