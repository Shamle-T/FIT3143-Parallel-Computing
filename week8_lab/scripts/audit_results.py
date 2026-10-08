#!/usr/bin/env python3
"""Recalculate stored speedup, efficiency, and Amdahl values."""

import csv
import math
from pathlib import Path


RESULTS = Path(__file__).resolve().parents[1] / "results"
ROUNDING_TOLERANCE = 1.1e-6


def rows(name):
    with (RESULTS / name).open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def agrees(stored, calculated):
    return math.isclose(float(stored), float(calculated),
                        rel_tol=ROUNDING_TOLERANCE,
                        abs_tol=ROUNDING_TOLERANCE)


issues = []

n_rows = rows("n_scaling.csv")
for index, row in enumerate(n_rows, start=1):
    speedup = float(row["serial_time"]) / float(row["mpi_time"])
    if not agrees(row["speedup"], speedup):
        issues.append(f"n-scaling speedup row {index}")
    if not agrees(row["efficiency"], speedup / int(row["processes"])):
        issues.append(f"n-scaling efficiency row {index}")

task1_rows = rows("task1_process_scaling.csv")
task1_parallel = float(task1_rows[0]["compute_seconds"])
task1_serial = float(task1_rows[0]["mpi_seconds"]) - task1_parallel
for index, row in enumerate(task1_rows, start=1):
    workers = int(row["processes"])
    empirical = float(row["serial_seconds"]) / float(row["mpi_seconds"])
    theoretical = ((task1_serial + task1_parallel) /
                   (task1_serial + task1_parallel / workers))
    if not agrees(row["empirical_speedup"], empirical):
        issues.append(f"Task 1 empirical row {index}")
    if not agrees(row["theoretical_speedup"], theoretical):
        issues.append(f"Task 1 Amdahl row {index}")
    if not agrees(row["efficiency"], empirical / workers):
        issues.append(f"Task 1 efficiency row {index}")

hybrid_rows = rows("hybrid_scaling.csv")
hybrid_parallel = float(hybrid_rows[0]["compute_seconds"])
hybrid_serial = float(hybrid_rows[0]["hybrid_seconds"]) - hybrid_parallel
for index, row in enumerate(hybrid_rows, start=1):
    workers = int(row["total_workers"])
    empirical = float(row["serial_seconds"]) / float(row["hybrid_seconds"])
    versus_mpi = float(row["mpi_seconds"]) / float(row["hybrid_seconds"])
    theoretical = ((hybrid_serial + hybrid_parallel) /
                   (hybrid_serial + hybrid_parallel / workers))
    if not agrees(row["empirical_speedup"], empirical):
        issues.append(f"hybrid empirical row {index}")
    if not agrees(row["vs_mpi_speedup"], versus_mpi):
        issues.append(f"hybrid versus-MPI row {index}")
    if not agrees(row["theoretical_speedup"], theoretical):
        issues.append(f"hybrid Amdahl row {index}")
    if not agrees(row["efficiency"], empirical / workers):
        issues.append(f"hybrid efficiency row {index}")

print(f"n rows={len(n_rows)}, Task 1 rows={len(task1_rows)}, "
      f"hybrid rows={len(hybrid_rows)}")
print(f"Task 1: Ts={task1_serial:.9f}, Tp={task1_parallel:.9f}, "
      f"fs={task1_serial / (task1_serial + task1_parallel):.9f}")
print(f"Task 2: Ts={hybrid_serial:.9f}, Tp={hybrid_parallel:.9f}, "
      f"fs={hybrid_serial / (hybrid_serial + hybrid_parallel):.9f}")

if issues:
    raise SystemExit("FAIL: " + ", ".join(issues))

print("PASS: all stored calculations agree within displayed rounding.")
