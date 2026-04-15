import csv
import matplotlib.pyplot as plt

vertices = []
seq_times = []
par_times = []
speedups = []

with open("bfs_results.csv") as f:
    reader = csv.DictReader(f)
    for row in reader:
        vertices.append(int(float(row["vertices"])))
        seq_times.append(float(row["seq_time"]))
        par_times.append(float(row["par_time"]))
        speedups.append(float(row["speedup"]))

labels = [str(v) for v in vertices]
x = range(len(labels))

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
fig.suptitle("BFS: Sequential vs Parallel Performance", fontsize=14)

# Time comparison
ax1.plot(labels, seq_times, marker='o', label='Sequential', color='blue')
ax1.plot(labels, par_times, marker='s', label='Parallel', color='orange')
ax1.set_xlabel("Number of Vertices")
ax1.set_ylabel("Time (seconds)")
ax1.set_title("Execution Time")
ax1.legend()
ax1.grid(True)

# Speedup
ax2.plot(labels, speedups, marker='^', color='green', label='Speedup')
ax2.axhline(y=1.0, color='red', linestyle='--', label='Speedup = 1 (baseline)')
ax2.set_xlabel("Number of Vertices")
ax2.set_ylabel("Speedup (Seq Time / Par Time)")
ax2.set_title("Speedup")
ax2.legend()
ax2.grid(True)

plt.tight_layout()
plt.savefig("bfs_speedup.png", dpi=150)
plt.show()
print("Graph saved as bfs_speedup.png")
