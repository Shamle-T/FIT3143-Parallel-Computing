#!/usr/bin/env bash

# Deterministic functional tests for serial, MPI, and hybrid MPI + OpenMP code.
# Run from the repository root: make verify
set -euo pipefail

MPI_RUN=(mpirun)
if [[ $(id -u) -eq 0 ]]; then
    MPI_RUN+=(--allow-run-as-root)
fi

verify_file() {
    local limit=$1
    local path=$2

    python3 - "$limit" "$path" <<'PY'
import sys

limit = int(sys.argv[1])
path = sys.argv[2]
with open(path, encoding="utf-8") as handle:
    actual = [int(line) for line in handle if line.strip()]
expected = [candidate for candidate in range(2, limit)
            if all(candidate % divisor for divisor in range(2, int(candidate ** 0.5) + 1))]
if actual != expected:
    raise SystemExit(
        f"{path}: incorrect or unsorted prime list for n={limit}; "
        f"got {len(actual)} values, expected {len(expected)}"
    )
PY
}

for limit in 0 1 2 3 4 10 1000; do
    for processes in 1 2 3 4 5; do
        for strategy in block-cyclic cyclic block; do
            "${MPI_RUN[@]}" -np "$processes" ./build/task1 "$limit" \
                --strategy "$strategy" --benchmark >/dev/null
            verify_file "$limit" task1_mpi_primes.txt
        done
    done
done

for limit in 0 1 2 3 4 10 1000; do
    for processes in 1 2 3 4 5; do
        for threads in 1 2 3; do
            "${MPI_RUN[@]}" -np "$processes" ./build/task2 "$limit" "$threads" >/dev/null
            verify_file "$limit" task2_primes.txt
        done
    done
done

printf 'All serial-independent MPI and hybrid correctness checks passed.\n'
