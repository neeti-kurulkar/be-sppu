// Assignment 4a: Vector Addition using CUDA
// Compile: nvcc vector_addition.cu -o vector_addition
// Run:     ./vector_addition

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <cuda_runtime.h>

using namespace std;

#define BLOCK_SIZE 256

#define CUDA_CHECK(call) \
    { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            cerr << "CUDA Error: " << cudaGetErrorString(err) \
                 << " at line " << __LINE__ << endl; \
            exit(EXIT_FAILURE); \
        } \
    }

// ──────────────────── GPU Kernel ───────────────────────────────────

__global__ void vectorAdd(float* a, float* b, float* c, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) c[i] = a[i] + b[i];
}

// ──────────────────── CPU Function ─────────────────────────────────

void cpuVectorAdd(float* a, float* b, float* c, int n) {
    for (int i = 0; i < n; i++) c[i] = a[i] + b[i];
}

// ──────────────────── Main ─────────────────────────────────────────

int main() {
    long long sizes[] = {1000000, 5000000, 10000000, 50000000, 100000000};
    int num_cases = 5;

    cout << "Neeti Kurulkar BE A 41038\n\n";

    ofstream csv("vector_addition_results.csv");
    csv << "size,cpu_time,gpu_time,speedup,efficiency\n";

    cout << left
         << setw(12) << "Size"
         << setw(14) << "CPU Time(s)"
         << setw(14) << "GPU Time(s)"
         << setw(10) << "Speedup"
         << setw(12) << "Efficiency"
         << "\n";
    cout << string(62, '-') << "\n";

    int smCount;
    cudaDeviceGetAttribute(&smCount, cudaDevAttrMultiProcessorCount, 0);

    for (int t = 0; t < num_cases; t++) {
        long long n = sizes[t];

        float* h_a = new float[n];
        float* h_b = new float[n];
        float* h_c_cpu = new float[n];
        float* h_c_gpu = new float[n];

        for (long long i = 0; i < n; i++) {
            h_a[i] = (float)(rand() % 1000) / 10.0f;
            h_b[i] = (float)(rand() % 1000) / 10.0f;
        }

        // ── CPU ──
        auto c1 = chrono::high_resolution_clock::now();
        cpuVectorAdd(h_a, h_b, h_c_cpu, n);
        double cpuTime = chrono::duration<double>(chrono::high_resolution_clock::now() - c1).count();

        // ── GPU ──
        float *d_a, *d_b, *d_c;
        CUDA_CHECK(cudaMalloc(&d_a, n * sizeof(float)));
        CUDA_CHECK(cudaMalloc(&d_b, n * sizeof(float)));
        CUDA_CHECK(cudaMalloc(&d_c, n * sizeof(float)));

        CUDA_CHECK(cudaMemcpy(d_a, h_a, n * sizeof(float), cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemcpy(d_b, h_b, n * sizeof(float), cudaMemcpyHostToDevice));

        int numBlocks = (n + BLOCK_SIZE - 1) / BLOCK_SIZE;

        cudaEvent_t ev1, ev2;
        float gpuMs = 0;
        cudaEventCreate(&ev1);
        cudaEventCreate(&ev2);
        cudaEventRecord(ev1);

        vectorAdd<<<numBlocks, BLOCK_SIZE>>>(d_a, d_b, d_c, n);

        cudaEventRecord(ev2);
        cudaEventSynchronize(ev2);
        cudaEventElapsedTime(&gpuMs, ev1, ev2);

        CUDA_CHECK(cudaMemcpy(h_c_gpu, d_c, n * sizeof(float), cudaMemcpyDeviceToHost));

        double gpuTime  = gpuMs / 1000.0;
        double speedup  = cpuTime / gpuTime;
        double efficiency = speedup / smCount;

        cout << fixed << setprecision(6) << left
             << setw(12) << n
             << setw(14) << cpuTime
             << setw(14) << gpuTime
             << setw(10) << fixed << setprecision(2) << speedup
             << setw(12) << fixed << setprecision(4) << efficiency
             << "\n";

        csv << n << "," << cpuTime << "," << gpuTime << ","
            << speedup << "," << efficiency << "\n";

        cudaFree(d_a); cudaFree(d_b); cudaFree(d_c);
        cudaEventDestroy(ev1); cudaEventDestroy(ev2);
        delete[] h_a; delete[] h_b; delete[] h_c_cpu; delete[] h_c_gpu;
    }

    csv.close();
    cout << "\nResults saved to vector_addition_results.csv\n";
    return 0;
}
