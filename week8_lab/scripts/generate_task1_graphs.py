#!/usr/bin/env python3
"""Generate presentation-ready FIT3143 Lab 2 Task 1 performance graphs.

The script consumes the existing benchmark CSV files without modifying them.
OpenMP speedup is calculated against the same recorded serial baseline used by
the MPI experiment, because the OpenMP CSV files only store parallel time.

Usage (from week8_lab):
    python3 scripts/generate_task1_graphs.py

Output PNGs are written to results/graphs/.
"""

from __future__ import annotations

import csv
import sys
from pathlib import Path
from typing import Iterable

from derive_task3_task1_metrics import derive_all

try:
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
except ModuleNotFoundError as error:
    raise SystemExit(
        "Matplotlib is required. Activate the plotting virtual environment and "
        "install it with: python3 -m pip install matplotlib"
    ) from error


PROJECT_ROOT = Path(__file__).resolve().parent.parent
RESULTS_DIR = PROJECT_ROOT / "results"
OUTPUT_DIR = RESULTS_DIR / "graphs"
FIGURE_SIZE = (12.8, 7.2)  # 16:9 at 150 DPI is exactly 1920 x 1080 pixels.
DPI = 150
MPI_COLOUR = "#0072B2"
OPENMP_COLOUR = "#D55E00"
EMPIRICAL_COLOUR = "#009E73"
THEORETICAL_COLOUR = "#CC79A7"


def read_csv(filename: str, required_columns: set[str]) -> list[dict[str, float]]:
    """Read a numeric CSV and verify its required schema and values."""
    path = RESULTS_DIR / filename

    try:
        with path.open(newline="", encoding="utf-8") as file_handle:
            reader = csv.DictReader(file_handle)
            if reader.fieldnames is None:
                raise ValueError("CSV is missing a header row")

            missing_columns = required_columns - set(reader.fieldnames)
            if missing_columns:
                missing = ", ".join(sorted(missing_columns))
                raise ValueError(f"CSV is missing required column(s): {missing}")

            rows: list[dict[str, float]] = []
            for line_number, row in enumerate(reader, start=2):
                converted_row: dict[str, float] = {}
                for column, value in row.items():
                    if value is None or value.strip() == "":
                        raise ValueError(f"empty value in column '{column}' on line {line_number}")
                    try:
                        converted_row[column] = float(value)
                    except ValueError as error:
                        raise ValueError(
                            f"non-numeric value '{value}' in column '{column}' "
                            f"on line {line_number}"
                        ) from error
                rows.append(converted_row)
    except OSError as error:
        raise ValueError(f"could not read {path}: {error}") from error

    if not rows:
        raise ValueError(f"{path} contains no data rows")

    for row in rows:
        for column in required_columns:
            if row[column] <= 0:
                raise ValueError(f"{path}: '{column}' values must be positive")

    return rows


def index_by(rows: Iterable[dict[str, float]], key: str, description: str) -> dict[int, dict[str, float]]:
    """Index rows by an integer key, rejecting duplicate non-integral values."""
    indexed: dict[int, dict[str, float]] = {}

    for row in rows:
        key_as_float = row[key]
        if not key_as_float.is_integer():
            raise ValueError(f"{description}: '{key}' must be an integer")
        key_as_int = int(key_as_float)
        if key_as_int in indexed:
            raise ValueError(f"{description}: duplicate '{key}' value {key_as_int}")
        indexed[key_as_int] = row

    return indexed


def validate_matching_keys(
    first: dict[int, dict[str, float]],
    second: dict[int, dict[str, float]],
    key_description: str,
) -> list[int]:
    """Return sorted shared keys, failing if either dataset has an unmatched key."""
    if set(first) != set(second):
        only_first = sorted(set(first) - set(second))
        only_second = sorted(set(second) - set(first))
        raise ValueError(
            f"{key_description} values do not match "
            f"(only first: {only_first}; only second: {only_second})"
        )
    return sorted(first)


