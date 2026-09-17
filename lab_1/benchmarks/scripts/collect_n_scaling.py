from __future__ import annotations

import csv
import re
import subprocess
from pathlib import Path

THREAD_COUNT = 20
INPUT_LIMITS = range(1_000_000, 30_000_001, 1_000_000)
LAB_ROOT = Path(__file__).resolve().parents[2]
DATA_DIRECTORY = LAB_ROOT / "benchmarks" / "data"
BUILD_DIRECTORY = LAB_ROOT / "bin"


def compile_program(source_name: str, executable_name: str, extra_flags: list[str]) -> Path:
    executable = BUILD_DIRECTORY / executable_name
    subprocess.run(
        [
            "gcc",
            "-std=c11",
            "-O3",
            "-Wall",
            "-Wextra",
            "-pedantic",
            str(LAB_ROOT / source_name),
            "-o",
            str(executable),
            *extra_flags,
        ],
        check=True,
    )
    return executable


def run_benchmark(executable: Path, *arguments: int) -> str:
    completed = subprocess.run(
        [str(executable), "--benchmark", *(str(argument) for argument in arguments)],
        check=True,
        capture_output=True,
        text=True,
    )
    return completed.stdout


def value(output: str, label: str, integer: bool = False) -> int | float:
    match = re.search(rf"^{re.escape(label)}: ([0-9.]+)", output, re.MULTILINE)
    if match is None:
        raise ValueError(f"Missing '{label}' in benchmark output:\n{output}")
    return int(match.group(1)) if integer else float(match.group(1))


def main() -> None:
    task1 = compile_program("task1.c", "task1_n_scaling_benchmark.exe", ["-lm"])
    task2 = compile_program("task2.c", "task2_n_scaling_benchmark.exe", ["-pthread", "-lm"])
    task3 = compile_program("task3.c", "task3_n_scaling_benchmark.exe", ["-fopenmp", "-lm"])

    task1_rows: list[dict[str, int | str]] = []
    task2_rows: list[dict[str, int | str]] = []
    task3_rows: list[dict[str, int | str]] = []

    for limit in INPUT_LIMITS:
        task1_output = run_benchmark(task1, limit)
        task2_output = run_benchmark(task2, limit, THREAD_COUNT)
        task3_output = run_benchmark(task3, limit, THREAD_COUNT)

        prime_count = value(task1_output, "Prime count", integer=True)
        if (value(task2_output, "Prime count", integer=True) != prime_count or
                value(task3_output, "Prime count", integer=True) != prime_count):
            raise ValueError(f"Prime-count mismatch at n={limit}")

        task1_rows.append(
            {
                "n": limit,
                "implementation": "serial",
                "run": 1,
                "prime_count": prime_count,
                "computation_seconds": f"{value(task1_output, 'Computation time'):.9f}",
            }
        )
        task2_rows.append(
            {
                "n": limit,
                "thread_count": THREAD_COUNT,
                "run": 1,
                "prime_count": prime_count,
                "serial_seconds": f"{value(task2_output, 'Serial computation time'):.9f}",
                "parallel_seconds": f"{value(task2_output, 'Pthreads computation time'):.9f}",
                "speedup": f"{value(task2_output, 'Speedup'):.6f}",
                "efficiency": f"{value(task2_output, 'Efficiency'):.6f}",
            }
        )
        task3_rows.append(
            {
                "n": limit,
                "thread_count": THREAD_COUNT,
                "run": 1,
                "prime_count": prime_count,
                "serial_seconds": f"{value(task3_output, 'Serial computation time'):.9f}",
                "parallel_seconds": f"{value(task3_output, 'OpenMP computation time'):.9f}",
                "speedup": f"{value(task3_output, 'Speedup'):.6f}",
                "efficiency": f"{value(task3_output, 'Efficiency'):.6f}",
            }
        )
        print(f"Completed n={limit:,}", flush=True)

    with (DATA_DIRECTORY / "task1_n_scaling_threads20.csv").open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(
            file,
            fieldnames=["n", "implementation", "run", "prime_count", "computation_seconds"],
        )
        writer.writeheader()
        writer.writerows(task1_rows)

    parallel_fieldnames = [
        "n",
        "thread_count",
        "run",
        "prime_count",
        "serial_seconds",
        "parallel_seconds",
        "speedup",
        "efficiency",
    ]
    for filename, rows in [
        ("task2_n_scaling_threads20.csv", task2_rows),
        ("task3_n_scaling_threads20.csv", task3_rows),
    ]:
        with (DATA_DIRECTORY / filename).open("w", newline="", encoding="utf-8") as file:
            writer = csv.DictWriter(file, fieldnames=parallel_fieldnames)
            writer.writeheader()
            writer.writerows(rows)

    with (DATA_DIRECTORY / "pthread_openmp_n_scaling_threads20.csv").open(
        "w", newline="", encoding="utf-8"
    ) as file:
        writer = csv.DictWriter(
            file,
            fieldnames=[
                "n",
                "thread_count",
                "run",
                "prime_count",
                "pthreads_seconds",
                "openmp_seconds",
                "pthreads_speedup",
                "openmp_speedup",
            ],
        )
        writer.writeheader()
        for pthread_row, openmp_row in zip(task2_rows, task3_rows, strict=True):
            writer.writerow(
                {
                    "n": pthread_row["n"],
                    "thread_count": THREAD_COUNT,
                    "run": 1,
                    "prime_count": pthread_row["prime_count"],
                    "pthreads_seconds": pthread_row["parallel_seconds"],
                    "openmp_seconds": openmp_row["parallel_seconds"],
                    "pthreads_speedup": pthread_row["speedup"],
                    "openmp_speedup": openmp_row["speedup"],
                }
            )


if __name__ == "__main__":
    main()
