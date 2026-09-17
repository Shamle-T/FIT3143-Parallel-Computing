from __future__ import annotations

import csv
import re
import subprocess
from pathlib import Path

LIMIT = 10_000_000
THREAD_COUNTS = range(1, 41)
LAB_ROOT = Path(__file__).resolve().parents[2]
DATA_DIRECTORY = LAB_ROOT / "benchmarks" / "data"
BUILD_DIRECTORY = LAB_ROOT / "bin"


def compile_program(source_name: str, executable_name: str, extra_flags: list[str]) -> Path:
    source = LAB_ROOT / source_name
    executable = BUILD_DIRECTORY / executable_name
    command = [
        "gcc",
        "-std=c11",
        "-O3",
        "-Wall",
        "-Wextra",
        "-pedantic",
        str(source),
        "-o",
        str(executable),
        *extra_flags,
    ]
    subprocess.run(command, check=True)
    return executable


def run_benchmark(executable: Path, *arguments: int) -> str:
    completed = subprocess.run(
        [str(executable), "--benchmark", *(str(argument) for argument in arguments)],
        check=True,
        capture_output=True,
        text=True,
    )
    return completed.stdout


def number(output: str, label: str, integer: bool = False) -> int | float:
    match = re.search(rf"^{re.escape(label)}: ([0-9.]+)", output, re.MULTILINE)
    if match is None:
        raise ValueError(f"Missing '{label}' in benchmark output:\n{output}")
    return int(match.group(1)) if integer else float(match.group(1))


def main() -> None:
    task1 = compile_program("task1.c", "task1_benchmark.exe", ["-lm"])
    task2 = compile_program("task2.c", "task2_benchmark.exe", ["-pthread", "-lm"])
    task3 = compile_program("task3.c", "task3_benchmark.exe", ["-fopenmp", "-lm"])

    task1_output = run_benchmark(task1, LIMIT)
    expected_prime_count = number(task1_output, "Prime count", integer=True)
    task1_seconds = number(task1_output, "Computation time")

    with (DATA_DIRECTORY / "task1_n10000000.csv").open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(
            file,
            fieldnames=["n", "implementation", "run", "prime_count", "computation_seconds"],
        )
        writer.writeheader()
        writer.writerow(
            {
                "n": LIMIT,
                "implementation": "serial",
                "run": 1,
                "prime_count": expected_prime_count,
                "computation_seconds": f"{task1_seconds:.9f}",
            }
        )

    task2_rows: list[dict[str, int | str]] = []
    task3_rows: list[dict[str, int | str]] = []

    for thread_count in THREAD_COUNTS:
        task2_output = run_benchmark(task2, LIMIT, thread_count)
        task3_output = run_benchmark(task3, LIMIT, thread_count)

        task2_prime_count = number(task2_output, "Prime count", integer=True)
        task3_prime_count = number(task3_output, "Prime count", integer=True)
        if task2_prime_count != expected_prime_count or task3_prime_count != expected_prime_count:
            raise ValueError(f"Prime-count mismatch at {thread_count} threads")

        task2_rows.append(
            {
                "n": LIMIT,
                "thread_count": thread_count,
                "run": 1,
                "prime_count": task2_prime_count,
                "serial_seconds": f"{number(task2_output, 'Serial computation time'):.9f}",
                "parallel_seconds": f"{number(task2_output, 'Pthreads computation time'):.9f}",
                "speedup": f"{number(task2_output, 'Speedup'):.6f}",
                "efficiency": f"{number(task2_output, 'Efficiency'):.6f}",
            }
        )
        task3_rows.append(
            {
                "n": LIMIT,
                "thread_count": thread_count,
                "run": 1,
                "prime_count": task3_prime_count,
                "serial_seconds": f"{number(task3_output, 'Serial computation time'):.9f}",
                "parallel_seconds": f"{number(task3_output, 'OpenMP computation time'):.9f}",
                "speedup": f"{number(task3_output, 'Speedup'):.6f}",
                "efficiency": f"{number(task3_output, 'Efficiency'):.6f}",
            }
        )
        print(f"Completed {thread_count}/40 threads", flush=True)

    fieldnames = [
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
        ("task2_thread_scaling_n10000000.csv", task2_rows),
        ("task3_thread_scaling_n10000000.csv", task3_rows),
    ]:
        with (DATA_DIRECTORY / filename).open("w", newline="", encoding="utf-8") as file:
            writer = csv.DictWriter(file, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(rows)

    with (DATA_DIRECTORY / "pthread_openmp_thread_scaling_n10000000.csv").open(
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
                    "n": LIMIT,
                    "thread_count": pthread_row["thread_count"],
                    "run": 1,
                    "prime_count": expected_prime_count,
                    "pthreads_seconds": pthread_row["parallel_seconds"],
                    "openmp_seconds": openmp_row["parallel_seconds"],
                    "pthreads_speedup": pthread_row["speedup"],
                    "openmp_speedup": openmp_row["speedup"],
                }
            )


if __name__ == "__main__":
    main()
