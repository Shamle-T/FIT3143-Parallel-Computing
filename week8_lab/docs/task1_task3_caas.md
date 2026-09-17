# Task 1 Task 3: Local and CAAS Runbook

This runbook covers the MPI-only Task 1 performance analysis. It does not run
or modify the hybrid Task 2 implementation.

## Local preparation

From `week8_lab`, build the serial baseline and the instrumented MPI version:

```bash
gcc -std=c11 -O3 -Wall -Wextra -pedantic task1_serial.c -o task1_serial -lm
mpicc -std=c11 -O3 -Wall -Wextra -pedantic task1_task3.c -o task1_task3 -lm
```

Confirm correctness before benchmarking:

```bash
mpirun -np 3 ./task1_task3 30 --strategy cyclic --benchmark
cat task1_mpi_primes.txt
```

The expected primes are `2 3 5 7 11 13 17 19 23 29`.

## Amdahl-style model

For every benchmark row, use the serial total time as the shared baseline:

```text
Rp = T_serial_compute / T_serial_total
Rs = 1 - Rp
C(P) = T_communication(P) / T_serial_total
S_theory(P) = 1 / (Rs + Rp/P + C(P))
S_empirical(P) = T_serial_total / T_MPI_total(P)
```

`T_communication` is the instrumented broadcast plus result-collection time.
The separate root sort/output time is retained as evidence of a root bottleneck.

Regenerate aligned local Task 1 metrics and graphs with:

```bash
python3 scripts/generate_task1_graphs.py
```

## CAAS procedure

1. Follow the CAAS documentation to obtain an interactive or batch allocation
   with the intended nodes and slots. Do not use `--oversubscribe`.
2. In the allocation, record `hostname`, `nproc`, `mpicc --version`, and
   `mpirun --version` for the presentation.
3. Transfer or clone the `week8_lab` directory, then build `task1_serial` and
   `task1_task3` using the commands above.
4. Verify placement before timing. For example, after allocation:

   ```bash
   mpirun -np 4 hostname | sort -u
   ```

5. Run the Task 1-only collector. The default process counts are 1, 2, 4, and
   8; set only counts that fit the slots allocated by CAAS:

   ```bash
   chmod +x scripts/task3_task1_caas_process_scaling.sh
   PROCESS_COUNTS="1 2 4 8" \
       ./scripts/task3_task1_caas_process_scaling.sh
   ```

6. If CAAS requires launcher options supplied by its documentation, pass them
   without changing the script. For example:

   ```bash
   MPI_LAUNCH_ARGS="<CAAS-required-launcher-options>" \
       PROCESS_COUNTS="1 2 4 8" \
       ./scripts/task3_task1_caas_process_scaling.sh
   ```

The script writes `results/caas_task1_process_scaling.csv` separately from
local evidence and records the host names and node count for each row.