def new_figure() -> tuple[plt.Figure, plt.Axes]:
    """Create a consistent widescreen presentation figure."""
    figure, axes = plt.subplots(figsize=FIGURE_SIZE, dpi=DPI)
    figure.patch.set_facecolor("white")
    axes.set_facecolor("#FAFAFA")
    axes.grid(True, linestyle="--", linewidth=0.8, alpha=0.55)
    axes.tick_params(labelsize=13)
    return figure, axes


def save_figure(figure: plt.Figure, filename: str) -> None:
    """Save one graph and release its resources."""
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    figure.tight_layout(pad=1.4)
    figure.savefig(OUTPUT_DIR / filename, dpi=DPI)
    plt.close(figure)


def plot_runtime_vs_n(
    mpi_by_n: dict[int, dict[str, float]], openmp_by_n: dict[int, dict[str, float]], n_values: list[int]
) -> None:
    figure, axes = new_figure()
    n_millions = [n / 1_000_000 for n in n_values]

    axes.plot(
        n_millions,
        [mpi_by_n[n]["mpi_time"] for n in n_values],
        marker="o",
        linewidth=2.7,
        markersize=5,
        color=MPI_COLOUR,
        label="MPI (4 processes)",
    )
    axes.plot(
        n_millions,
        [openmp_by_n[n]["openmp_time"] for n in n_values],
        marker="s",
        linewidth=2.7,
        markersize=5,
        color=OPENMP_COLOUR,
        label="OpenMP (4 threads)",
    )
    axes.set_title("Graph 1: Overall Runtime vs Prime-Search Limit", fontsize=20, weight="bold", pad=16)
    axes.set_xlabel("Prime-search limit n (millions)", fontsize=15)
    axes.set_ylabel("Overall wall-clock time (seconds)", fontsize=15)
    axes.legend(fontsize=13, frameon=True)
    save_figure(figure, "graph1_runtime_vs_n.png")


def plot_speedup_vs_n(
    mpi_by_n: dict[int, dict[str, float]], openmp_by_n: dict[int, dict[str, float]], n_values: list[int]
) -> None:
    figure, axes = new_figure()
    n_millions = [n / 1_000_000 for n in n_values]
    openmp_speedups = [mpi_by_n[n]["serial_time"] / openmp_by_n[n]["openmp_time"] for n in n_values]

    axes.plot(
        n_millions,
        [mpi_by_n[n]["speedup"] for n in n_values],
        marker="o",
        linewidth=2.7,
        markersize=5,
        color=MPI_COLOUR,
        label="MPI (4 processes)",
    )
    axes.plot(
        n_millions,
        openmp_speedups,
        marker="s",
        linewidth=2.7,
        markersize=5,
        color=OPENMP_COLOUR,
        label="OpenMP (4 threads)",
    )
    axes.axhline(4, color="#666666", linestyle=":", linewidth=1.4, label="Ideal 4-worker speedup")
    axes.set_title("Graph 2: Overall-Time Speedup vs Prime-Search Limit", fontsize=20, weight="bold", pad=16)
    axes.set_xlabel("Prime-search limit n (millions)", fontsize=15)
    axes.set_ylabel("Speedup relative to recorded serial baseline", fontsize=15)
    axes.legend(fontsize=12, frameon=True)
    save_figure(figure, "graph2_speedup_vs_n.png")


