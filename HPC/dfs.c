// Assignment 1: Parallel Depth First Search using OpenMP
// Compile: gcc -fopenmp dfs.c -o dfs
// Run:     ./dfs

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

void sequentialDFS(struct Graph* g, int start, int* visited) {
    visited[start] = 1;
    for (int i = 0; i < g->V; i++) {
        if (g->adj[start][i] && !visited[i])
            sequentialDFS(g, i, visited);
    }
}

// Must be called from inside an existing omp parallel+single region.
// Do NOT add #pragma omp parallel here — nesting parallel regions inside
// a task spawns a new thread team on every recursive call and explodes.
void parallelDFS(struct Graph* g, int start, int* visited) {
    for (int i = 0; i < g->V; i++) {
        if (g->adj[start][i] && !visited[i]) {
            int should_visit = 0;
            // Mark visited inside critical before spawning the task.
            // Prevents two threads both seeing !visited[i] and spawning
            // duplicate tasks for the same node.
            #pragma omp critical
            {
                if (!visited[i]) {
                    visited[i] = 1;
                    should_visit = 1;
                }
            }
            if (should_visit) {
                #pragma omp task
                parallelDFS(g, i, visited);
            }
        }
    }
    #pragma omp taskwait
}

int main() {
    int num_threads = omp_get_max_threads();
    double seq_times[5], par_times[5], speedups[5], efficiencies[5];
    int verts[5], start;

    printf("Neeti Kurulkar BE A 41038\n");
    printf("Threads available: %d\n\n", num_threads);

    for (int i = 0; i < 5; i++) {
        printf("Enter vertices for graph %d (1-10000): ", i + 1);
        scanf("%d", &verts[i]);
        if (verts[i] < 1 || verts[i] > 10000) {
            printf("Must be between 1 and 10000.\n");
            return 1;
        }

        // Same edge cap as BFS — generating ALL edges via rejection sampling
        // gets extremely slow as the graph fills up
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

        // Sequential DFS
        int* visited1 = (int*)calloc(verts[i], sizeof(int));
        double t1 = omp_get_wtime();
        sequentialDFS(&g, start, visited1);
        seq_times[i] = omp_get_wtime() - t1;
        free(visited1);

        // Parallel DFS — single parallel region, tasks spawned inside
        int* visited2 = (int*)calloc(verts[i], sizeof(int));
        visited2[start] = 1;
        double t2 = omp_get_wtime();
        #pragma omp parallel
        {
            #pragma omp single
            parallelDFS(&g, start, visited2);
        }
        par_times[i] = omp_get_wtime() - t2;
        free(visited2);

        speedups[i]     = seq_times[i] / par_times[i];
        efficiencies[i] = speedups[i] / num_threads;

        freeGraph(&g);
        printf("\n");
    }

    printf("Neeti Kurulkar BE A 41038\n");
    printf("\n| %-6s | %-8s | %-14s | %-14s | %-9s | %-10s |\n",
           "Graph", "Vertices", "Seq Time (s)", "Par Time (s)", "Speedup", "Efficiency");
    printf("|--------|----------|----------------|----------------|-----------|------------|\n");
    for (int i = 0; i < 5; i++) {
        printf("| %-6d | %-8d | %-14.6f | %-14.6f | %-9.2f | %-10.4f |\n",
               i + 1, verts[i], seq_times[i], par_times[i], speedups[i], efficiencies[i]);
    }
    printf("|--------|----------|----------------|----------------|-----------|------------|\n");

    FILE* csv = fopen("dfs_results.csv", "w");
    if (csv) {
        fprintf(csv, "graph,vertices,seq_time,par_time,speedup,efficiency\n");
        for (int i = 0; i < 5; i++)
            fprintf(csv, "%d,%d,%.6f,%.6f,%.4f,%.4f\n",
                    i + 1, verts[i], seq_times[i], par_times[i], speedups[i], efficiencies[i]);
        fclose(csv);
        printf("Results saved to dfs_results.csv\n");
    }

    return 0;
}
