# Assignment 5: Mandelbrot Fractal using CUDA Python (Numba)
# Install: pip install numba numpy matplotlib
# Run:     python3 mandelbrot.py

import numpy as np
from numba import cuda
import matplotlib.pyplot as plt
import time

WIDTH  = 1920
HEIGHT = 1080
MAX_ITER = 256

MIN_X, MAX_X = -2.5, 1.0
MIN_Y, MAX_Y = -1.25, 1.25

# ──────────────────── CPU Implementation ───────────────────────────

def mandelbrot_cpu(min_x, max_x, min_y, max_y, width, height, max_iter):
    image = np.zeros((height, width), dtype=np.int32)
    for y in range(height):
        for x in range(width):
            cr = min_x + x * (max_x - min_x) / width
            ci = min_y + y * (max_y - min_y) / height
            zr, zi = 0.0, 0.0
            for i in range(max_iter):
                zr2, zi2 = zr * zr, zi * zi
                if zr2 + zi2 > 4.0:
                    image[y, x] = i
                    break
                zi = 2.0 * zr * zi + ci
                zr = zr2 - zi2 + cr
            else:
                image[y, x] = max_iter
    return image

# ──────────────────── GPU Kernel ───────────────────────────────────

@cuda.jit
def mandelbrot_gpu(min_x, max_x, min_y, max_y, image, max_iter):
    height, width = image.shape
    x, y = cuda.grid(2)
    if x < width and y < height:
        cr = min_x + x * (max_x - min_x) / width
        ci = min_y + y * (max_y - min_y) / height
        zr, zi = 0.0, 0.0
        for i in range(max_iter):
            zr2 = zr * zr
            zi2 = zi * zi
            if zr2 + zi2 > 4.0:
                image[y, x] = i
                return
            zi = 2.0 * zr * zi + ci
            zr = zr2 - zi2 + cr
        image[y, x] = max_iter

# ──────────────────── Main ─────────────────────────────────────────

print("Neeti Kurulkar BE A 41038\n")

# ── CPU ──
# CPU is slow for full resolution — use smaller size for timing
cpu_w, cpu_h = 480, 270
print(f"Running CPU Mandelbrot ({cpu_w}x{cpu_h})...")
t1 = time.time()
cpu_image = mandelbrot_cpu(MIN_X, MAX_X, MIN_Y, MAX_Y, cpu_w, cpu_h, MAX_ITER)
cpu_time = time.time() - t1
print(f"CPU Time: {cpu_time:.4f}s")

# Scale CPU time to full resolution for a fair comparison estimate
scale = (WIDTH * HEIGHT) / (cpu_w * cpu_h)
cpu_time_estimated = cpu_time * scale
print(f"CPU estimated time at {WIDTH}x{HEIGHT}: {cpu_time_estimated:.2f}s")

# ── GPU ──
print(f"\nRunning GPU Mandelbrot ({WIDTH}x{HEIGHT})...")
gpu_image = np.zeros((HEIGHT, WIDTH), dtype=np.int32)
d_image   = cuda.to_device(gpu_image)

threads_per_block = (16, 16)
blocks_x = (WIDTH  + 15) // 16
blocks_y = (HEIGHT + 15) // 16
blocks   = (blocks_x, blocks_y)

# Warm-up run (first CUDA call has JIT overhead)
mandelbrot_gpu[blocks, threads_per_block](MIN_X, MAX_X, MIN_Y, MAX_Y, d_image, MAX_ITER)
cuda.synchronize()

t2 = time.time()
mandelbrot_gpu[blocks, threads_per_block](MIN_X, MAX_X, MIN_Y, MAX_Y, d_image, MAX_ITER)
cuda.synchronize()
gpu_time = time.time() - t2

gpu_image = d_image.copy_to_host()
print(f"GPU Time: {gpu_time:.4f}s")

# ── Results ──
speedup    = cpu_time_estimated / gpu_time
print(f"\n{'─'*45}")
print(f"{'Metric':<25} {'Value':>18}")
print(f"{'─'*45}")
print(f"{'CPU Time (estimated)':<25} {cpu_time_estimated:>17.4f}s")
print(f"{'GPU Time':<25} {gpu_time:>17.4f}s")
print(f"{'Speedup':<25} {speedup:>17.2f}x")
print(f"{'Resolution':<25} {f'{WIDTH}x{HEIGHT}':>18}")
print(f"{'Max Iterations':<25} {MAX_ITER:>18}")
print(f"{'─'*45}")

# ── Plot ──
fig, axes = plt.subplots(1, 2, figsize=(16, 5))
fig.suptitle("Mandelbrot Fractal — CPU vs GPU (Numba CUDA)", fontsize=14)

axes[0].imshow(cpu_image, cmap='inferno', extent=[MIN_X, MAX_X, MIN_Y, MAX_Y])
axes[0].set_title(f"CPU ({cpu_w}x{cpu_h}), {cpu_time:.3f}s")
axes[0].set_xlabel("Re"); axes[0].set_ylabel("Im")

axes[1].imshow(gpu_image, cmap='inferno', extent=[MIN_X, MAX_X, MIN_Y, MAX_Y])
axes[1].set_title(f"GPU ({WIDTH}x{HEIGHT}), {gpu_time:.3f}s")
axes[1].set_xlabel("Re"); axes[1].set_ylabel("Im")

plt.tight_layout()
plt.savefig("mandelbrot.png", dpi=150)
plt.show()
print("Saved mandelbrot.png")
