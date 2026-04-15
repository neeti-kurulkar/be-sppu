// Assignment 2: Parallel Merge Sort using OpenMP
// Compile: gcc -fopenmp mergesort.c -o mergesort
// Run:     ./mergesort

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

void merge(int* arr, int l, int m, int r) {
    int n1 = m - l + 1, n2 = r - m;
    int* L = (int*)malloc(n1 * sizeof(int));
    int* R = (int*)malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int i = 0; i < n2; i++) R[i] = arr[m + 1 + i];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2)
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

void sequentialMergeSort(int* arr, int l, int r) {
    if (l < r) {
        int m = (l + r) / 2;
        sequentialMergeSort(arr, l, m);
        sequentialMergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

// Must be called inside a parallel+single region.
// depth controls how many levels of recursion spawn tasks.
// depth=4 gives up to 2^4=16 concurrent tasks — enough to saturate
// most systems without excessive task-creation overhead.
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
            // Below depth threshold, fall back to sequential to avoid
            // spawning thousands of tiny tasks
            sequentialMergeSort(arr, l, m);
            sequentialMergeSort(arr, m + 1, r);
        }
        merge(arr, l, m, r);
    }
}

int main() {
    int sizes[] = {100000, 500000, 1000000, 5000000, 10000000};
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
        for (int i = 0; i < n; i++) arr1[i] = rand() % 1000000;
        memcpy(arr2, arr1, n * sizeof(int));

        double t1 = omp_get_wtime();
        sequentialMergeSort(arr1, 0, n - 1);
        seq_times[t] = omp_get_wtime() - t1;

        double t2 = omp_get_wtime();
        #pragma omp parallel
        {
            #pragma omp single
            parallelMergeSort(arr2, 0, n - 1, 4);
        }
        par_times[t] = omp_get_wtime() - t2;

        speedups[t]     = seq_times[t] / par_times[t];
        efficiencies[t] = speedups[t] / num_threads;

        free(arr1);
        free(arr2);
    }

    printf("| %-10s | %-14s | %-14s | %-9s | %-10s |\n",
           "Size", "Seq Time (s)", "Par Time (s)", "Speedup", "Efficiency");
    printf("|------------|----------------|----------------|-----------|------------|\n");
    for (int t = 0; t < num_cases; t++) {
        printf("| %-10d | %-14.6f | %-14.6f | %-9.2f | %-10.4f |\n",
               sizes[t], seq_times[t], par_times[t], speedups[t], efficiencies[t]);
    }
    printf("|------------|----------------|----------------|-----------|------------|\n");

    FILE* csv = fopen("mergesort_results.csv", "w");
    if (csv) {
        fprintf(csv, "size,seq_time,par_time,speedup,efficiency\n");
        for (int t = 0; t < num_cases; t++)
            fprintf(csv, "%d,%.6f,%.6f,%.4f,%.4f\n",
                    sizes[t], seq_times[t], par_times[t], speedups[t], efficiencies[t]);
        fclose(csv);
        printf("Results saved to mergesort_results.csv\n");
    }

    return 0;
}
