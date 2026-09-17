#!/bin/bash

set -e

RESULT_FILE="results/n_scaling.csv"
PROCESSES=4

echo "n,processes,serial_time,mpi_time,speedup,efficiency" > "$RESULT_FILE"

for n in $(seq 20000000 2000000 78000000)
do
    echo
    echo "========================================="
    echo "Testing n = $n"
    echo "========================================="

    # ---------------------------
    # Serial
    # ---------------------------

    serial_output=$(
        ./task1_serial --benchmark "$n"
    )

    serial_time=$(
        echo "$serial_output" |
        awk '/Overall wall-clock time:/ {print $4}'
    )

    echo "Serial: $serial_time s"

    # ---------------------------
    # MPI - cyclic
    # ---------------------------

    mpi_output=$(
        mpirun -np "$PROCESSES" \
        ./task1 "$n" \
        --strategy cyclic \
        --benchmark
    )

    mpi_time=$(
        echo "$mpi_output" |
        awk '/Overall wall-clock time:/ {print $4}'
    )

    echo "MPI:    $mpi_time s"

    # ---------------------------
    # Speedup
    # ---------------------------

    speedup=$(
        awk -v serial="$serial_time" \
            -v mpi="$mpi_time" \
            'BEGIN { printf "%.6f", serial / mpi }'
    )

    # ---------------------------
    # Efficiency
    # ---------------------------

    efficiency=$(
        awk -v speedup="$speedup" \
            -v p="$PROCESSES" \
            'BEGIN { printf "%.6f", speedup / p }'
    )

    echo "Speedup:    $speedup"
    echo "Efficiency: $efficiency"

    # ---------------------------
    # Save CSV
    # ---------------------------

    echo \
    "$n,$PROCESSES,$serial_time,$mpi_time,$speedup,$efficiency" \
    >> "$RESULT_FILE"
done

echo
echo "========================================="
echo "Finished."
echo "Results saved to:"
echo "$RESULT_FILE"
echo "========================================="
