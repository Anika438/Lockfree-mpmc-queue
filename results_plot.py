import pandas as pd
import matplotlib.pyplot as plt

# Read benchmark data
df = pd.read_csv("benchmark_results.csv")

# Compute average throughput
avg = df.groupby("Queue")["Throughput"].mean()

# Plot
plt.figure(figsize=(6,4))
avg.plot(kind="bar")

plt.ylabel("Throughput (ops/sec)")
plt.title("Mutex Queue vs Lock-Free Queue")

plt.tight_layout()
plt.savefig("benchmark_results.png")

plt.show()