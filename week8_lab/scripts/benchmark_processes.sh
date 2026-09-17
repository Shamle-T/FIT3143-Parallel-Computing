#!/bin/bash

set -e

RESULT_FILE="results/process_scaling.csv"
N=60000000

echo "n,processes,serial_time,mpi_time,speedup,efficiency" > "$RESULT_FILE"

echo "Running serial baseline..."

serial_output=$(./task1_serial --benchmark "$N")
serial_time=$(echo "$serial_output" | awk '/Overall wall-clock time:/ {print $4}')

if [ -z "$serial_time" ]; then
    echo "Error: failed to extract serial time."
    exit 1
fi

echo "Serial time: $serial_time s"

for p in 1 2 4 8 16
do
    echo "======================================"
    echo "Testing MPI: n=$N, processes=$p"
    echo "======================================"

    mpi_output=$(
        mpirun --use-hwthread-cpus -np "$p" \
        ./task1 "$N" --strategy cyclic --benchmark
    )

    mpi_time=$(echo "$mpi_output" | awk '/Overall wall-clock time:/ {print $4}')

    if [ -z "$mpi_time" ]; then
        echo "Error: failed to extract MPI time for p=$p"
        exit 1
    fi

    speedup=$(
        awk -v serial="$serial_time" -v mpi="$mpi_time" \
        'BEGIN { printf "%.6f", serial / mpi }'
    )

    efficiency=$(
        awk -v speedup="$speedup" -v p="$p" \
        'BEGIN { printf "%.6f", speedup / p }'
    )

    echo "MPI time:   $mpi_time s"
    echo "Speedup:    $speedup"
    echo "Efficiency: $efficiency"

    echo "$N,$p,$serial_time,$mpi_time,$speedup,$efficiency" >> "$RESULT_FILE"
done

echo "Finished."
echo "Results saved to $RESULT_FILE"
