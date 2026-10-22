#!/usr/bin/env bash

# Reproducible end-to-end performance measurements for FIT3143 Lab 2.
#
# Outputs are median wall-clock timings.  Every run includes result gathering,
# root ordering, and file writing, matching the assessment rubric.  The default
# n range contains exactly 30 problem sizes and starts at a scale where timing
# noise is unlikely to dominate on the lab machine.
#
# Examples:
#   ./scripts/run_benchmarks.sh n
#   ./scripts/run_benchmarks.sh task1-process
#   ./scripts/run_benchmarks.sh hybrid
#   ./scripts/run_benchmarks.sh balance
#   ./scripts/run_benchmarks.sh all
set -euo pipefail

MODE=${1:-all}
RESULT_DIR=${RESULT_DIR:-results}
REPEATS=${REPEATS:-3}
N_START=${N_START:-20000000}
N_STEP=${N_STEP:-2000000}
N_END=${N_END:-78000000}
BENCHMARK_N=${BENCHMARK_N:-60000000}
MPI_PROCESSES=${MPI_PROCESSES:-4}
HYBRID_THREADS=${HYBRID_THREADS:-2}
PROCESS_COUNTS=(${PROCESS_COUNTS:-1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16})
HYBRID_CONFIGS=(${HYBRID_CONFIGS:-1:1 1:2 1:4 1:8 1:16 2:1 2:2 4:1 4:2 4:4 4:8 4:16 8:1 8:2 16:1})
BALANCE_N=${BALANCE_N:-20000000}
BALANCE_PROCESSES=(${BALANCE_PROCESSES:-3 4 5})

mkdir -p "$RESULT_DIR"

#
# Disable rank binding so OpenMP worker threads may use the machine's cores.
# Oversubscription lets the final configurations test behaviour past the
# physical-core count, as required by the assessment specification.
MPI_RUN=(mpirun --bind-to none --oversubscribe)
if [[ $(id -u) -eq 0 ]]; then
    MPI_RUN+=(--allow-run-as-root)
fi

median() {
    printf '%s\n' "$@" | LC_ALL=C sort -n | awk '{ values[NR] = $1 } END { if (NR % 2) print values[(NR + 1) / 2]; else printf "%.9f\n", (values[NR / 2] + values[NR / 2 + 1]) / 2 }'
}

field() {
    local label=$1
    awk -F': ' -v label="$label" '$1 == label { sub(/ seconds$/, "", $2); print $2; exit }'
}

timed() {
    local output label=$1
    shift
    output=$("$@")
    field "$label" <<<"$output"
}

median_timed() {
    local label=$1
    shift
    local samples=()
    local sample

    for ((run = 1; run <= REPEATS; ++run)); do
        sample=$(timed "$label" "$@")
        [[ -n "$sample" ]] || {
            printf 'Could not extract "%s" from benchmark output.\n' "$label" >&2
            return 1
        }
        samples+=("$sample")
    done
    median "${samples[@]}"
}

run_n_scaling() {
    local output="$RESULT_DIR/runtime_n_scaling.csv"
    printf 'n,mpi_processes,hybrid_threads,serial_seconds,openmp_seconds,mpi_seconds,hybrid_seconds,mpi_speedup,hybrid_speedup\n' >"$output"

    for ((n = N_START; n <= N_END; n += N_STEP)); do
        local serial openmp mpi hybrid mpi_speedup hybrid_speedup
        serial=$(median_timed 'Overall wall-clock time' ./build/task1_serial --benchmark "$n")
        openmp=$(median_timed 'Overall wall-clock time' ./build/task3_benchmark --benchmark "$n" "$MPI_PROCESSES")
        mpi=$(median_timed 'Overall wall-clock time' "${MPI_RUN[@]}" -np "$MPI_PROCESSES" ./build/task1 "$n" --strategy block-cyclic --benchmark)
        hybrid=$(median_timed 'Overall wall-clock time' "${MPI_RUN[@]}" -np "$MPI_PROCESSES" ./build/task2 "$n" "$HYBRID_THREADS")
        mpi_speedup=$(awk -v serial="$serial" -v parallel="$mpi" 'BEGIN { printf "%.9f", serial / parallel }')
        hybrid_speedup=$(awk -v serial="$serial" -v parallel="$hybrid" 'BEGIN { printf "%.9f", serial / parallel }')
        printf '%s,%s,%s,%s,%s,%s,%s,%s,%s\n' "$n" "$MPI_PROCESSES" "$HYBRID_THREADS" "$serial" "$openmp" "$mpi" "$hybrid" "$mpi_speedup" "$hybrid_speedup" >>"$output"
    done
}

