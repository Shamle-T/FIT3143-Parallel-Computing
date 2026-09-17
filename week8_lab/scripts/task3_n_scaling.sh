#!/usr/bin/env bash

set -e

PROCESSES="${PROCESSES:-4}"
N_START="${N_START:-20000000}"
N_END="${N_END:-78000000}"
N_STEP="${N_STEP:-2000000}"
RESULT_FILE="results/task3_n_scaling.csv"

mkdir -p results

echo "n,processes,serial_compute,serial_total,mpi_compute,communication,postprocess,mpi_total,rp,rs,comm_fraction,empirical_speedup,theoretical_speedup" > "$RESULT_FILE"

for n in $(seq "$N_START" "$N_STEP" "$N_END")
do
    echo "======================================"
    echo "Task 3: n=$n, MPI processes=$PROCESSES"
    echo "======================================"

    serial_output=$(./task1_serial --benchmark "$n")

    serial_compute=$(echo "$serial_output" | awk '/^Computation time:/ {print $3}')
    serial_total=$(echo "$serial_output" | awk '/^Overall wall-clock time:/ {print $4}')

    if [ -z "$serial_compute" ] || [ -z "$serial_total" ]; then
        echo "Error: failed to extract serial timing for n=$n"
        exit 1
    fi

    mpi_output=$(
        mpirun --use-hwthread-cpus -np "$PROCESSES" \
        ./task1_task3 "$n" \
        --strategy cyclic \
        --benchmark
    )

    mpi_compute=$(echo "$mpi_output" | awk '/^Computation time \(max\):/ {print $4}')
    communication=$(echo "$mpi_output" | awk '/^Communication time:/ {print $3}')
    postprocess=$(echo "$mpi_output" | awk '/^Root sort\/output time:/ {print $4}')
    mpi_total=$(echo "$mpi_output" | awk '/^Overall wall-clock time:/ {print $4}')

    if [ -z "$mpi_compute" ] || \
       [ -z "$communication" ] || \
       [ -z "$postprocess" ] || \
       [ -z "$mpi_total" ]; then
        echo "Error: failed to extract MPI timing for n=$n"
        exit 1
    fi

    values=$(
        awk \
        -v p="$PROCESSES" \
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

    echo "Serial overall:      $serial_total s"
    echo "MPI overall:         $mpi_total s"
    echo "Communication:       $communication s"
    echo "Parallel fraction:   $rp"
    echo "Serial fraction:     $rs"
    echo "Empirical speedup:   $empirical"
    echo "Theoretical speedup: $theoretical"

    echo "$n,$PROCESSES,$serial_compute,$serial_total,$mpi_compute,$communication,$postprocess,$mpi_total,$rp,$rs,$comm_fraction,$empirical,$theoretical" >> "$RESULT_FILE"
done

echo "======================================"
echo "Finished."
echo "Results saved to $RESULT_FILE"
echo "======================================"
