# FIT3143 Week 8 - Open MPI Prime Search

## Build and verify

Build inside a Linux environment with Open MPI and OpenMP:

~~~bash
make all
make verify
~~~

The verification script checks exact, sorted output against an independent
reference for n = 0, 1, 2, 3, 4, 10, 1000, 1/2/4 MPI processes, and 1/2/3
OpenMP threads per MPI process.

## Run the programs

~~~bash
mpirun -np 4 ./build/task1 60000000 --strategy cyclic --benchmark
mpirun -np 4 ./build/task2 60000000 2
~~~

Task 1 accepts --strategy cyclic or --strategy block. Cyclic is the default
and recommended strategy. Trial-division cost increases with candidate
magnitude, so round-robin ownership gives every process a mixture of small and
large odd candidates. The block option remains available for a controlled
workload-distribution comparison.

Task 2 takes n and threads_per_process. MPI distributes odd candidate indices
cyclically across processes. Each process uses an OpenMP parallel loop with
dynamic scheduling to smooth residual variation in primality-test cost.

Both programs:

- accept n only at the command line on rank 0;
- broadcast configuration to all MPI processes;
- exclude every even candidate except 2;
- test only odd divisors through sqrt(candidate);
- gather only prime values rather than all candidate flags;
- produce a strictly ascending prime file at rank 0; and
- include communication, root ordering, and file writing in end-to-end time.

Task 1 and Task 2 each merge locally sorted rank lists with a min-heap on rank
0. This takes O(m log p) for m found primes and p processes, instead of a
root-wide O(m log m) sort.

## Reproducible performance experiments

~~~bash
./scripts/run_benchmarks.sh n
./scripts/run_benchmarks.sh task1-process
./scripts/run_benchmarks.sh hybrid
~~~

The default n experiment uses 30 values from 20,000,000 to 78,000,000 in steps
of 2,000,000. The process and hybrid experiments use n = 60,000,000 and three
repetitions per configuration, reporting the median. Environment variables can
change the range or repetitions without editing the script:

~~~bash
REPEATS=5 BENCHMARK_N=80000000 ./scripts/run_benchmarks.sh hybrid
~~~

The runner writes:

| File | Use in the presentation |
| --- | --- |
| results/runtime_n_scaling.csv | Required runtime and empirical-speedup comparisons as n increases |
| results/task1_process_scaling.csv | Task 1 empirical and Amdahl theoretical speedup against MPI process count |
| results/hybrid_scaling.csv | Task 2 empirical speedup, Task 2 versus Task 1, and Amdahl theoretical speedup across process/thread configurations |

The runner deliberately uses mpirun --bind-to none --oversubscribe. OpenMP
teams must not be pinned to a single CPU by MPI rank affinity. The
16-rank and 16-thread configurations also expose the effect of
oversubscription after the machine's 14 physical cores have been exceeded.

## Amdahl analysis

For each implementation, use the one-worker measurement as the baseline:

~~~text
T1 = Ts + Tp
f_s = Ts / T1
f_p = Tp / T1
S_Amdahl(W) = T1 / (Ts + Tp / W)
~~~

Tp is the measured maximum search-and-compaction time. Ts = T1 - Tp includes
collective communication, root merge, and file writing. W is the number of MPI
ranks for Task 1, and MPI ranks × OpenMP threads per rank for Task 2. The
empirical speedup is always serial end-to-end time / parallel end-to-end time,
so it uses the same output workload as the theoretical baseline.

Actual speedup can fall below this ideal model because Amdahl's Law does not
model collective latency, root-only output, process start-up, memory
contention, cache effects, scheduling, or oversubscription.

## Presentation and Q&A checklist

Keep the main presentation to 6-7 minutes:

1. Task 1 design: odd-only kernel, cyclic distribution, MPI collectives, and
   rank-list merge.
2. Task 1 results: runtime and speedup graphs, including comparison with the
   supplied Week 4 OpenMP data.
3. Task 2 design: two-level decomposition and OpenMP dynamic scheduling.
4. Task 2 results: total-worker and fixed-process comparisons against Task 1
   and OpenMP.
5. Task 3: state the measured Ts, Tp, Amdahl formula, and empirical versus
   theoretical comparison.
6. Limitations: root output remains serial, communication increases with rank
   count, and oversubscription reduces efficiency.

For Q&A, distinguish distributed-memory MPI communication from shared-memory
OpenMP worksharing. MPI is useful when work spans nodes or exceeds one
machine's memory, but for this fine-grained prime kernel it adds collective
and root-aggregation overhead. Empirical speedup differs from Amdahl's
prediction because the prediction assumes a fixed serial fraction and ideal
parallel scaling.
