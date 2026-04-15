// Assignment 2: Parallel Bubble Sort using OpenMP
// Compile: gcc -fopenmp bubblesort.c -o bubblesort
// Run:     ./bubblesort

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// Standard sequential bubble sort
void sequentialBubbleSort(int* arr, int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = tmp;
            }
}

// Odd-Even Transposition Sort — parallel-safe version of bubble sort.
// Each phase compares independent non-overlapping pairs, so all swaps
// within one phase can run in parallel without data conflicts.
// n phases guarantees the array is fully sorted.
void parallelBubbleSort(int* arr, int n) {
    for (int phase = 0; phase < n; phase++) {
        int start = phase % 2;  // 0 = even-indexed pairs, 1 = odd-indexed pairs
        #pragma omp parallel for shared(arr)
        for (int j = start; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = tmp;
            }
        }
    }
}

int main() {
    int sizes[] = {5000, 10000, 20000, 30000, 50000};
    int num_cases = 5;
    int num_threads = omp_get_max_threads();

    double seq_times[5], par_times[5], speedups[5], efficiencies[5];

    printf("Neeti Kurulkar BE A 41038\n");
    printf("Threads available: %d\n\n", num_threads);

    for (int t = 0; t < num_cases; t++) {
        int n = sizes[t];
        int* arr1 = (int*)malloc(n * sizeof(int));
        int* arr2 = (int*)malloc(n * sizeof(int));

        srand(42 + t);
        for (int i = 0; i < n; i++) arr1[i] = rand() % 100000;
        memcpy(arr2, arr1, n * sizeof(int));

        printf("Running size %d... (sequential)", n);
        fflush(stdout);
        double t1 = omp_get_wtime();
        sequentialBubbleSort(arr1, n);
        seq_times[t] = omp_get_wtime() - t1;

        printf(" done. (parallel)");
        fflush(stdout);
        double t2 = omp_get_wtime();
        parallelBubbleSort(arr2, n);
        par_times[t] = omp_get_wtime() - t2;
        printf(" done.\n");

        speedups[t]     = seq_times[t] / par_times[t];
        efficiencies[t] = speedups[t] / num_threads;

        free(arr1);
        free(arr2);
    }

    printf("| %-8s | %-14s | %-14s | %-9s | %-10s |\n",
           "Size", "Seq Time (s)", "Par Time (s)", "Speedup", "Efficiency");
    printf("|----------|----------------|----------------|-----------|------------|\n");
    for (int t = 0; t < num_cases; t++) {
        printf("| %-8d | %-14.6f | %-14.6f | %-9.2f | %-10.4f |\n",
               sizes[t], seq_times[t], par_times[t], speedups[t], efficiencies[t]);
    }
    printf("|----------|----------------|----------------|-----------|------------|\n");

    FILE* csv = fopen("bubblesort_results.csv", "w");
    if (csv) {
        fprintf(csv, "size,seq_time,par_time,speedup,efficiency\n");
        for (int t = 0; t < num_cases; t++)
            fprintf(csv, "%d,%.6f,%.6f,%.4f,%.4f\n",
                    sizes[t], seq_times[t], par_times[t], speedups[t], efficiencies[t]);
        fclose(csv);
        printf("Results saved to bubblesort_results.csv\n");
    }

    return 0;
}
