import matplotlib.pyplot as plt
import numpy as np

# === File path (change this to your actual file) ===
filename = "capture.txt"

# === Load data ===
data = np.loadtxt(filename, delimiter=";")

# Create a time axis in milliseconds
time = np.arange(len(data))  # each row is 1 ms

# === Plot ===
plt.figure(figsize=(12, 6))

for i in range(data.shape[1]):  # plot each channel
    plt.plot(time, data[:, i], marker='.', label=f"Channel {i+1}")

plt.xlabel("Time (ms)")
plt.ylabel("Analog Value")
plt.title("Analog Data Visualization")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()