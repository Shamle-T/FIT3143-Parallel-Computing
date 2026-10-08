# Experimental Configuration and Evidence

## Environment

| Item | Verified configuration |
| --- | --- |
| Nodes | 1 local WSL2 Ubuntu node |
| CPU | Intel Core i7-13700H |
| Physical / logical CPUs | 14 cores / 20 logical CPUs |
| MPI runtime | Open MPI 4.1.6 |
| MPI binding | `--bind-to none` so OpenMP teams are not pinned to one CPU |
| Oversubscription | Enabled explicitly with `--oversubscribe` |
| Output | Rank 0 writes one sorted text file per implementation |

This evidence verifies single-node behaviour. It does not prove multi-node
network performance and must not be described as a CAAS run unless an
authentic CAAS execution is added.

## Measurement matrix

| Experiment | `n` | MPI processes | Threads/process | Total workers | Repetitions | Data |
| --- | ---: | --- | --- | --- | ---: | --- |
| Current Task 1 `n` sweep | 20M–78M, 30 values | 4 | N/A | 4 | 1 per `n` | `results/n_scaling.csv` |
| Prior Week 4 OpenMP `n` sweep | same 30 values | N/A | 4 total threads | 4 | 1 per `n` | `results/openmp_n_scaling.csv` |
| Task 1 process scaling | 60M | every integer 1–16 | N/A | 1–16 | median 3 | `results/task1_process_scaling.csv` |
| Current OpenMP thread scaling | 60M | N/A | 1, 2, 4, 8, 16 | 1–16 | 1 | `results/openmp_thread_scaling.csv` |
| Hybrid scaling | 60M | 1, 2, 4, 8, 16 by configuration | 1, 2, 4, 8, 16 by configuration | 1–64 | median 3 | `results/hybrid_scaling.csv` |
| Partition comparison | 20M | 3, 4, 5 | N/A | 3–5 | median 3 | `results/partition_balance.csv` |
| Representative exact run | 60M | 4 | 2 for Task 2 | 4 / 8 | 1 | `results/local_cluster_execution.txt` |

All reported end-to-end timings include synchronization, result
communication, rank-0 merge, and file writing. Computation, gather, and root
post-processing are also recorded separately. All 30 `n`-sweep runtimes exceed
one second.

## Required graph mapping

| Graph | Requirement covered | Source data |
| ---: | --- | --- |
| 1 | Runtime: MPI vs serial and Week 4 OpenMP as `n` increases | `n_scaling.csv`, `openmp_n_scaling.csv` |
| 2 | Empirical speedup: MPI and OpenMP as `n` increases | `n_scaling.csv`, `task3_n_scaling.csv`, `openmp_n_scaling.csv` |
| 3 | MPI processes vs matching OpenMP thread counts | `task1_process_scaling.csv`, `openmp_thread_scaling.csv` |
| 4 | Hybrid vs Task 1 while threads increase at fixed 4 MPI processes | `hybrid_scaling.csv` |
| 5 | Hybrid vs OpenMP at matched total worker counts | `hybrid_scaling.csv`, `openmp_thread_scaling.csv` |
| 6 | Task 1 empirical vs Amdahl speedup for ranks 1–16 | `task1_process_scaling.csv` |
| 7 | Task 2 empirical vs Amdahl speedup along an increasing process/thread path | `hybrid_scaling.csv` |

The submission deck contains all seven as editable native PowerPoint charts.
The PNG files in `results/charts/` are supporting exports, not the evidence
source.

## Workload balance

The selected MPI partition assigns 64-candidate odd-index chunks round-robin.
This prevents both the magnitude imbalance of a single contiguous block and
the residue-class bias of pure cyclic ownership at odd process counts.

Controlled max/min computation-time ratios:

| `p` | Block-cyclic | Pure cyclic | Block |
| ---: | ---: | ---: | ---: |
| 3 | 1.047 | 627.764 | 2.204 |
| 4 | 1.058 | 1.032 | 2.615 |
| 5 | 1.057 | 491.084 | 2.780 |

The representative `n = 60M` execution recorded:

| Program | Configuration | Candidate range/rank | Compute range | Ratio | End-to-end |
| --- | --- | ---: | ---: | ---: | ---: |
| Task 1 | 4 MPI ranks | 7,499,968–7,500,032 | 6.446–6.507 s | 1.009 | 6.965 s |
| Task 2 | 4 MPI × 2 OpenMP | 7,499,968–7,500,032 | 4.567–4.685 s | 1.026 | 5.161 s |

This is direct per-process timing evidence. It supports the claim that the
selected partition is the best measured strategy among those tested; it does
not claim a universal proof for every machine or process count.

## Performance highlights

- Task 1 at 16 ranks: 6.015× empirical speedup versus 12.667× Amdahl.
- Best measured hybrid point: 1 MPI × 16 OpenMP, 8.461× speedup.
- Fixed 4-rank hybrid peaks at 4 threads/rank: 2.386× faster than the
  corresponding Task 1 run.
- 4×8 and 4×16 configurations use 32 and 64 workers on 20 logical CPUs;
  efficiency falls under oversubscription.

## Theoretical speedup audit

Task 1 uses `Ts = 0.427566 s`, `Tp = 23.948293 s`, `fs = 0.017541`.
Task 2 uses `Ts = 0.411327 s`, `Tp = 23.749574 s`, `fs = 0.017024`.

For `W` workers:

```text
S_Amdahl(W) = (Ts + Tp) / (Ts + Tp / W)
```

All CSV speedup, efficiency, and theoretical-speedup columns were independently
recalculated from their stored inputs. No arithmetic discrepancies were found.

The growing empirical/theoretical gap is expected because basic Amdahl
analysis omits worker-dependent collective latency, root-only merge/output,
memory and cache contention, scheduling overhead, and oversubscription.

## Evidence limitations

- The `n` sweep is one run per point, so it shows coverage and trend rather
  than a confidence interval.
- The Week 4 OpenMP `n` sweep was recorded in a separate prior session. Its
  empirical speedup uses the serial measurements from that same prior session.
- The current evidence is local single-node evidence. Authentic local WSL2
  terminal captures are stored in `evidence/screenshots/` and reproduced in
  presentation Appendices 3–4. They support execution verification only and
  are not graph data. Add an authentic CAAS screenshot/run only before making
  a CAAS execution claim.