run_task1_process_scaling() {
    local output="$RESULT_DIR/task1_process_scaling.csv"
    local serial
    serial=$(median_timed 'Overall wall-clock time' ./build/task1_serial --benchmark "$BENCHMARK_N")
    printf 'n,processes,serial_seconds,compute_seconds,gather_seconds,postprocess_seconds,mpi_seconds,empirical_speedup,theoretical_speedup,efficiency\n' >"$output"

    local baseline_serial= baseline_parallel=
    for processes in "${PROCESS_COUNTS[@]}"; do
        local compute gather postprocess overall empirical theoretical efficiency
        local compute_samples=() gather_samples=() postprocess_samples=() overall_samples=()

        for ((run = 1; run <= REPEATS; ++run)); do
            local measurement
            measurement=$("${MPI_RUN[@]}" -np "$processes" ./build/task1 "$BENCHMARK_N" --strategy block-cyclic --benchmark)
            compute_samples+=("$(field 'Computation time (max)' <<<"$measurement")")
            gather_samples+=("$(field 'Maximum result-gather time' <<<"$measurement")")
            postprocess_samples+=("$(field 'Root merge + file-write time' <<<"$measurement")")
            overall_samples+=("$(field 'Overall wall-clock time' <<<"$measurement")")
        done
        compute=$(median "${compute_samples[@]}")
        gather=$(median "${gather_samples[@]}")
        postprocess=$(median "${postprocess_samples[@]}")
        overall=$(median "${overall_samples[@]}")

        if [[ $processes -eq 1 ]]; then
            baseline_serial=$(awk -v total="$overall" -v compute="$compute" 'BEGIN { printf "%.9f", total - compute }')
            baseline_parallel=$compute
        fi

        empirical=$(awk -v serial="$serial" -v parallel="$overall" 'BEGIN { printf "%.9f", serial / parallel }')
        theoretical=$(awk -v serial="$baseline_serial" -v parallel="$baseline_parallel" -v workers="$processes" 'BEGIN { printf "%.9f", (serial + parallel) / (serial + parallel / workers) }')
        efficiency=$(awk -v speedup="$empirical" -v workers="$processes" 'BEGIN { printf "%.9f", speedup / workers }')
        printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' "$BENCHMARK_N" "$processes" "$serial" "$compute" "$gather" "$postprocess" "$overall" "$empirical" "$theoretical" "$efficiency" >>"$output"
    done
}

