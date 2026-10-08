# Q&A Guide

Use this as revision material. Answer in your own words during assessment.

## Explain the code in one minute

Rank 0 parses `n` and the configuration, then broadcasts it. The search space
contains only odd candidates from 3 to `n - 1`. MPI assigns 64-candidate
chunks block-cyclically. Each rank performs trial division using odd divisors
up to `sqrt(candidate)` and stores only primes. Task 2 adds an OpenMP dynamic
loop inside each rank. Counts are gathered first, then prime arrays are
gathered with `MPI_Gatherv`. Because every local list is sorted, rank 0 uses a
min-heap to merge them in `O(m log p)` order and writes through a large buffer.

## Why block-cyclic instead of pure cyclic or block?

A contiguous block gives later ranks larger candidates, which tend to cost
more. Pure one-candidate cyclic looks balanced at four ranks but fails badly at
odd process counts: the stride can align with divisibility classes, so one
rank receives many candidates rejected by the same small factor. At `p = 3`
and `p = 5`, measured max/min time ratios were about 628 and 491. Chunks of 64
assigned round-robin break that alignment while still distributing candidate
magnitudes. Its worst measured ratio for `p = 3–5` was 1.058.

## Is the partition mathematically optimal?

Do not claim a universal proof. Say: **it is the best measured strategy among
the three controlled alternatives and is robust for the tested odd and even
process counts**. The direct evidence is per-rank computation time, not only
equal candidate counts.

## Why is candidate count not enough to prove balance?

Trial division is irregular. A composite with a small factor exits quickly;
a prime may test every odd divisor up to its square root. Equal counts can
still have unequal work. Per-rank computation times capture the real cost.

## What does MPI do?

- `MPI_Init` / `MPI_Init_thread`: starts the MPI processes.
- `MPI_Bcast`: sends `n` and configuration from rank 0.
- `MPI_Gather`: collects counts and timing evidence.
- `MPI_Gatherv`: collects variable-length prime arrays.
- `MPI_Reduce`: obtains maximum/minimum phase times.
- `MPI_Barrier`: aligns ranks for comparable end-to-end timing.

Task 2 requests `MPI_THREAD_FUNNELED`, meaning multiple OpenMP threads may
exist but only the main thread makes MPI calls.

## What does OpenMP add?

Every MPI process creates an OpenMP team for its local candidate array.
`schedule(dynamic, 64)` lets a thread take another chunk when it finishes,
reducing thread-level imbalance from irregular primality-test cost.
`omp_set_dynamic(0)` prevents the runtime silently changing the requested team
size.

## How is sorted output preserved?

Each rank compacts primes in increasing local-index order. The rank lists are
therefore individually sorted but interleaved globally. Rank 0 puts the first
value from each rank in a min-heap, repeatedly writes the smallest, and
inserts the next value from that same rank. This is a k-way merge.

## Why not gather a Boolean for every candidate?

Gathering only primes reduces communication and root memory. At `n = 60M`,
there are 3,562,115 primes but almost 30 million odd candidates.

## Define the reported timings

- **Computation:** primality testing and local compaction; rank maximum is used
  for the critical parallel phase.
- **Gather:** MPI result-count and variable-length value communication.
- **Post-processing:** root k-way merge and file writing.
- **End-to-end:** synchronized start through completed output, reported as the
  slowest rank's wall time.

## Empirical speedup and efficiency

```text
empirical speedup = serial end-to-end time / parallel end-to-end time
efficiency = empirical speedup / worker count
```

Task 1 uses MPI ranks as workers. Task 2 uses `MPI ranks × threads per rank`.

## Amdahl calculation

From the one-worker version of each parallel program:

```text
Ts = one-worker total - maximum computation time
Tp = maximum computation time
S(W) = (Ts + Tp) / (Ts + Tp / W)
```

Task 1: `Ts = 0.427566 s`, `Tp = 23.948293 s`, `fs = 0.017541`.
Task 2: `Ts = 0.411327 s`, `Tp = 23.749574 s`, `fs = 0.017024`.

Empirical speedup uses the Week 4 serial executable; the Amdahl curve uses the
one-worker decomposition of the relevant parallel program. State this baseline
difference explicitly.

## Why is empirical speedup below Amdahl?

Amdahl models a fixed serial fraction and ideal division of the parallel
fraction. It does not include worker-dependent collective latency, MPI process
startup, cache and memory contention, OpenMP scheduling overhead,
oversubscription, or increasing root aggregation cost.

## Why does oversubscription hurt?

The machine exposes 20 logical CPUs. Configurations with 32 or 64 workers must
time-share those CPUs. Context switching and competition for caches and memory
bandwidth raise runtime, so speedup plateaus or falls and efficiency drops.

## Why can OpenMP beat MPI on one node?

OpenMP shares memory and does not need inter-process result transfer. MPI adds
collective communication and separate process state. MPI becomes more valuable
when execution spans nodes or data exceeds one node's memory.

## Main limitations

- Current verified evidence is one local node, not a networked multi-node run.
- Rank 0 remains responsible for the final merge and all output.
- Trial division performs far more work than a segmented sieve.
- The 30-point `n` sweep has one sample per point.
- The Week 4 OpenMP `n` sweep is a separate prior session.

## Sensible future work

- Run the same matrix on authentic multi-node CAAS hardware.
- Replace individual trial division with segmented-sieve work units.
- Use distributed or MPI I/O to reduce the rank-0 output bottleneck.
- Repeat each `n` value and report confidence intervals.
- Tune MPI and OpenMP chunk sizes per architecture.
