# FIT3143 Lab 2 Presentation Script

Target time: **6 minutes 25 seconds** for slides 1–9. Slides 10–11 are Q&A
appendices and are not part of the timed presentation.

## Slide 1 — Title (15 seconds)

We implemented prime search below `n` first with Open MPI and then with a
hybrid MPI and OpenMP design. I will focus on correctness, workload balance,
measured scalability, and why the results differ from Amdahl's ideal model.

## Slide 2 — Setup and correctness (40 seconds)

The experiments used one WSL2 Ubuntu node with Open MPI 4.1.6 on an Intel
i7-13700H: 14 physical cores and 20 logical CPUs. Process and hybrid scaling
use `n = 60 million`; the problem-size study has all 30 required values from
20 to 78 million. Process and hybrid points are medians of three full runs;
the 30-point sweep is one run per value and every run exceeds one second.

We checked complete outputs against an independent sieve. The tests covered
small boundary values, MPI ranks 1 to 5, and hybrid thread counts 1 to 3. Both
programs produce a sorted, unique and complete list of primes strictly below
`n`; at 60 million there are 3,562,115 primes.

## Slide 3 — Task 1 partition (50 seconds)

We test only odd candidates and odd divisors up to the square root. MPI uses
block-cyclic ownership: chunks of 64 odd-candidate indices are assigned
round-robin. This preserves short local runs while spreading small and large
candidates across ranks.

The controlled comparison exposed an important flaw in pure one-candidate
cyclic ownership. At three and five ranks, divisibility patterns align with
the rank stride, producing max/min timing ratios of about 628 and 491.
Contiguous blocks were also unbalanced. Block-cyclic stayed at or below 1.06
for all tested process counts, so it had the lowest measured worst-case
imbalance. Rank 0 then performs an `O(m log p)` k-way merge with buffered
output.

## Slide 4 — Runtime over 30 `n` values (35 seconds)

This graph shows all 30 measurements. The current serial and four-process MPI
curves were measured together. The four-thread OpenMP curve is our prior Week
4 comparator from the same machine but a separate session, so I treat small
point-to-point differences as measurement noise. Runtime grows with `n`, and
the MPI implementation remains consistently faster than serial.

## Slide 5 — Speedup over 30 `n` values (35 seconds)

For a fair calculation, each implementation uses the serial baseline from its
own measurement session. Four-process MPI stays around 3.3 to 3.6 times
speedup. OpenMP is noisier and ranges roughly from 2 to 3 times. This graph
shows strong-scaling benefit across the full required problem-size range,
without omitting difficult points.

## Slide 6 — Task 1 process scaling and Amdahl (55 seconds)

At matched worker counts, both MPI and OpenMP improve through 16 workers; on
this single node OpenMP reaches the higher final speedup because it avoids MPI
process communication and aggregation overhead.

Task 1 reaches 6.02 times empirical speedup at 16 ranks, while Amdahl predicts
12.67. The theoretical curve uses the measured one-rank decomposition:
`Ts = 0.427566 seconds` and `Tp = 23.948293 seconds`. The widening gap is not an
arithmetic error: basic Amdahl assumes the parallel portion divides ideally
and does not include rank-dependent collectives, resource contention, or the
root-only merge and file write.

## Slide 7 — Task 2 hybrid design and balance (45 seconds)

Task 2 keeps the same block-cyclic MPI partition, then applies OpenMP dynamic
scheduling with chunk size 64 inside each process. MPI balances process-level
work, while dynamic scheduling smooths residual differences between cheap
composite tests and expensive prime tests.

In the representative four-rank run, Task 1's rank timing ratio is 1.009. With
two threads per rank, Task 2's ratio is 1.026 and runtime falls from 6.97 to
5.16 seconds, a 1.35 times improvement for that run. Both outputs pass exact
validation.

## Slide 8 — Hybrid scaling (55 seconds)

With four MPI ranks, increasing from one to four threads per rank raises the
speedup relative to Task 1 from about 1 to 2.39. More threads then provide no
additional benefit because 32 and 64 workers oversubscribe 20 logical CPUs.

The matched-worker graph compares hybrid and OpenMP at equal totals. The best
measured hybrid point overall is one MPI process with 16 threads at 8.46 times
speedup. The four-by-four configuration reaches 8.03 times. The trend shows
that adding parallel workers helps only while useful computation outweighs
communication, scheduling, and hardware contention.

## Slide 9 — Hybrid Amdahl gap and limitations (55 seconds)

For Task 2, the measured one-worker values are `Ts = 0.411327 seconds` and
`Tp = 23.749574 seconds`. Amdahl's ideal continues rising with total workers,
but empirical speedup plateaus and then falls after the available logical CPUs
are exceeded.

The main limitations are single-node evidence, collective and rank-0 output
overhead, and the trial-division kernel itself. Future work would test
authentic multi-node CAAS runs, use segmented-sieve work units, and distribute
or parallelise output. The key conclusion is that the partition is balanced,
but balance alone cannot remove communication and serial aggregation costs.

## Timing check

| Slide | Seconds | Cumulative |
| ---: | ---: | ---: |
| 1 | 15 | 0:15 |
| 2 | 40 | 0:55 |
| 3 | 50 | 1:45 |
| 4 | 35 | 2:20 |
| 5 | 35 | 2:55 |
| 6 | 55 | 3:50 |
| 7 | 45 | 4:35 |
| 8 | 55 | 5:30 |
| 9 | 55 | 6:25 |