def plot_speedup_vs_workers(
    mpi_by_worker: dict[int, dict[str, float]],
    openmp_by_worker: dict[int, dict[str, float]],
    workers: list[int],
) -> None:
    figure, axes = new_figure()
    openmp_speedups = [
        mpi_by_worker[worker]["serial_time"] / openmp_by_worker[worker]["openmp_time"]
        for worker in workers
    ]

    axes.plot(
        workers,
        [mpi_by_worker[worker]["speedup"] for worker in workers],
        marker="o",
        linewidth=2.7,
        markersize=8,
        color=MPI_COLOUR,
        label="MPI processes",
    )
    axes.plot(
        workers,
        openmp_speedups,
        marker="s",
        linewidth=2.7,
        markersize=8,
        color=OPENMP_COLOUR,
        label="OpenMP threads",
    )
    axes.plot(
        workers,
        workers,
        color="#666666",
        linestyle=":",
        linewidth=1.4,
        label="Ideal linear speedup",
    )
    axes.set_title("Graph 3: Overall-Time Speedup vs Worker Count (n = 60 million)", fontsize=20, weight="bold", pad=16)
    axes.set_xlabel("MPI processes / OpenMP threads (log₂ scale)", fontsize=15)
    axes.set_ylabel("Speedup relative to recorded serial baseline", fontsize=15)
    axes.set_xscale("log", base=2)
    axes.set_xticks(workers)
    axes.set_xticklabels(workers)
    axes.legend(fontsize=12, frameon=True)
    save_figure(figure, "graph3_speedup_vs_workers.png")


def plot_empirical_vs_theoretical(task3_rows: list[dict[str, float]]) -> None:
    by_process = index_by(task3_rows, "processes", "task3_process_scaling_aligned.csv")
    processes = sorted(by_process)
    figure, axes = new_figure()

    axes.plot(
        processes,
        [by_process[process]["empirical_speedup"] for process in processes],
        marker="o",
        linewidth=2.7,
        markersize=8,
        color=EMPIRICAL_COLOUR,
        label="Empirical speedup",
    )
    axes.plot(
        processes,
        [by_process[process]["theoretical_speedup"] for process in processes],
        marker="D",
        linewidth=2.7,
        markersize=7,
        color=THEORETICAL_COLOUR,
        label="Amdahl-style theoretical speedup (aligned baseline)",
    )
    axes.plot(
        processes,
        processes,
        color="#666666",
        linestyle=":",
        linewidth=1.4,
        label="Ideal linear speedup",
    )
    axes.set_title("Graph 6: Empirical vs Theoretical MPI Speedup (n = 60 million)", fontsize=20, weight="bold", pad=16)
    axes.set_xlabel("MPI processes (log₂ scale)", fontsize=15)
    axes.set_ylabel("Speedup", fontsize=15)
    axes.set_xscale("log", base=2)
    axes.set_xticks(processes)
    axes.set_xticklabels(processes)
    axes.legend(fontsize=12, frameon=True)
    save_figure(figure, "graph6_empirical_vs_theoretical.png")


def main() -> int:
    try:
        derive_all()
        mpi_n_rows = read_csv(
            "n_scaling.csv", {"n", "processes", "serial_time", "mpi_time", "speedup", "efficiency"}
        )
        openmp_n_rows = read_csv("openmp_n_scaling.csv", {"n", "threads", "openmp_time"})
        mpi_worker_rows = read_csv(
            "process_scaling.csv", {"n", "processes", "serial_time", "mpi_time", "speedup", "efficiency"}
        )
        openmp_worker_rows = read_csv("openmp_thread_scaling.csv", {"n", "threads", "openmp_time"})
        task3_rows = read_csv(
            "task3_process_scaling_aligned.csv",
            {"processes", "empirical_speedup", "theoretical_speedup"},
        )

        mpi_by_n = index_by(mpi_n_rows, "n", "n_scaling.csv")
        openmp_by_n = index_by(openmp_n_rows, "n", "openmp_n_scaling.csv")
        n_values = validate_matching_keys(mpi_by_n, openmp_by_n, "n")

        mpi_by_worker = index_by(mpi_worker_rows, "processes", "process_scaling.csv")
        openmp_by_worker = index_by(openmp_worker_rows, "threads", "openmp_thread_scaling.csv")
        workers = validate_matching_keys(mpi_by_worker, openmp_by_worker, "worker-count")

        plot_runtime_vs_n(mpi_by_n, openmp_by_n, n_values)
        plot_speedup_vs_n(mpi_by_n, openmp_by_n, n_values)
        plot_speedup_vs_workers(mpi_by_worker, openmp_by_worker, workers)
        plot_empirical_vs_theoretical(task3_rows)
    except (OSError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1

    print(f"Generated four graphs in: {OUTPUT_DIR}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
