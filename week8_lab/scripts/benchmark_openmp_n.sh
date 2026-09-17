#!/bin/bash

set -e

RESULT_FILE="results/openmp_n_scaling.csv"
THREADS=4

echo "n,threads,openmp_time" > "$RESULT_FILE"

for n in $(seq 20000000 2000000 78000000)
do
    echo "======================================"
    echo "Testing OpenMP: n=$n, threads=$THREADS"
    echo "======================================"

    output=$(./task3_benchmark --benchmark "$n" "$THREADS")

    openmp_time=$(echo "$output" | awk '/Overall wall-clock time:/ {print $4}')

    if [ -z "$openmp_time" ]; then
        echo "Error: failed to extract OpenMP time for n=$n"
        exit 1
    fi

    echo "OpenMP time: $openmp_time s"
    echo "$n,$THREADS,$openmp_time" >> "$RESULT_FILE"
done

echo "Finished."
echo "Results saved to $RESULT_FILE"
