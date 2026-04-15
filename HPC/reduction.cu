// Assignment 3: Parallel Reduction - Min, Max, Sum, Average using CUDA
// Compile: nvcc reduction.cu -o reduction
// Run:     ./reduction

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <limits>
#include <cstdlib>
#include <cuda_runtime.h>

using namespace std;

#define BLOCK_SIZE 256
#define WARP_SIZE  32

#define CUDA_CHECK(call) \
    { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            cerr << "CUDA Error: " << cudaGetErrorString(err) \
                 << " at line " << __LINE__ << endl; \
            exit(EXIT_FAILURE); \
        } \
    }

// ──────────────────── Warp-level reductions ────────────────────────

__inline__ __device__ int warpReduceSum(int val) {
    for (int off = WARP_SIZE / 2; off > 0; off >>= 1)
        val += __shfl_down_sync(0xFFFFFFFF, val, off);
    return val;
}

__inline__ __device__ int warpReduceMin(int val) {
    for (int off = WARP_SIZE / 2; off > 0; off >>= 1)
        val = min(val, __shfl_down_sync(0xFFFFFFFF, val, off));
    return val;
}

__inline__ __device__ int warpReduceMax(int val) {
    for (int off = WARP_SIZE / 2; off > 0; off >>= 1)
        val = max(val, __shfl_down_sync(0xFFFFFFFF, val, off));
    return val;
}

// ──────────────────── GPU Kernels ──────────────────────────────────

__global__ void reduceSum(int* input, unsigned long long* output, int n) {
    __shared__ int shared[BLOCK_SIZE];
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    int val = (tid < n) ? input[tid] : 0;

    val = warpReduceSum(val);
    int lane = threadIdx.x % WARP_SIZE;
    int warpId = threadIdx.x / WARP_SIZE;
    if (lane == 0) shared[warpId] = val;
    __syncthreads();

    if (warpId == 0) {
        val = (lane < blockDim.x / WARP_SIZE) ? shared[lane] : 0;
        val = warpReduceSum(val);
    }
    if (threadIdx.x == 0) atomicAdd(output, (unsigned long long)val);
}

__global__ void reduceMin(int* input, int* output, int n) {
    __shared__ int shared[BLOCK_SIZE];
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    int val = (tid < n) ? input[tid] : INT_MAX;

    val = warpReduceMin(val);
    int lane = threadIdx.x % WARP_SIZE;
    int warpId = threadIdx.x / WARP_SIZE;
    if (lane == 0) shared[warpId] = val;
    __syncthreads();

    if (warpId == 0) {
        val = (lane < blockDim.x / WARP_SIZE) ? shared[lane] : INT_MAX;
        val = warpReduceMin(val);
    }
    if (threadIdx.x == 0) atomicMin(output, val);
}

__global__ void reduceMax(int* input, int* output, int n) {
    __shared__ int shared[BLOCK_SIZE];
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    int val = (tid < n) ? input[tid] : INT_MIN;

    val = warpReduceMax(val);
    int lane = threadIdx.x % WARP_SIZE;
    int warpId = threadIdx.x / WARP_SIZE;
    if (lane == 0) shared[warpId] = val;
    __syncthreads();

    if (warpId == 0) {
        val = (lane < blockDim.x / WARP_SIZE) ? shared[lane] : INT_MIN;
        val = warpReduceMax(val);
    }
    if (threadIdx.x == 0) atomicMax(output, val);
}

// ──────────────────── CPU Functions ────────────────────────────────

long long cpuSum(const vector<int>& data) {
    long long s = 0;
    for (int v : data) s += v;
    return s;
}

int cpuMin(const vector<int>& data) {
    int m = INT_MAX;
    for (int v : data) if (v < m) m = v;
    return m;
}

int cpuMax(const vector<int>& data) {
    int m = INT_MIN;
    for (int v : data) if (v > m) m = v;
    return m;
}

// ──────────────────── Main ─────────────────────────────────────────

