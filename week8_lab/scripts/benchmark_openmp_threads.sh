#!/bin/bash

set -e

RESULT_FILE="results/openmp_thread_scaling.csv"
N=60000000

echo "n,threads,openmp_time" > "$RESULT_FILE"

for threads in 1 2 4 8 16
do
    echo "======================================"
    echo "Testing OpenMP: n=$N, threads=$threads"
    echo "======================================"

    output=$(./task3_benchmark --benchmark "$N" "$threads")

    openmp_time=$(echo "$output" | awk '/Overall wall-clock time:/ {print $4}')

    if [ -z "$openmp_time" ]; then
        echo "Error: failed to extract OpenMP time for threads=$threads"
        exit 1
    fi

    echo "OpenMP time: $openmp_time s"
    echo "$N,$threads,$openmp_time" >> "$RESULT_FILE"
done

echo "Finished."
echo "Results saved to $RESULT_FILE"
