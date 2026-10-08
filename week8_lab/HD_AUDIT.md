# HD Audit Against the Specification and Rubric

| Criterion | Status | Verified evidence | Remaining action |
| --- | --- | --- | --- |
| Task 1 correctness and sorted primes `< n` | Pass | Expanded independent-sieve test; representative 60M exact validation | None |
| Task 2 correctness and sorted primes `< n` | Pass | Expanded process/thread matrix; representative 60M exact validation | None |
| Clean/optimised implementation | Pass | Odd-only kernel, compact prime communication, k-way merge, buffered output, warnings enabled | None |
| Balanced/optimal workload distribution | Strong | Block-cyclic comparison at `p=3,4,5`; worst measured ratio 1.058 | Describe as best measured, not universally proven |
| Per-process timing evidence | Pass | `partition_balance.csv`, `local_cluster_execution.txt`, and Appendices 3–4 | None |
| 30 problem sizes | Pass | 20M–78M in `n_scaling.csv` and OpenMP comparator | None |
| Process/thread/core coverage | Pass | MPI ranks 1–16; OpenMP 1–16; hybrid up to 64 workers on 20 logical CPUs | None |
| Seven required graphs | Pass | Editable charts on slides 4–9; mapping documented in `EXPERIMENTS.md` | Open once in PowerPoint before upload |
| Empirical speedup arithmetic | Pass | Independently recalculated from stored runtimes | None |
| Amdahl theoretical speedup | Pass | Measured `Ts`/`Tp`; independent recalculation; appendix formula/table | Explain baseline distinction in Q&A |
| Experimental configuration | Pass | Node, CPU, MPI, ranks, threads, workers, `n`, repeats and timing scope documented | None |
| Representative execution evidence | Pass | 60M commands/timings/validation, inspectable `n=100` prime files, and authentic local WSL2 captures in Appendices 3–4 | Add CAAS evidence only if CAAS is actually used |
| Presentation quality and duration | Pass | Nine-slide 6:25 script; two appendices; limitations/future work | Rehearse without reading verbatim |
| Q&A independence | Prepared | `Q_AND_A_GUIDE.md` covers code, partition, MPI/OpenMP, speedup and limitations | Practise explaining in own words |
| Required Moodle files | Partial | Sources, final evidence deck, AI declaration, and local execution screenshots exist | Export prompt-record PDF before upload |

## Material correction made during audit

The original pure cyclic strategy appeared balanced at four ranks but was not
robust. Odd MPI process counts aligned candidate residue classes with ranks,
causing extreme measured imbalance. The implementation was refined to a
64-candidate block-cyclic strategy while retaining pure cyclic and block modes
only for comparison. No working search, gather, merge, or output component was
rebuilt.

## Submission blockers

The implementation and presentation artifacts are ready, but the submission
is not complete until one user-originated item is added:

1. the authentic AI prompt/conversation record PDF.

This cannot be fabricated from the repository and is deliberately marked as
missing in the appendix and submission checklist. The authentic local WSL2
terminal captures are already included in Appendices 3–4; they do not claim
CAAS or multi-node execution.
