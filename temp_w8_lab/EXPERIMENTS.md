# Experimental Configuration and Evidence

## Execution environment

The reproducible experiments run in a single-node local Open MPI environment:

| Item | Configuration |
| --- | --- |
| Node count | 1 local WSL2 Ubuntu node |
| Processor | 13th Gen Intel Core i7-13700H |
| Physical cores | 14 |
| Logical CPUs visible to Linux | 20 |
| MPI runtime | Open MPI 4.1.6 |
| MPI process binding | Disabled with mpirun --bind-to none |
| Oversubscription experiment | Enabled with mpirun --oversubscribe |
| Prime output | Rank 0 writes one sorted text file per implementation |

The single-node design isolates MPI communication, root aggregation, and
OpenMP thread behaviour on the same host. The 16-rank and 16-thread points
deliberately exceed physical-core count to demonstrate oversubscription.

## Measurements

| Experiment | n | MPI processes | OpenMP threads per process | Total workers | Repetitions | Result file |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Problem-size comparison | 20M to 78M, 30 values | 4 | 2 | 8 | 3 median | results/runtime_n_scaling.csv |
| Task 1 process scaling | 60M | 1, 2, 4, 8, 16 | N/A | 1, 2, 4, 8, 16 | 3 median | results/task1_process_scaling.csv |
| Task 2 hybrid scaling | 60M | 1, 2, 4, 8, 16 | 1, 2, 4, 8, 16 by configuration | p × t | 3 median | results/hybrid_scaling.csv |
| Balance and sorted-output evidence | 60M | 4 | 2 | 8 | 1 representative run | results/local_cluster_execution.txt |

Every reported wall-clock time includes process synchronization, MPI result
communication, rank-0 merge/order work, and file writing. The benchmark
scripts retain maximum, minimum, and per-rank computation times separately.

## Balance evidence

Task 1 assigns odd candidate indices round-robin across MPI ranks. Candidate
count differs by at most one when n does not divide evenly by process count.
Because trial division is costlier near sqrt(n), each rank receives a
representative range of candidate magnitudes.

Task 2 keeps the cyclic MPI partition and adds OpenMP dynamic scheduling inside
each process. Dynamic worksharing assigns chunks as threads finish, smoothing
variation from composite early exits and expensive prime candidates.

The execution transcript records candidate count and computation seconds for
each rank. Use the max/min computation-time ratio in the appendix as direct
evidence of balance. A ratio near 1 indicates balanced process workloads.

## Correctness evidence

The verification script and representative execution transcript independently
check that each output list is sorted, unique, contains only prime numbers, and
contains no value greater than or equal to n.
