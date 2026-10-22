"""Generate the seven required Task 4 figures from measured CSV files."""
from pathlib import Path
import csv
import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parents[1]
RESULTS = ROOT / "results"
OUT = RESULTS / "charts"
OUT.mkdir(exist_ok=True)

COLORS = {
    "serial": "#4C78A8",
    "mpi": "#E45756",
    "openmp": "#59A14F",
    "hybrid": "#B279A2",
    "theory": "#F28E2B",
}


def rows(name):
    with (RESULTS / name).open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def save_line(x, series, xlabel, ylabel, title, filename, xlabels=None):
    fig, axis = plt.subplots(figsize=(8.4, 4.8))
    for label, values, color in series:
        axis.plot(x, values, marker="o", markersize=4.5, linewidth=2,
                  label=label, color=color)
    axis.set_xlabel(xlabel)
    axis.set_ylabel(ylabel)
    axis.set_title(title, pad=10, weight="bold")
    axis.grid(True, alpha=0.25)
    if xlabels is not None:
        axis.set_xticks(x, xlabels)
    axis.legend(frameon=False)
    fig.tight_layout()
    fig.savefig(OUT / filename, dpi=200, facecolor="white")
    plt.close(fig)


# Graphs 1 and 2: 30 distinct n values, Task 1 versus Week 4 OpenMP.
n_data = rows("n_scaling.csv")
openmp_n = rows("openmp_n_scaling.csv")
openmp_baseline_n = rows("task3_n_scaling.csv")
if len(n_data) != 30 or len(openmp_n) != 30 or len(openmp_baseline_n) != 30:
    raise ValueError("The n-scaling figures require exactly 30 measured n values.")
if not ([r["n"] for r in n_data] == [r["n"] for r in openmp_n]
        == [r["n"] for r in openmp_baseline_n]):
    raise ValueError("Task 1 and OpenMP n values do not match.")

x_n = [int(r["n"]) / 1e6 for r in n_data]
serial_n = [float(r["serial_time"]) for r in n_data]
mpi_n = [float(r["mpi_time"]) for r in n_data]
openmp_n_times = [float(r["openmp_time"]) for r in openmp_n]
openmp_serial_n = [float(r["serial_total"]) for r in openmp_baseline_n]

save_line(
    x_n,
    [
        ("Serial", serial_n, COLORS["serial"]),
        ("Open MPI, 4 processes", mpi_n, COLORS["mpi"]),
        ("OpenMP, 4 threads", openmp_n_times, COLORS["openmp"]),
    ],
    "n (millions)", "End-to-end runtime (s)",
    "Runtime across 30 problem sizes", "01_runtime_n.png",
)
save_line(
    x_n,
    [
        ("Open MPI", [s / p for s, p in zip(serial_n, mpi_n)], COLORS["mpi"]),
        ("OpenMP", [s / p for s, p in zip(openmp_serial_n, openmp_n_times)],
         COLORS["openmp"]),
    ],
    "n (millions)", "Empirical speedup against serial",
    "Empirical speedup across 30 problem sizes", "02_speedup_n.png",
)


# Graph 3: matching MPI-process and OpenMP-thread counts.
task1 = rows("task1_process_scaling.csv")
openmp_threads = rows("openmp_thread_scaling.csv")
comparison_counts = [1, 2, 4, 8, 16]
task1_by_process = {int(r["processes"]): r for r in task1}
openmp_by_threads = {int(r["threads"]): r for r in openmp_threads}
serial_reference = float(task1[0]["serial_seconds"])

save_line(
    comparison_counts,
    [
        ("Open MPI processes",
         [float(task1_by_process[p]["empirical_speedup"])
          for p in comparison_counts], COLORS["mpi"]),
        ("OpenMP threads",
         [serial_reference / float(openmp_by_threads[p]["openmp_time"])
          for p in comparison_counts], COLORS["openmp"]),
    ],
    "Processes or threads", "Empirical speedup against serial",
    "Task 1 and OpenMP scaling at n = 60 million",
    "03_task1_process.png",
)


# Graph 4: increasing OpenMP threads at a fixed four MPI processes.
hybrid = rows("hybrid_scaling.csv")
hybrid_lookup = {
    (int(r["processes"]), int(r["threads_per_process"])): r
    for r in hybrid
}
fixed_four = [hybrid_lookup[(4, t)] for t in comparison_counts]
save_line(
    list(range(len(comparison_counts))),
    [("Hybrid relative to Task 1",
      [float(r["vs_mpi_speedup"]) for r in fixed_four], COLORS["hybrid"])],
    "OpenMP threads per MPI process", "Speedup relative to 4-process Task 1",
    "Task 2 benefit at four MPI processes", "04_hybrid_threads.png",
    xlabels=[str(value) for value in comparison_counts],
)


# Graph 5: matched total workers while both MPI processes and threads increase.
matched_path = [(1, 1), (2, 1), (2, 2), (4, 2), (4, 4)]
matched_rows = [hybrid_lookup[key] for key in matched_path]
matched_workers = [int(r["total_workers"]) for r in matched_rows]
matched_labels = [f"{w}\n({p}x{t})" for w, (p, t) in zip(matched_workers, matched_path)]
hybrid_serial = float(hybrid[0]["serial_seconds"])
save_line(
    list(range(len(matched_workers))),
    [
        ("Hybrid MPI x OpenMP",
         [float(r["empirical_speedup"]) for r in matched_rows], COLORS["hybrid"]),
        ("OpenMP, matched threads",
         [hybrid_serial / float(openmp_by_threads[w]["openmp_time"])
          for w in matched_workers], COLORS["openmp"]),
    ],
    "Total workers (hybrid configuration)", "Empirical speedup against serial",
    "Hybrid and OpenMP at matched worker counts", "05_hybrid_openmp.png",
    xlabels=matched_labels,
)


# Graph 6: Task 1 empirical versus Amdahl speedup for every process count 1-16.
process_counts = [int(r["processes"]) for r in task1]
save_line(
    process_counts,
    [
        ("Empirical vs Week 4 serial",
         [float(r["empirical_speedup"]) for r in task1], COLORS["mpi"]),
        ("Amdahl ideal from 1-process decomposition",
         [float(r["theoretical_speedup"]) for r in task1], COLORS["theory"]),
    ],
    "MPI processes", "Speedup",
    "Task 1 empirical and Amdahl speedup", "06_task1_amdahl.png",
)


# Graph 7: unique worker counts along a non-decreasing MPI/thread path.
amdahl_path = [(1, 1), (2, 1), (2, 2), (4, 2), (4, 4), (4, 8), (4, 16)]
amdahl_rows = [hybrid_lookup[key] for key in amdahl_path]
amdahl_workers = [int(r["total_workers"]) for r in amdahl_rows]
amdahl_labels = [f"{w}\n({p}x{t})" for w, (p, t) in zip(amdahl_workers, amdahl_path)]
save_line(
    list(range(len(amdahl_workers))),
    [
        ("Hybrid empirical vs Week 4 serial",
         [float(r["empirical_speedup"]) for r in amdahl_rows], COLORS["hybrid"]),
        ("Amdahl ideal from 1-worker decomposition",
         [float(r["theoretical_speedup"]) for r in amdahl_rows], COLORS["theory"]),
    ],
    "Total workers (hybrid configuration)", "Speedup",
    "Task 2 empirical and Amdahl speedup", "07_hybrid_amdahl.png",
    xlabels=amdahl_labels,
)

print(f"Wrote seven required charts to {OUT}")