int main() {
    vector<long long> sizes = {1000000, 5000000, 10000000, 50000000, 100000000};

    cout << "Neeti Kurulkar BE A 41038\n\n";

    ofstream csv("reduction_results.csv");
    csv << "size,cpu_time,gpu_time,speedup,efficiency,cpu_sum,gpu_sum,cpu_min,gpu_min,cpu_max,gpu_max\n";

    cout << left
         << setw(12) << "Size"
         << setw(14) << "CPU Time(s)"
         << setw(14) << "GPU Time(s)"
         << setw(10) << "Speedup"
         << setw(12) << "Efficiency"
         << setw(14) << "CPU Sum"
         << setw(14) << "GPU Sum"
         << setw(10) << "CPU Min"
         << setw(10) << "GPU Min"
         << setw(10) << "CPU Max"
         << setw(10) << "GPU Max"
         << "\n";
    cout << string(130, '-') << "\n";

    for (long long n : sizes) {
        vector<int> data(n);
        for (long long j = 0; j < n; j++) data[j] = rand() % 10000;

        // ── Device allocations ──
        int* d_input;
        unsigned long long* d_sum;
        int* d_min;
        int* d_max;

        unsigned long long h_sum = 0;
        int h_min = INT_MAX;
        int h_max = INT_MIN;

        CUDA_CHECK(cudaMalloc(&d_input, n * sizeof(int)));
        CUDA_CHECK(cudaMalloc(&d_sum,   sizeof(unsigned long long)));
        CUDA_CHECK(cudaMalloc(&d_min,   sizeof(int)));
        CUDA_CHECK(cudaMalloc(&d_max,   sizeof(int)));

        CUDA_CHECK(cudaMemcpy(d_input, data.data(), n * sizeof(int), cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemset(d_sum, 0, sizeof(unsigned long long)));
        CUDA_CHECK(cudaMemcpy(d_min, &h_min, sizeof(int), cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemcpy(d_max, &h_max, sizeof(int), cudaMemcpyHostToDevice));

        int numBlocks = (n + BLOCK_SIZE - 1) / BLOCK_SIZE;

        // ── CPU ──
        auto c1 = chrono::high_resolution_clock::now();
        long long cs = cpuSum(data);
        int       cm = cpuMin(data);
        int       cx = cpuMax(data);
        double    ca = (double)cs / n;
        auto c2 = chrono::high_resolution_clock::now();
        double cpuTime = chrono::duration<double>(c2 - c1).count();

        // ── GPU ──
        cudaEvent_t ev1, ev2;
        float gpuMs = 0;
        cudaEventCreate(&ev1);
        cudaEventCreate(&ev2);
        cudaEventRecord(ev1);

        reduceSum<<<numBlocks, BLOCK_SIZE>>>(d_input, d_sum, n);
        reduceMin<<<numBlocks, BLOCK_SIZE>>>(d_input, d_min, n);
        reduceMax<<<numBlocks, BLOCK_SIZE>>>(d_input, d_max, n);

        cudaEventRecord(ev2);
        cudaEventSynchronize(ev2);
        cudaEventElapsedTime(&gpuMs, ev1, ev2);

        CUDA_CHECK(cudaMemcpy(&h_sum, d_sum, sizeof(unsigned long long), cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaMemcpy(&h_min, d_min, sizeof(int),                cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaMemcpy(&h_max, d_max, sizeof(int),                cudaMemcpyDeviceToHost));

        double gpuTime  = gpuMs / 1000.0;
        double speedup  = cpuTime / gpuTime;
        // Efficiency relative to GPU SM count — get it at runtime
        int smCount;
        cudaDeviceGetAttribute(&smCount, cudaDevAttrMultiProcessorCount, 0);
        double efficiency = speedup / smCount;

        cout << fixed << setprecision(6)
             << left
             << setw(12) << n
             << setw(14) << cpuTime
             << setw(14) << gpuTime
             << setw(10) << fixed << setprecision(2) << speedup
             << setw(12) << fixed << setprecision(4) << efficiency
             << setw(14) << cs
             << setw(14) << (long long)h_sum
             << setw(10) << cm
             << setw(10) << (int)h_min
             << setw(10) << cx
             << setw(10) << (int)h_max
             << "\n";

        csv << n << "," << cpuTime << "," << gpuTime << ","
            << speedup << "," << efficiency << ","
            << cs << "," << (long long)h_sum << ","
            << cm << "," << (int)h_min << ","
            << cx << "," << (int)h_max << "\n";

        cudaFree(d_input);
        cudaFree(d_sum);
        cudaFree(d_min);
        cudaFree(d_max);
        cudaEventDestroy(ev1);
        cudaEventDestroy(ev2);
    }

    csv.close();
    cout << "\nResults saved to reduction_results.csv\n";
    return 0;
}
