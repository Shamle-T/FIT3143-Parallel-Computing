#!/usr/bin/env bash

set -e

N="${N:-60000000}"
PROCESS_COUNTS="${PROCESS_COUNTS:-1 2 4 8 16}"
RESULT_FILE="results/task3_process_scaling.csv"

mkdir -p results

echo "processes,serial_compute,serial_total,mpi_compute,communication,postprocess,mpi_total,rp,rs,comm_fraction,empirical_speedup,theoretical_speedup" > "$RESULT_FILE"

echo "Running serial baseline for n=$N..."

serial_output=$(./task1_serial --benchmark "$N")

serial_compute=$(echo "$serial_output" | awk '/^Computation time:/ {print $3}')
serial_total=$(echo "$serial_output" | awk '/^Overall wall-clock time:/ {print $4}')

if [ -z "$serial_compute" ] || [ -z "$serial_total" ]; then
    echo "Error: failed to extract serial timing."
    exit 1
fi

echo "Serial computation: $serial_compute s"
echo "Serial overall:     $serial_total s"

for p in $PROCESS_COUNTS
do
    echo "======================================"
    echo "Task 3: n=$N, MPI processes=$p"
    echo "======================================"

    output=$(
        mpirun --use-hwthread-cpus -np "$p" \
        ./task1_task3 "$N" \
        --strategy cyclic \
        --benchmark
    )

    mpi_compute=$(echo "$output" | awk '/^Computation time \(max\):/ {print $4}')
    communication=$(echo "$output" | awk '/^Communication time:/ {print $3}')
    postprocess=$(echo "$output" | awk '/^Root sort\/output time:/ {print $4}')
    mpi_total=$(echo "$output" | awk '/^Overall wall-clock time:/ {print $4}')

    if [ -z "$mpi_compute" ] || \
       [ -z "$communication" ] || \
       [ -z "$postprocess" ] || \
       [ -z "$mpi_total" ]; then
        echo "Error: failed to extract timing for p=$p"
        exit 1
    fi

    values=$(
        awk \
        -v p="$p" \
        -v serial_compute="$serial_compute" \
        -v serial_total="$serial_total" \
        -v communication="$communication" \
        -v mpi_total="$mpi_total" \
        'BEGIN {
            # Keep the theoretical and empirical speedups on the
            # same serial-total baseline. Communication is an added
            # distributed-memory overhead, not part of the serial baseline.
            rp = serial_compute / serial_total
            comm_fraction = communication / serial_total
            rs = 1.0 - rp
            empirical = serial_total / mpi_total
            theoretical = 1.0 / (rs + (rp / p) + comm_fraction)

            printf "%.9f %.9f %.9f %.9f %.9f",
                   rp,
                   rs,
                   comm_fraction,
                   empirical,
                   theoretical
        }'
    )

    read -r rp rs comm_fraction empirical theoretical <<< "$values"

    echo "MPI computation:     $mpi_compute s"
    echo "Communication:       $communication s"
    echo "Root postprocess:    $postprocess s"
    echo "MPI overall:         $mpi_total s"
    echo "Parallel fraction:   $rp"
    echo "Serial fraction:     $rs"
    echo "Communication frac:  $comm_fraction"
    echo "Empirical speedup:   $empirical"
    echo "Theoretical speedup: $theoretical"

    echo "$p,$serial_compute,$serial_total,$mpi_compute,$communication,$postprocess,$mpi_total,$rp,$rs,$comm_fraction,$empirical,$theoretical" >> "$RESULT_FILE"
done

echo "======================================"
echo "Finished."
echo "Results saved to $RESULT_FILE"
echo "======================================"
