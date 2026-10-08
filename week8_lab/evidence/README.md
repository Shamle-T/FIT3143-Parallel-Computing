# Representative Execution Evidence

- `commands_and_validation.txt` records the exact commands used for the
  inspectable sample and the file comparison result.
- `task1_primes_n10000.txt` and `task2_primes_n10000.txt` are the actual
  generated prime files. They contain 1,229 lines, are byte-identical, and
  list every prime strictly below 10,000 in ascending order.
- `../results/local_cluster_execution.txt` is the performance-scale evidence
  at `n = 60,000,000`, including rank timings and independent full-output
  validation.

The sample files make sorted output directly inspectable without adding two
large copies of the 60-million result. The 60M transcript supplies the stable
performance and balance evidence. `screenshots/task1_local_mpi_execution.png`
and `screenshots/task2_local_hybrid_execution.png` are authentic local WSL2
captures used in presentation Appendices 3–4; they are execution proof only,
not graph data, and do not represent a CAAS or multi-node run.
