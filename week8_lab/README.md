# FIT3143 Lab 2 — Open MPI and Hybrid Prime Search

The assessed implementations are `task1.c` (Open MPI) and `task2.c`
(Open MPI + OpenMP). Both find every prime strictly below `n`, gather only
prime values, and write a globally sorted output file on rank 0.

## Build and verify

Use a Linux environment with Open MPI and OpenMP:

```bash
make clean all
make verify
```

`make verify` compares the complete output with an independent sieve for
`n = 0, 1, 2, 3, 4, 10, 1000`. Task 1 is checked with 1–5 MPI processes and
all three available partition strategies. Task 2 is checked with 1–5 MPI
processes and 1–3 OpenMP threads per process.

## Run

```bash
mpirun --bind-to none --oversubscribe -np 4 \
  ./build/task1 60000000 --strategy block-cyclic --benchmark

mpirun --bind-to none --oversubscribe -np 4 \
  ./build/task2 60000000 2
```

Task 1 accepts `block-cyclic`, `cyclic`, or `block`. `block-cyclic` is the
default and selected strategy. It assigns chunks of 64 odd-candidate indices
round-robin, combining the locality of blocks with a mix of candidate
magnitudes on every rank. The pure cyclic and contiguous block modes remain
only for the controlled partition comparison.

Task 2 uses the same MPI block-cyclic ownership, then OpenMP
`schedule(dynamic, 64)` inside each process. MPI is initialised with
`MPI_THREAD_FUNNELED`; only the main thread calls MPI.

Both implementations:

- read and validate arguments only on rank 0, then broadcast configuration;
- test only odd candidates and odd divisors up to `sqrt(candidate)`;
- compact local prime values before communication;
- gather sorted local prime lists with `MPI_Gatherv`;
- perform an `O(m log p)` heap-based k-way merge on rank 0;
- use a 1 MiB output buffer; and
- report end-to-end, computation, gather, post-processing, and per-rank
  balance timings.

## Why block-cyclic is used

Pure one-candidate cyclic ownership has a residue-class pathology when the
process count has an odd factor: one rank can receive a disproportionate set
of candidates divisible by that factor. At `n = 20,000,000`, the measured
max/min computation-time ratios were:

| MPI processes | Block-cyclic | Pure cyclic | Contiguous block |
| ---: | ---: | ---: | ---: |
| 3 | 1.047 | 627.764 | 2.204 |
| 4 | 1.058 | 1.032 | 2.615 |
| 5 | 1.057 | 491.084 | 2.780 |

These are medians of three runs from `results/partition_balance.csv`.
Block-cyclic had the lowest measured worst-case imbalance across the tested
process counts and is robust to both odd and even `p`.

## Reproduce experiments

```bash
./scripts/benchmark_n.sh
./scripts/run_benchmarks.sh task1-process
./scripts/run_benchmarks.sh hybrid
./scripts/run_benchmarks.sh balance
./scripts/benchmark_openmp_threads.sh
python3 scripts/generate_charts.py
python3 scripts/audit_results.py
```

The runner disables MPI rank binding so OpenMP threads can use the machine's
cores. `--oversubscribe` allows the required beyond-core-count cases.

Key result files:

| File | Contents |
| --- | --- |
| `results/n_scaling.csv` | Current Task 1 serial/MPI sweep for 30 `n` values, 20M–78M |
| `results/openmp_n_scaling.csv` | Prior Week 4 OpenMP comparator for the same 30 values |
| `results/task3_n_scaling.csv` | Matched serial baseline for the prior OpenMP sweep |
| `results/openmp_thread_scaling.csv` | Current OpenMP thread scaling at `n = 60M` |
| `results/task1_process_scaling.csv` | Task 1 ranks 1–16; median of three runs |
| `results/hybrid_scaling.csv` | Hybrid configurations from 1 to 64 workers; median of three runs |
| `results/partition_balance.csv` | Controlled partition comparison at `p = 3, 4, 5` |
| `results/local_cluster_execution.txt` | Representative commands, configuration, rank timings, and exact validation |

The 30-point current MPI `n` sweep contains one run per `n`; the process,
hybrid, and partition studies use medians of three. The Week 4 OpenMP `n`
sweep is a separate prior measurement session. Its speedup therefore uses its
own matched serial measurements from `task3_n_scaling.csv`, not the current
MPI session's serial times.

Do not overwrite `openmp_n_scaling.csv` or `task3_n_scaling.csv` unless both
the OpenMP series and its serial baseline are remeasured in the same new
session. `run_benchmarks.sh n` can create a new fully matched four-program
session in `runtime_n_scaling.csv` if a fresh comparison is required.

## Amdahl analysis

For each parallel implementation, the one-worker measurement is decomposed as:

```text
T1 = Ts + Tp
fs = Ts / T1
S_Amdahl(W) = T1 / (Ts + Tp / W)
```

`Tp` is the measured maximum search-and-compaction time. `Ts = T1 - Tp`
contains communication, root merge, and file output. `W` is the MPI process
count for Task 1 and total MPI × OpenMP workers for Task 2.

Measured inputs:

| Program | Ts | Tp | fs |
| --- | ---: | ---: | ---: |
| Task 1 | 0.427566 s | 23.948293 s | 0.017541 |
| Task 2 | 0.411327 s | 23.749574 s | 0.017024 |

Empirical speedup uses the Week 4 serial end-to-end program as the baseline;
theoretical speedup uses the measured one-worker decomposition of the relevant
parallel program. This baseline distinction is stated in the presentation.

## Presentation and submission

Use only
`output/FIT3143_Week8_Lab2_Presentation_HD_Submission_With_Evidence_And_AI_Declaration.pptx`
for submission. Slides 1–9 are the timed 6–7 minute presentation; slides
10–14 are appendices for Q&A, local execution evidence, and the AI declaration.
See
`PRESENTATION_SCRIPT.md`, `Q_AND_A_GUIDE.md`,
`HD_AUDIT.md`, and `SUBMISSION_CHECKLIST.md`.
