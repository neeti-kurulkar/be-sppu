// Assignment 2: Parallel Bubble Sort and Merge Sort using OpenMP
// Compile: gcc -fopenmp sorting.c -o sorting
// Run:     ./sorting

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// ─────────────────────────── Bubble Sort ───────────────────────────

void sequentialBubbleSort(int* arr, int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = tmp;
            }
}

// Odd-Even Transposition Sort: parallel-safe version of bubble sort.
// Each phase swaps independent pairs, so all swaps in one phase are parallelizable.
void parallelBubbleSort(int* arr, int n) {
    for (int phase = 0; phase < n; phase++) {
        int start = phase % 2;  // 0 = even pairs, 1 = odd pairs
        #pragma omp parallel for shared(arr)
        for (int j = start; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = tmp;
            }
        }
    }
}

// ─────────────────────────── Merge Sort ────────────────────────────

void merge(int* arr, int l, int m, int r) {
    int n1 = m - l + 1, n2 = r - m;
    int* L = (int*)malloc(n1 * sizeof(int));
    int* R = (int*)malloc(n2 * sizeof(int));
    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int i = 0; i < n2; i++) R[i] = arr[m + 1 + i];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    free(L); free(R);
}

void sequentialMergeSort(int* arr, int l, int r) {
    if (l < r) {
        int m = (l + r) / 2;
        sequentialMergeSort(arr, l, m);
        sequentialMergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

// depth controls recursion levels that spawn parallel tasks.
// depth=4 → up to 16 concurrent tasks at the leaves.
void parallelMergeSort(int* arr, int l, int r, int depth) {
    if (l < r) {
        int m = (l + r) / 2;
        if (depth > 0) {
            #pragma omp task shared(arr)
            parallelMergeSort(arr, l, m, depth - 1);
            #pragma omp task shared(arr)
            parallelMergeSort(arr, m + 1, r, depth - 1);
            #pragma omp taskwait
        } else {
            sequentialMergeSort(arr, l, m);
            sequentialMergeSort(arr, m + 1, r);
        }
        merge(arr, l, m, r);
    }
}

// ─────────────────────────── Main ──────────────────────────────────

int main() {
    int sizes[] = {1000, 5000, 10000, 20000, 30000};
    int num_cases = 5;
    int num_threads = omp_get_max_threads();

    // Storage for CSV output
    double bub_seq[5], bub_par[5], mer_seq[5], mer_par[5];

    printf("Advait Shinde BE A 41058\n");
    printf("Threads available: %d\n\n", num_threads);

    // ── Bubble Sort ──
    printf("=== Bubble Sort (Odd-Even Transposition) ===\n");
    printf("%-10s %-14s %-14s %-10s %-12s\n",
           "Size", "Seq Time(s)", "Par Time(s)", "Speedup", "Efficiency");
    printf("---------------------------------------------------------------\n");

    for (int t = 0; t < num_cases; t++) {
        int n = sizes[t];
        int* arr1 = (int*)malloc(n * sizeof(int));
        int* arr2 = (int*)malloc(n * sizeof(int));

        srand(42 + t);
        for (int i = 0; i < n; i++) arr1[i] = rand() % 100000;
        memcpy(arr2, arr1, n * sizeof(int));

        double seq_start = omp_get_wtime();
        sequentialBubbleSort(arr1, n);
        bub_seq[t] = omp_get_wtime() - seq_start;

        double par_start = omp_get_wtime();
        parallelBubbleSort(arr2, n);
        bub_par[t] = omp_get_wtime() - par_start;

        double speedup = bub_seq[t] / bub_par[t];
        printf("%-10d %-14.6f %-14.6f %-10.2f %-12.4f\n",
               n, bub_seq[t], bub_par[t], speedup, speedup / num_threads);

        free(arr1); free(arr2);
    }

    // ── Merge Sort ──
    printf("\n=== Merge Sort ===\n");
    printf("%-10s %-14s %-14s %-10s %-12s\n",
           "Size", "Seq Time(s)", "Par Time(s)", "Speedup", "Efficiency");
    printf("---------------------------------------------------------------\n");

    for (int t = 0; t < num_cases; t++) {
        int n = sizes[t];
        int* arr1 = (int*)malloc(n * sizeof(int));
        int* arr2 = (int*)malloc(n * sizeof(int));

        srand(42 + t);
        for (int i = 0; i < n; i++) arr1[i] = rand() % 100000;
        memcpy(arr2, arr1, n * sizeof(int));

        double seq_start = omp_get_wtime();
        sequentialMergeSort(arr1, 0, n - 1);
        mer_seq[t] = omp_get_wtime() - seq_start;

        double par_start = omp_get_wtime();
        #pragma omp parallel
        {
            #pragma omp single
            parallelMergeSort(arr2, 0, n - 1, 4);
        }
        mer_par[t] = omp_get_wtime() - par_start;

        double speedup = mer_seq[t] / mer_par[t];
        printf("%-10d %-14.6f %-14.6f %-10.2f %-12.4f\n",
               n, mer_seq[t], mer_par[t], speedup, speedup / num_threads);

        free(arr1); free(arr2);
    }

    // ── Write CSV ──
    FILE* csv = fopen("sorting_results.csv", "w");
    if (csv) {
        fprintf(csv, "algorithm,size,seq_time,par_time,speedup\n");
        for (int t = 0; t < num_cases; t++) {
            fprintf(csv, "bubble,%d,%.6f,%.6f,%.4f\n",
                    sizes[t], bub_seq[t], bub_par[t], bub_seq[t] / bub_par[t]);
            fprintf(csv, "merge,%d,%.6f,%.6f,%.4f\n",
                    sizes[t], mer_seq[t], mer_par[t], mer_seq[t] / mer_par[t]);
        }
        fclose(csv);
        printf("\nResults saved to sorting_results.csv\n");
    }

    return 0;
}
