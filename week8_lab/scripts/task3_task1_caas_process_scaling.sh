#!/usr/bin/env bash

# Task 3, Task 1 only: collect Open MPI timing data on an allocated CAAS job.
# The CAAS scheduler must allocate nodes/slots before this script is run.

set -euo pipefail

N="${N:-60000000}"
PROCESS_COUNTS="${PROCESS_COUNTS:-1 2 4 8}"
RESULT_FILE="${RESULT_FILE:-results/caas_task1_process_scaling.csv}"
MPI_LAUNCHER="${MPI_LAUNCHER:-mpirun}"
MPI_LAUNCH_ARGS="${MPI_LAUNCH_ARGS:-}"

read -r -a launcher_args <<< "$MPI_LAUNCH_ARGS"

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Error: required command not found: $1" >&2
        exit 1
    fi
}

extract_value() {
    local label="$1"
    awk -v label="$label" '$0 ~ ("^" label) {print $NF}'
}

require_command "$MPI_LAUNCHER"
require_command awk
require_command sort
require_command paste

if [[ ! -x ./task1_task3 || ! -x ./task1_serial ]]; then
    echo "Error: build ./task1_task3 and ./task1_serial before running this script." >&2
    exit 1
fi

mkdir -p "$(dirname "$RESULT_FILE")"

serial_output=$(./task1_serial --benchmark "$N")
serial_compute=$(printf '%s\n' "$serial_output" | extract_value "Computation time:")
serial_total=$(printf '%s\n' "$serial_output" | extract_value "Overall wall-clock time:")

if [[ -z "$serial_compute" || -z "$serial_total" ]]; then
    echo "Error: failed to extract serial timings." >&2
    exit 1
fi

echo "n,processes,nodes,hosts,serial_compute,serial_total,mpi_compute,communication,postprocess,mpi_total" > "$RESULT_FILE"

for processes in $PROCESS_COUNTS; do
    if ! [[ "$processes" =~ ^[1-9][0-9]*$ ]]; then
        echo "Error: invalid process count: $processes" >&2
        exit 1
    fi

    host_list=$("$MPI_LAUNCHER" "${launcher_args[@]}" -np "$processes" hostname | sort -u | paste -sd ';' -)
    node_count=$(printf '%s\n' "$host_list" | awk -F ';' '{print NF}')

    output=$("$MPI_LAUNCHER" "${launcher_args[@]}" -np "$processes" \
        ./task1_task3 "$N" --strategy cyclic --benchmark)

    mpi_compute=$(printf '%s\n' "$output" | extract_value "Computation time \(max\):")
    communication=$(printf '%s\n' "$output" | extract_value "Communication time:")
    postprocess=$(printf '%s\n' "$output" | extract_value "Root sort/output time:")
    mpi_total=$(printf '%s\n' "$output" | extract_value "Overall wall-clock time:")

    if [[ -z "$mpi_compute" || -z "$communication" || -z "$postprocess" || -z "$mpi_total" ]]; then
        echo "Error: failed to extract MPI timings for P=$processes." >&2
        exit 1
    fi

    echo "$N,$processes,$node_count,$host_list,$serial_compute,$serial_total,$mpi_compute,$communication,$postprocess,$mpi_total" >> "$RESULT_FILE"
    echo "Recorded P=$processes across $node_count node(s): $host_list"
done

echo "Saved CAAS Task 1 timing data to $RESULT_FILE"