run_hybrid_scaling() {
    local output="$RESULT_DIR/hybrid_scaling.csv"
    local serial
    declare -A mpi_cache
    serial=$(median_timed 'Overall wall-clock time' ./build/task1_serial --benchmark "$BENCHMARK_N")
    printf 'n,processes,threads_per_process,total_workers,serial_seconds,mpi_seconds,compute_seconds,gather_seconds,postprocess_seconds,hybrid_seconds,empirical_speedup,vs_mpi_speedup,theoretical_speedup,efficiency\n' >"$output"

    local baseline_serial= baseline_parallel=
    for configuration in "${HYBRID_CONFIGS[@]}"; do
        local processes=${configuration%%:*}
        local threads=${configuration##*:}
        local workers=$((processes * threads))
        local mpi compute gather postprocess overall empirical versus_mpi theoretical efficiency
        local compute_samples=() gather_samples=() postprocess_samples=() overall_samples=()

        if [[ ! -v "mpi_cache[$processes]" ]]; then
            local mpi_samples=()
            for ((run = 1; run <= REPEATS; ++run)); do
                local mpi_measurement
                mpi_measurement=$("${MPI_RUN[@]}" -np "$processes" ./build/task1 "$BENCHMARK_N" --strategy block-cyclic --benchmark)
                mpi_samples+=("$(field 'Overall wall-clock time' <<<"$mpi_measurement")")
            done
            mpi_cache[$processes]=$(median "${mpi_samples[@]}")
        fi
        mpi=${mpi_cache[$processes]}

        for ((run = 1; run <= REPEATS; ++run)); do
            local hybrid_measurement
            hybrid_measurement=$("${MPI_RUN[@]}" -np "$processes" ./build/task2 "$BENCHMARK_N" "$threads")
            compute_samples+=("$(field 'Maximum computation time' <<<"$hybrid_measurement")")
            gather_samples+=("$(field 'Maximum gather time' <<<"$hybrid_measurement")")
            postprocess_samples+=("$(field 'Root merge + file-write time' <<<"$hybrid_measurement")")
            overall_samples+=("$(field 'Overall wall-clock time' <<<"$hybrid_measurement")")
        done
        compute=$(median "${compute_samples[@]}")
        gather=$(median "${gather_samples[@]}")
        postprocess=$(median "${postprocess_samples[@]}")
        overall=$(median "${overall_samples[@]}")

        if [[ $processes -eq 1 && $threads -eq 1 ]]; then
            baseline_parallel=$compute
            baseline_serial=$(awk -v total="$overall" -v compute="$compute" 'BEGIN { printf "%.9f", total - compute }')
        fi

        empirical=$(awk -v serial="$serial" -v parallel="$overall" 'BEGIN { printf "%.9f", serial / parallel }')
        versus_mpi=$(awk -v mpi="$mpi" -v hybrid="$overall" 'BEGIN { printf "%.9f", mpi / hybrid }')
        theoretical=$(awk -v serial="$baseline_serial" -v parallel="$baseline_parallel" -v workers="$workers" 'BEGIN { printf "%.9f", (serial + parallel) / (serial + parallel / workers) }')
        efficiency=$(awk -v speedup="$empirical" -v workers="$workers" 'BEGIN { printf "%.9f", speedup / workers }')
        printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' "$BENCHMARK_N" "$processes" "$threads" "$workers" "$serial" "$mpi" "$compute" "$gather" "$postprocess" "$overall" "$empirical" "$versus_mpi" "$theoretical" "$efficiency" >>"$output"
    done
}

run_partition_balance() {
    local output="$RESULT_DIR/partition_balance.csv"
    printf 'n,processes,strategy,min_compute_seconds,max_compute_seconds,imbalance_ratio,overall_seconds\n' >"$output"

    for processes in "${BALANCE_PROCESSES[@]}"; do
        for strategy in block-cyclic cyclic block; do
            local minimum maximum overall imbalance
            local minimum_samples=() maximum_samples=() overall_samples=()

            for ((run = 1; run <= REPEATS; ++run)); do
                local measurement
                measurement=$("${MPI_RUN[@]}" -np "$processes" ./build/task1 "$BALANCE_N" --strategy "$strategy" --benchmark)
                minimum_samples+=("$(field 'Computation time (min)' <<<"$measurement")")
                maximum_samples+=("$(field 'Computation time (max)' <<<"$measurement")")
                overall_samples+=("$(field 'Overall wall-clock time' <<<"$measurement")")
            done

            minimum=$(median "${minimum_samples[@]}")
            maximum=$(median "${maximum_samples[@]}")
            overall=$(median "${overall_samples[@]}")
            imbalance=$(awk -v maximum="$maximum" -v minimum="$minimum" 'BEGIN { if (minimum > 0) printf "%.9f", maximum / minimum; else print "nan" }')
            printf '%s,%s,%s,%s,%s,%s,%s\n' "$BALANCE_N" "$processes" "$strategy" "$minimum" "$maximum" "$imbalance" "$overall" >>"$output"
        done
    done
}

case "$MODE" in
    n) run_n_scaling ;;
    task1-process) run_task1_process_scaling ;;
    hybrid) run_hybrid_scaling ;;
    balance) run_partition_balance ;;
    all)
        run_n_scaling
        run_task1_process_scaling
        run_hybrid_scaling
        run_partition_balance
        ;;
    *)
        printf 'Usage: %s {n|task1-process|hybrid|balance|all}\n' "$0" >&2
        exit 2
        ;;
esac

printf 'Benchmark data written to %s.\n' "$RESULT_DIR"
