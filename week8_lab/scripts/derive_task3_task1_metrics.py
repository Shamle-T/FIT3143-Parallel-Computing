#!/usr/bin/env python3
"""Derive baseline-aligned Task 3 metrics from Task 1 timing measurements.

The original CSV files are retained as raw benchmark evidence.  This script
creates separate ``*_aligned.csv`` files whose theoretical and empirical
speedups use the same serial-total baseline:

    S_theory(P) = T_serial / (T_serial_part + T_parallel / P + T_comm(P))

where T_parallel is the serial computation time, T_serial_part is the
remaining serial total time, and T_comm(P) is measured MPI communication.
"""

from __future__ import annotations

import csv
import math
from pathlib import Path


RESULTS_DIR = Path(__file__).resolve().parent.parent / "results"
REQUIRED_COLUMNS = {
    "processes",
    "serial_compute",
    "serial_total",
    "mpi_compute",
    "communication",
    "postprocess",
    "mpi_total",
}


def read_rows(path: Path) -> tuple[list[dict[str, str]], list[str]]:
    """Read and validate one raw timing CSV without modifying it."""
    with path.open(newline="", encoding="utf-8") as file_handle:
        reader = csv.DictReader(file_handle)
        if reader.fieldnames is None:
            raise ValueError(f"{path.name} has no header row")
        missing = REQUIRED_COLUMNS - set(reader.fieldnames)
        if missing:
            raise ValueError(f"{path.name} is missing: {', '.join(sorted(missing))}")
        rows = list(reader)

    if not rows:
        raise ValueError(f"{path.name} has no data rows")
    return rows, reader.fieldnames


def numeric_value(
    row: dict[str, str],
    column: str,
    source_name: str,
    line_number: int,
    must_be_positive: bool = False,
) -> float:
    """Convert one finite timing or worker-count field."""
    try:
        value = float(row[column])
    except (KeyError, TypeError, ValueError) as error:
        raise ValueError(f"{source_name}, line {line_number}: invalid {column}") from error

    if not math.isfinite(value) or value < 0.0:
        raise ValueError(f"{source_name}, line {line_number}: {column} must be non-negative")
    if must_be_positive and value <= 0.0:
        raise ValueError(f"{source_name}, line {line_number}: {column} must be positive")
    return value


def derive_file(source_name: str, destination_name: str) -> None:
    """Write one aligned analysis CSV from an existing raw timing CSV."""
    source_path = RESULTS_DIR / source_name
    destination_path = RESULTS_DIR / destination_name
    rows, source_fields = read_rows(source_path)
    leading_fields = [field for field in ("n", "processes") if field in source_fields]
    output_fields = leading_fields + [
        "serial_compute",
        "serial_total",
        "mpi_compute",
        "communication",
        "postprocess",
        "mpi_total",
        "parallel_fraction",
        "serial_fraction",
        "communication_fraction",
        "empirical_speedup",
        "theoretical_speedup",
    ]

    with destination_path.open("w", newline="", encoding="utf-8") as file_handle:
        writer = csv.DictWriter(file_handle, fieldnames=output_fields)
        writer.writeheader()

        for line_number, row in enumerate(rows, start=2):
            process_count = numeric_value(row, "processes", source_name, line_number, True)
            serial_compute = numeric_value(row, "serial_compute", source_name, line_number, True)
            serial_total = numeric_value(row, "serial_total", source_name, line_number, True)
            communication = numeric_value(row, "communication", source_name, line_number)
            mpi_total = numeric_value(row, "mpi_total", source_name, line_number, True)

            if not process_count.is_integer():
                raise ValueError(f"{source_name}, line {line_number}: processes must be an integer")
            if serial_compute > serial_total:
                raise ValueError(
                    f"{source_name}, line {line_number}: serial_compute exceeds serial_total"
                )

            parallel_fraction = serial_compute / serial_total
            serial_fraction = 1.0 - parallel_fraction
            communication_fraction = communication / serial_total
            empirical_speedup = serial_total / mpi_total
            theoretical_speedup = 1.0 / (
                serial_fraction
                + parallel_fraction / process_count
                + communication_fraction
            )

            output_row = {field: row[field] for field in leading_fields}
            for field in ("serial_compute", "serial_total", "mpi_compute", "communication", "postprocess", "mpi_total"):
                numeric_value(row, field, source_name, line_number)
                output_row[field] = row[field]
            output_row.update(
                {
                    "parallel_fraction": f"{parallel_fraction:.9f}",
                    "serial_fraction": f"{serial_fraction:.9f}",
                    "communication_fraction": f"{communication_fraction:.9f}",
                    "empirical_speedup": f"{empirical_speedup:.9f}",
                    "theoretical_speedup": f"{theoretical_speedup:.9f}",
                }
            )
            writer.writerow(output_row)


def derive_all() -> None:
    """Create aligned Task 1 analysis files for process and n scaling."""
    derive_file("task3_process_scaling.csv", "task3_process_scaling_aligned.csv")
    derive_file("task3_n_scaling.csv", "task3_n_scaling_aligned.csv")


def main() -> int:
    try:
        derive_all()
    except (OSError, ValueError) as error:
        print(f"Error: {error}")
        return 1

    print(f"Generated aligned Task 3 Task 1 metrics in: {RESULTS_DIR}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
