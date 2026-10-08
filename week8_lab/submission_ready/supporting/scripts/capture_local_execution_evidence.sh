#!/usr/bin/env bash

# Capture a compact, reproducible local Open MPI execution transcript for the
# Task 4 appendix. It records runtime configuration, balance evidence, and
# independent sorted-prime validation for both implementations.
set -euo pipefail

RESULT_FILE=${RESULT_FILE:-results/local_cluster_execution.txt}
LIMIT=${LIMIT:-60000000}
MPI_PROCESSES=${MPI_PROCESSES:-4}
THREADS_PER_PROCESS=${THREADS_PER_PROCESS:-2}

mkdir -p "$(dirname "$RESULT_FILE")"

MPI_RUN=(mpirun --bind-to none --oversubscribe)
if [[ $(id -u) -eq 0 ]]; then
    MPI_RUN+=(--allow-run-as-root)
fi

validate_output() {
    local path=$1

    python3 - "$LIMIT" "$path" <<'PY'
import sys

limit = int(sys.argv[1])
path = sys.argv[2]

# A sieve makes a full 60-million-value validation practical.  Compare the
# expected stream to the file stream, so no large Python list of primes is
# needed in addition to the sieve.
sieve = bytearray(b"\x01") * limit
if limit > 0:
    sieve[0] = 0
if limit > 1:
    sieve[1] = 0
for divisor in range(2, int(limit ** 0.5) + 1):
    if sieve[divisor]:
        start = divisor * divisor
        sieve[start:limit:divisor] = b"\x00" * (((limit - start - 1) // divisor) + 1)

expected = (value for value in range(2, limit) if sieve[value])
count = 0
with open(path, encoding="utf-8") as handle:
    for line in handle:
        if not line.strip():
            continue
        actual = int(line)
        try:
            required = next(expected)
        except StopIteration:
            raise SystemExit(f"FAILED: {path} contains an extra value {actual}.")
        if actual != required:
            raise SystemExit(
                f"FAILED: {path} differs at position {count}: "
                f"expected {required}, found {actual}."
            )
        count += 1
try:
    missing = next(expected)
    raise SystemExit(f"FAILED: {path} is missing expected prime {missing}.")
except StopIteration:
    pass
print(f"PASS: {path} contains {count} sorted primes strictly below {limit}.")
PY
}

{
    printf 'FIT3143 Week 8 local Open MPI execution evidence\n'
    printf 'UTC timestamp: '
    date -u '+%Y-%m-%dT%H:%M:%SZ'
    printf '\nEnvironment\n'
    uname -a
    printf '\nCPU summary\n'
    lscpu | grep -E 'Model name|Socket|Core\\(s\\) per socket|CPU\\(s\\)'
    printf '\nMPI runtime\n'
    mpirun --version | head -n 2
    printf '\nConfiguration: nodes=1, MPI processes=%s, OpenMP threads/process=%s, total workers=%s, n=%s\n\n' \
        "$MPI_PROCESSES" "$THREADS_PER_PROCESS" \
        "$((MPI_PROCESSES * THREADS_PER_PROCESS))" "$LIMIT"
    printf 'Task 1 command\n'
    printf '%q ' "${MPI_RUN[@]}" -np "$MPI_PROCESSES" ./build/task1 "$LIMIT" --strategy block-cyclic --benchmark
    printf '\nTask 1 output\n'
    "${MPI_RUN[@]}" -np "$MPI_PROCESSES" ./build/task1 "$LIMIT" --strategy block-cyclic --benchmark
    validate_output task1_mpi_primes.txt
    printf '\nTask 2 command\n'
    printf '%q ' "${MPI_RUN[@]}" -np "$MPI_PROCESSES" ./build/task2 "$LIMIT" "$THREADS_PER_PROCESS"
    printf '\nTask 2 output\n'
    "${MPI_RUN[@]}" -np "$MPI_PROCESSES" ./build/task2 "$LIMIT" "$THREADS_PER_PROCESS"
    validate_output task2_primes.txt
} | tee "$RESULT_FILE"
