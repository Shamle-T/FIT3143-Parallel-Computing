# FIT3143 Week 8 Lab 2 - 6 to 7 Minute Presentation Script

## Slide 1 - Title (15 seconds)

We implemented prime search below n using Open MPI, then a hybrid Open MPI and
OpenMP version. We compare end-to-end runtime, empirical speedup, workload
balance, and Amdahl theoretical speedup.

## Slide 2 - Environment and correctness (40 seconds)

All experiments ran on one local WSL2 Ubuntu node with Open MPI 4.1.6 on an
Intel i7-13700H. It has 14 physical cores and 20 logical CPUs. We use n equals
60 million for process and hybrid scaling, and 30 n values for the
problem-size comparison. Each scaling value is the median of three full runs.

The root writes the final list. We independently generated the expected primes
with a sieve and compared every output element. Both Task 1 and Task 2
generated 3,562,115 sorted primes below 60 million.

## Slide 3 - Task 1 design and balance (55 seconds)

We exclude even candidates and test only odd divisors up to the square root.
MPI uses cyclic candidate-index ownership. With p ranks, rank r evaluates
r, r plus p, r plus 2p, and so on. A contiguous block would assign larger
candidates to later ranks, where trial division is generally more expensive.
Cyclic ownership therefore mixes cheap and expensive candidates across every
rank.

At four ranks, each rank receives almost exactly 7.5 million candidates. The
measured max/min computation-time ratio is 1.017, which directly demonstrates
balanced work. Each rank returns only primes, then rank 0 uses a heap-based
k-way merge to write globally ordered output.

## Slides 4 and 5 - Task 1 results (70 seconds)

These graphs compare serial, Open MPI, and the supplied Week 4 OpenMP baseline
as n increases. Speedup rises when n is large enough for computation to
dominate MPI startup and root-output overhead.

For process scaling at 60 million, Task 1 reaches 6.66 times empirical
speedup at 16 ranks. It is below the Amdahl estimate because that ideal model
does not include increasing collective latency, root aggregation, cache
effects, or file writing.

## Slide 6 - Task 2 design and balance (45 seconds)

The hybrid program keeps the cyclic MPI partition, then each MPI process uses
an OpenMP parallel-for with dynamic scheduling. Dynamic scheduling helps the
threads absorb variation from early composite exits and expensive prime tests.

At four ranks and two threads per rank, the max/min rank computation-time
ratio is 1.054. This confirms balanced distributed work while OpenMP improves
the local shared-memory phase.

## Slide 7 - Hybrid scaling (55 seconds)

At four MPI ranks and four OpenMP threads per rank, the hybrid program reaches
6.63 times speedup. Increasing threads beyond available cores does not keep
improving time. For example, four ranks and sixteen threads per rank has
lower efficiency because 64 workers oversubscribe the machine and contend for
cores and memory bandwidth.

## Slide 8 - Amdahl, limitations, and Q&A (45 seconds)

We measure the parallel part as maximum search-and-compaction time and define
the serial part as the one-worker total minus that time. Amdahl speedup is
T1 divided by Ts plus Tp over W, where W is MPI ranks for Task 1 and total
MPI times OpenMP workers for Task 2.

Actual speedup is lower because MPI communication, the root merge and file
write, and scheduling overhead do not scale ideally. MPI is valuable when
work or memory spans nodes, while OpenMP has lower local communication cost.

## Q&A revision answers

**Why is cyclic better than a block partition here?**

Trial division gets more expensive for large prime candidates. A block gives
later ranks a concentration of larger candidates. Cyclic assignment gives all
ranks a representative range of magnitudes, confirmed by the near-one
per-rank timing ratios.

**Why can actual speedup differ from Amdahl speedup?**

Amdahl assumes a fixed serial fraction and ideal division of the parallel
fraction. The implementation has collective communication, process startup,
root-only merge/output, cache and memory effects, and scheduler overhead.

**Would you always use MPI for prime search?**

No. On one node, OpenMP commonly has lower overhead. MPI becomes useful for
distributed-memory execution across nodes or when the problem exceeds one
node's memory. For this kernel, MPI communication and root aggregation limit
strong scaling at high rank counts.
