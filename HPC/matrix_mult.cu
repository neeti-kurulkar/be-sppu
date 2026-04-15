// Assignment 4b: Matrix Multiplication using CUDA
// Compile: nvcc matrix_mult.cu -o matrix_mult
// Run:     ./matrix_mult

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <cuda_runtime.h>

using namespace std;

#define TILE_SIZE 16

#define CUDA_CHECK(call) \
    { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            cerr << "CUDA Error: " << cudaGetErrorString(err) \
                 << " at line " << __LINE__ << endl; \
            exit(EXIT_FAILURE); \
        } \
    }

// ──────────────────── GPU Kernel (Tiled) ───────────────────────────
// Loads tiles of A and B into shared memory to reduce global memory
// accesses, which is the main bottleneck in naive matrix multiply.

__global__ void matMulTiled(float* A, float* B, float* C, int n) {
    __shared__ float tileA[TILE_SIZE][TILE_SIZE];
    __shared__ float tileB[TILE_SIZE][TILE_SIZE];

    int row = blockIdx.y * TILE_SIZE + threadIdx.y;
    int col = blockIdx.x * TILE_SIZE + threadIdx.x;
    float sum = 0.0f;

    for (int t = 0; t < (n + TILE_SIZE - 1) / TILE_SIZE; t++) {
        // Load tile of A
        if (row < n && t * TILE_SIZE + threadIdx.x < n)
            tileA[threadIdx.y][threadIdx.x] = A[row * n + t * TILE_SIZE + threadIdx.x];
        else
            tileA[threadIdx.y][threadIdx.x] = 0.0f;

        // Load tile of B
        if (col < n && t * TILE_SIZE + threadIdx.y < n)
            tileB[threadIdx.y][threadIdx.x] = B[(t * TILE_SIZE + threadIdx.y) * n + col];
        else
            tileB[threadIdx.y][threadIdx.x] = 0.0f;

        __syncthreads();

        for (int k = 0; k < TILE_SIZE; k++)
            sum += tileA[threadIdx.y][k] * tileB[k][threadIdx.x];

        __syncthreads();
    }

    if (row < n && col < n)
        C[row * n + col] = sum;
}

// ──────────────────── CPU Function ─────────────────────────────────

void cpuMatMul(float* A, float* B, float* C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            float s = 0;
            for (int k = 0; k < n; k++)
                s += A[i * n + k] * B[k * n + j];
            C[i * n + j] = s;
        }
}

// ──────────────────── Main ─────────────────────────────────────────

int main() {
    int sizes[] = {128, 256, 512, 1024, 2048};
    int num_cases = 5;

    cout << "Neeti Kurulkar BE A 41038\n\n";

    ofstream csv("matrix_mult_results.csv");
    csv << "size,cpu_time,gpu_time,speedup,efficiency\n";

    cout << left
         << setw(8)  << "N"
         << setw(14) << "CPU Time(s)"
         << setw(14) << "GPU Time(s)"
         << setw(10) << "Speedup"
         << setw(12) << "Efficiency"
         << "\n";
    cout << string(58, '-') << "\n";

    int smCount;
    cudaDeviceGetAttribute(&smCount, cudaDevAttrMultiProcessorCount, 0);

    for (int t = 0; t < num_cases; t++) {
        int n = sizes[t];
        long long total = (long long)n * n;

        float* h_A = new float[total];
        float* h_B = new float[total];
        float* h_C_cpu = new float[total]();
        float* h_C_gpu = new float[total]();

        for (long long i = 0; i < total; i++) {
            h_A[i] = (float)(rand() % 100) / 10.0f;
            h_B[i] = (float)(rand() % 100) / 10.0f;
        }

        // ── CPU (skip for 2048 — too slow) ──
        double cpuTime = 0;
        if (n <= 1024) {
            auto c1 = chrono::high_resolution_clock::now();
            cpuMatMul(h_A, h_B, h_C_cpu, n);
            cpuTime = chrono::duration<double>(chrono::high_resolution_clock::now() - c1).count();
        }

        // ── GPU ──
        float *d_A, *d_B, *d_C;
        CUDA_CHECK(cudaMalloc(&d_A, total * sizeof(float)));
        CUDA_CHECK(cudaMalloc(&d_B, total * sizeof(float)));
        CUDA_CHECK(cudaMalloc(&d_C, total * sizeof(float)));

        CUDA_CHECK(cudaMemcpy(d_A, h_A, total * sizeof(float), cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemcpy(d_B, h_B, total * sizeof(float), cudaMemcpyHostToDevice));

        dim3 threads(TILE_SIZE, TILE_SIZE);
        dim3 blocks((n + TILE_SIZE - 1) / TILE_SIZE, (n + TILE_SIZE - 1) / TILE_SIZE);

        cudaEvent_t ev1, ev2;
        float gpuMs = 0;
        cudaEventCreate(&ev1);
        cudaEventCreate(&ev2);
        cudaEventRecord(ev1);

        matMulTiled<<<blocks, threads>>>(d_A, d_B, d_C, n);

        cudaEventRecord(ev2);
        cudaEventSynchronize(ev2);
        cudaEventElapsedTime(&gpuMs, ev1, ev2);

        CUDA_CHECK(cudaMemcpy(h_C_gpu, d_C, total * sizeof(float), cudaMemcpyDeviceToHost));

        double gpuTime  = gpuMs / 1000.0;
        double speedup  = (cpuTime > 0) ? cpuTime / gpuTime : 0;
        double efficiency = (speedup > 0) ? speedup / smCount : 0;

        cout << fixed << left
             << setw(8)  << n
             << setw(14) << setprecision(6) << cpuTime
             << setw(14) << setprecision(6) << gpuTime
             << setw(10) << setprecision(2) << speedup
             << setw(12) << setprecision(4) << efficiency;
        if (n > 1024) cout << "  (CPU skipped - too slow)";
        cout << "\n";

        csv << n << "," << cpuTime << "," << gpuTime << ","
            << speedup << "," << efficiency << "\n";

        cudaFree(d_A); cudaFree(d_B); cudaFree(d_C);
        cudaEventDestroy(ev1); cudaEventDestroy(ev2);
        delete[] h_A; delete[] h_B; delete[] h_C_cpu; delete[] h_C_gpu;
    }

    csv.close();
    cout << "\nResults saved to matrix_mult_results.csv\n";
    return 0;
}
