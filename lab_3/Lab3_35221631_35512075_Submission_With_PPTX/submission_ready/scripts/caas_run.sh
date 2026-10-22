#!/usr/bin/env bash
# Manual runner inside an existing SLURM allocation; full study: caas_audit.job.
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p results/caas
: "${SLURM_JOB_ID:?Run within a SLURM allocation}"
module load openmpi/4.1.5-gcc-11.2.0-ux65npg
mpicc -std=c11 -O2 -g -fopenmp -Wall -Wextra -Wpedantic -Werror src/wsn.c -o wsn_caas -lm
mpirun --version > results/caas/environment.txt
hostname >> results/caas/environment.txt
# Set ROWS/COLS/PORTS only to values that fit your actual allocation.
: "${ROWS:?Set ROWS}" "${COLS:?Set COLS}" "${PORTS:?Set PORTS}"
[[ "$ROWS" =~ ^[1-9][0-9]*$ && "$COLS" =~ ^[1-9][0-9]*$ && "$PORTS" =~ ^[1-9][0-9]*$ ]] || exit 2
(( ROWS*COLS+1 <= SLURM_NTASKS && PORTS <= SLURM_CPUS_PER_TASK )) || {
  echo 'The requested MPI ranks or port threads exceed the allocation.' >&2; exit 2;
}
export OMP_NUM_THREADS="$PORTS" OMP_WAIT_POLICY=PASSIVE OMP_PLACES=cores OMP_PROC_BIND=close
ranks=$((ROWS*COLS+1))
nodes=$(( ranks < SLURM_NNODES ? ranks : SLURM_NNODES ))
srun --mpi=none --exact -N "$nodes" -n "$nodes" --ntasks-per-node=1 -c "$PORTS" hostname >> results/caas/environment.txt
for version in 0 1 2; do
  srun --mpi=none --exact -N "$nodes" -n "$ranks" --ntasks-per-node "$(((ranks+nodes-1)/nodes))" -c "$PORTS" --cpu-bind=cores ./wsn_caas "$ROWS" "$COLS" "$PORTS" \
    --variant "$version" --rounds 20 --warmup 3 --tick 0 --scenario mixed \
    --output "results/caas/${ROWS}x${COLS}_v${version}"
done
