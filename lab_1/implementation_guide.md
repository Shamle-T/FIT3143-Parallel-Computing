# FIT3143 Lab #1 — Codex Implementation Guide for an HD-Targeted Submission

> **Purpose:** This file distils the requirements from the **Lab #1 Assessment Specification** and the **Lab #1 Rubric (Draft)** into concrete instructions for an AI coding agent (Codex). The goal is to guide implementation, testing, benchmarking, documentation, and presentation preparation toward the **HD criteria**.
>
> **Scope:** Tasks 1–4: serial C, POSIX Threads, OpenMP, performance experiments/graphs, presentation/documentation, and Q&A preparation.
>
> **Important:** The rubric supplied is explicitly labelled **Draft**. If the teaching team publishes a newer rubric, treat the latest official rubric as authoritative.

---

## 1. Assessment Context and Non-Negotiable Constraints

- Unit/lab topic: **Threads & OpenMP** for **shared-memory parallel computing**.
- The assessment is worth **8% of the unit final mark**.
- Team size: **2 students** unless an exceptional arrangement is approved.
- Required submission files:
  - `task1.c`
  - `task2.c`
  - `task3.c`
  - presentation slides or equivalent documentation
  - AI declaration / prompt records in PDF form where required
- Code, slides, results, and explanations may be assessed through:
  - offline marking,
  - in-class presentation/demo,
  - peer review,
  - Q&A.
- AI tools may be used during preparation, but AI use must be declared and prompt records uploaded as required.
- AI tools are **not allowed during the presentation or oral/coding interview**.
- All submitted files should ideally contain student names, student IDs, and Monash email addresses.
- Missing submissions, absence from the required in-class assessment, or failure to contribute can result in **zero marks**.
- Late penalty stated in the specification: **5% per day**.

### Presentation timing
There is a minor inconsistency in the specification:
- one section states **5 min presentation + 2 min Q&A**,
- Task 4 states **7 min presentation + 1–2 min Q&A**,
- the rubric's HD criterion expects the presentation itself to be **between 6 and 8 minutes**,
- the submission checklist says to keep the full presentation/documentation to about **8 minutes including Q&A**.

**HD-safe interpretation for planning:** prepare a tightly timed presentation of roughly **6–7 minutes of content**, with **1–2 minutes for Q&A**, and confirm the exact session format with the teaching team before submission/presentation.

---

# 2. HD Success Criteria — What Codex Must Optimise For

The methodology component is 20% and presentation/Q&A is 80%. Therefore, Codex must not stop at producing correct code; it must help generate **explainable, benchmarked, experimentally defensible parallel implementations**.

## Task 1 — HD target
The serial program must:
- correctly find all primes **strictly less than `n`**;
- output primes in **ascending order**;
- support terminal output for small `n` and text-file output for larger `n`;
- handle at least `n = 10,000,000`;
- measure execution time;
- perform **significant optimisation** to reduce unnecessary computation;
- use clean, readable, industry-style C;
- include strong comments / documentation suitable for explaining the code in an assessment.

## Task 2 — HD target
The POSIX Threads version must:
- preserve correctness and sorted output;
- genuinely parallelise the prime search using Pthreads;
- remove or minimise unnecessary computation and **parallel bottlenecks**;
- demonstrate total computation speedup **strictly greater than 1**;
- target speedup **close to linear relative to available CPU cores**, where realistically achievable;
- use an explicit, defensible **parallel partitioning / workload distribution scheme**;
- be clean, robust, free from deadlocks/crashes, and industry-style documented.

## Task 3 — HD target
The OpenMP version must:
- preserve correctness and sorted output;
- genuinely parallelise the prime search using OpenMP;
- use **different OpenMP features** to reduce unnecessary computation / bottlenecks;
- demonstrate speedup > 1 and target near-linear speedup where hardware/workload permits;
- use a clear, defensible workload distribution strategy;
- be clean, robust, and industry-style documented.

## Task 4 — HD target
The presentation/documentation must:
- be exceptionally clear, organised, concise, precise, and accurate;
- use **parallel-computing terminology**, not vague generic computing language;
- include all required content and graphs with **no important omissions**;
- explain optimisations and workload distribution;
- present statistically meaningful performance evidence;
- conclude with major issues, limitations, and future work;
- prepare students to answer all Q&A correctly **without external tools**.

---

# 3. Global Coding Rules for All Three Programs

Codex should apply the following conventions consistently unless the team's existing codebase requires otherwise.

## 3.1 Correctness contract
For an input `n`, the result set must be exactly:

```text
{ p | p is prime and 2 <= p < n }
```

Examples:
- `n = 10` -> `2, 3, 5, 7`
- `n <= 2` -> empty result

The output must remain sorted in ascending order for Tasks 1, 2, and 3.

## 3.2 Prime checking
The specification explicitly hints that divisibility testing only needs to continue up to the square root of the candidate.

Codex should therefore avoid a naive test from `2` to `k-1`.

### HD-oriented implementation guidance
A reasonable optimised primality predicate is:
1. Handle `2` explicitly.
2. Reject values `< 2`.
3. Reject even values greater than 2.
4. Test only odd divisors up to `sqrt(k)` (or equivalently `d <= k / d` to avoid repeated `sqrt`).

This reduces unnecessary divisibility checks while keeping the algorithm simple enough to explain during Q&A.

> Do not introduce a radically different prime algorithm (e.g. a sieve) unless the team deliberately chooses it and can fully explain the design, parallelisation, memory trade-offs, benchmarking fairness, and why the serial/parallel comparison remains valid. The task wording and hints strongly frame the exercise around candidate-wise prime testing and parallel workload distribution.

## 3.3 Output policy
Implement a clear output mode:
- small `n` (example in specification: `n < 100`) -> standard output;
- larger `n` (example: `n > 100`) -> text file.

Avoid timing contamination:
- benchmark the **computation phase** separately from large file I/O wherever possible;
- if total runtime including output is also measured, label it separately.

Reason: printing a huge prime list can dominate runtime and destroy the validity of parallel speedup comparisons.

## 3.4 Timing policy
Timing must be implemented consistently across Tasks 1–3.

Record at minimum:
- elapsed computation time in seconds;
- input `n`;
- thread count (Tasks 2–3);
- implementation type (`serial`, `pthread`, `openmp`).

Speedup formula:

```text
Speedup S = T_serial / T_parallel
```

Use the same machine, comparable runtime conditions, same input, same compiler optimisation level, and the same definition of the timed region for fair comparisons.

## 3.5 Build quality
Codex must make compilation easy and warning-free.

The assessment explicitly notes:
- `math.h` for `sqrt()`;
- GCC `-lm` when linking the math library.

Typical implementation-specific flags that may be needed are:
- POSIX Threads: `-pthread`
- OpenMP: `-fopenmp`

Recommended development build policy:
- compile with warnings enabled;
- use an optimisation level suitable for performance experiments;
- keep the compiler/options identical across comparable implementations except for the parallel-runtime-specific flags.

Example build patterns (implementation guidance, not quoted assessment commands):

```bash
gcc -O3 -Wall -Wextra -pedantic task1.c -o task1 -lm
gcc -O3 -Wall -Wextra -pedantic task2.c -o task2 -pthread -lm
gcc -O3 -Wall -Wextra -pedantic task3.c -o task3 -fopenmp -lm
```

## 3.6 Code style / documentation
HD requires industry-level clarity.

Every source file should include:
- file header: purpose, authors/team details, compile command, run command;
- descriptive function names;
- small cohesive functions;
- comments explaining **why**, especially parallel design decisions;
- no large blocks of redundant comments that merely restate obvious syntax;
- constants instead of unexplained magic numbers;
- input validation and error handling;
- checked return values for allocations, files, and thread operations;
- no data races, deadlocks, invalid memory access, or resource leaks.

---

# 4. Task 1 — `task1.c` Serial Prime Search

## 4.1 Required behaviour
Create a serial C program that:
1. obtains integer `n` from the user;
2. searches for all primes strictly less than `n`;
3. stores or otherwise produces them in ascending order;
4. outputs to terminal for small input and to a text file for large input;
5. works for at least `10,000,000`;
6. measures and prints execution time.

## 4.2 HD-oriented design
Codex should keep this serial implementation as the **baseline** for all speedup calculations.

Suggested structure:

```text
parse/validate input
start timer
search candidate numbers in ascending order
store prime results
stop timer
print timing summary
output prime list (stdout or file)
free resources
```

### Serial optimisation priorities
- special-case `2`;
- skip even candidate numbers after 2;
- test divisors only up to the square root;
- skip even divisors;
- avoid repeated expensive work inside inner loops;
- use a result representation that does not create avoidable overhead.

## 4.3 Task 1 verification
Codex should add deterministic checks before benchmarking:
- test `n = 0, 1, 2, 3, 10, 100`;
- verify no composite values are emitted;
- verify ordering;
- verify the largest output is `< n`;
- verify serial result count/list exactly matches Task 2 and Task 3 for the same `n`.

## 4.4 Task 1 presentation evidence
The rubric expects:
- coding approach clearly explained;
- **sample output** shown;
- optimisations explained;
- for HD, **statistically significant experimental results** showing the improvements from the serial optimisations.

Therefore Codex should preserve or generate data comparing, for example:
- baseline naive serial implementation;
- optimised serial implementation;

for enough non-trivial input sizes to demonstrate the benefit of eliminating unnecessary work.

---

# 5. Task 2 — `task2.c` POSIX Threads

## 5.1 Required behaviour
Implement the prime search in C using **POSIX Threads**.

Must:
- accept `n`;
- accept or otherwise configure thread count;
- distribute prime-testing work among threads;
- return the same prime list as Task 1;
- preserve ascending order in final output;
- time the computation;
- compute speedup against Task 1;
- support experiments over different `n` values and thread counts.

## 5.2 Partitioning is a core marking point
The specification repeatedly asks:
- how work is distributed;
- whether the workload is balanced;
- whether the approach is good;
- how speedup changes with thread count and `n`.

Therefore the partitioning design must be explicit in both code and presentation.

## 5.3 HD-oriented Pthreads partitioning strategy
Codex should prefer a design that reduces load imbalance caused by variable prime-test cost.

Prime tests near larger candidate values can require more divisor checks than smaller values, so a naive single contiguous range per thread can create imbalance.

### Recommended design option: cyclic / strided assignment
After special-casing `2`, operate only on odd candidates.

Thread `t` processes candidate indices in a strided pattern:

```text
candidate = first_odd + 2*t
candidate += 2*num_threads each iteration
```

Benefits:
- spreads small and large candidates across all threads;
- avoids central work-queue lock contention;
- deterministic ownership;
- easy to explain;
- naturally balances variable candidate costs better than coarse contiguous blocks.

### Alternative design
A dynamic shared work queue can improve balancing but introduces synchronisation overhead. If chosen, the presentation must quantify the trade-off.

## 5.4 Avoiding parallel bottlenecks
For HD, Codex should specifically avoid:
- a mutex around every primality test;
- a mutex around every discovered prime;
- serial insertion into one global sorted list inside the hot loop;
- excessive barriers;
- false assumptions that more threads always produce more speedup.

### Recommended result-collection pattern
Each thread should have a **thread-local result buffer**.

After all threads join:
- merge results into a global buffer;
- produce ascending output.

If strided partitioning is used, simple concatenation by thread ID will **not** necessarily be globally sorted. Codex must either:
- merge sorted per-thread lists correctly, or
- use an order-preserving result bitmap/flag array indexed by candidate and perform one final serial scan, or
- perform a final sort (less attractive if avoidable).

A result bitmap/flag array is simple, deterministic, race-free if each candidate index is written by only one thread, and guarantees sorted output during the final scan.

## 5.5 Thread lifecycle and safety
Codex must:
- check `pthread_create()` return values;
- check `pthread_join()` return values;
- use stable per-thread argument structures;
- avoid passing the address of a loop variable as all thread arguments;
- free allocated thread metadata/results;
- avoid shared mutable state unless ownership/synchronisation is explicit.

## 5.6 Pthreads measurements
For each tested configuration record:
- `n`;
- thread count;
- serial time;
- Pthreads time;
- speedup;
- optionally efficiency:

```text
Efficiency E = Speedup / number_of_threads
```

Efficiency is not explicitly required but is useful for explaining why speedup deviates from linear scaling.

## 5.7 Required Task 2 graphs
The specification requires all four:

1. **Serial vs POSIX runtime** with increasing `n`.
2. **POSIX speedup** with increasing `n`.
3. **Serial vs POSIX runtime** with increasing number of threads.
4. **POSIX speedup** with increasing number of threads.

For HD, all graphs must be present with no significant flaws or data omissions.

---

# 6. Task 3 — `task3.c` OpenMP

## 6.1 Required behaviour
Implement the same prime-search problem using **OpenMP**.

Must:
- accept `n`;
- allow thread-count variation;
- distribute workload with OpenMP;
- preserve exact correctness and sorted output;
- measure computation time;
- compare with serial and Pthreads implementations.

## 6.2 HD requirement: use OpenMP features deliberately
The rubric specifically expects significant optimisation using **different OpenMP features**.

Codex should not produce only a trivial `#pragma omp parallel for` and stop.

The implementation/documentation should demonstrate deliberate use of relevant features such as:
- `parallel for`;
- `schedule(...)`;
- `num_threads(...)` or runtime thread configuration;
- private/shared variable scoping where appropriate;
- reductions only if a reduction is actually needed;
- barriers/`nowait` only when semantically justified.

Do not add directives merely to appear sophisticated; every feature must have a measurable or correctness-related purpose.

## 6.3 OpenMP scheduling / workload balance
Because primality-test cost varies per candidate, experiment with scheduling.

HD-oriented candidates:
- `schedule(static)` — low scheduling overhead, deterministic chunks;
- `schedule(dynamic, chunk)` — better load balancing at increased scheduler overhead;
- possibly `schedule(guided)` — decreasing chunk sizes, potentially useful for irregular work.

Codex should benchmark at least two meaningful scheduling choices and retain the best-supported choice for the final implementation/presentation.

### Important
Do not claim dynamic/guided is automatically better. Use measured evidence.

## 6.4 Result collection
As with Pthreads, avoid critical-section insertion for every prime.

Prefer:
- per-candidate flag array where each loop iteration writes to a unique index; or
- thread-local buffers followed by a controlled merge.

Then scan in ascending candidate order to generate sorted output.

## 6.5 Required Task 3 / comparison graphs
The specification requires:

5. **Serial vs OpenMP runtime** with increasing `n`.
6. **OpenMP speedup** with increasing `n`.
7. **POSIX Threads vs OpenMP runtime** with increasing `n`.
8. **POSIX Threads vs OpenMP runtime** with increasing number of threads.

These must be complete and free from important omissions for an HD-level presentation.

---

# 7. Experimental Methodology — Critical for HD

## 7.1 Number of input sizes
The specification says that to obtain statistically convincing evidence, test with **at least 30 different values of `n`**.

Codex should generate a repeatable benchmark plan with 30+ input sizes large enough to produce meaningful compute times.

The specification recommends first testing above **10,000,000** for the parallel comparisons and increasing `n` as needed depending on hardware.

## 7.2 Thread-count sweep
Test:
- 1 thread;
- increasing thread counts;
- at least up to the number of CPU cores available;
- at least one or more counts **above the available core count** to demonstrate oversubscription behaviour.

The presentation must be able to answer:
- what happens when threads < cores;
- around threads = cores;
- threads > cores;
- why speedup stops scaling linearly.

## 7.3 Repetitions
The specification says results must be statistically convincing but does not prescribe a repetition count.

HD-oriented experimental guidance:
- execute each (`implementation`, `n`, `thread_count`) configuration multiple times;
- store every raw run;
- report a robust aggregate such as median or mean;
- optionally report variability (standard deviation or min/max);
- document the selected statistic.

This prevents one noisy run from driving conclusions.

## 7.4 Fairness controls
Use the same:
- hardware;
- compiler and optimisation level;
- prime-checking logic where possible;
- input values;
- timed computation region;
- output suppression policy during timing;
- measurement method.

Record machine information for the presentation:
- CPU model;
- number of physical/logical cores if known;
- OS;
- compiler version;
- compile flags.

## 7.5 Preventing invalid performance measurements
Codex should make it difficult to accidentally benchmark I/O instead of computation.

Recommended modes:

```text
--benchmark      compute and time, suppress huge prime output
--print          print primes (small n)
--output FILE    save primes after computation
```

If the assessment expects a simpler interface, retain the required user-facing behaviour but implement an internal benchmark mode for data collection.

---

# 8. Benchmark Data File Format

Codex should emit machine-readable CSV so graphs can be reproduced.

Recommended schema:

```csv
implementation,n,threads,run,compute_seconds,prime_count
serial,10000000,1,1,0.000000,0
pthread,10000000,4,1,0.000000,0
openmp,10000000,4,1,0.000000,0
```

A separate post-processing step can compute:

```text
speedup = serial_time / parallel_time
```

Do not manually copy timing values into graph code if avoidable. Graphs should be reproducible from raw CSV.

---

# 9. Automated Correctness Testing

Before collecting performance data, Codex should implement or provide a validation workflow.

## 9.1 Cross-implementation equivalence
For many small/medium `n` values:

```text
Task1 output == Task2 output == Task3 output
```

Compare:
- prime count;
- exact sequence;
- ordering;
- boundary condition `< n`.

## 9.2 Edge cases
Include at least:
- negative input if accepted by parser -> reject clearly;
- `0`;
- `1`;
- `2`;
- `3`;
- small odd/even `n`;
- `100`;
- `10,000,000`;
- large benchmark values used in graphs.

## 9.3 Parallel correctness stress
Run Task 2 and Task 3 repeatedly with different thread counts to detect non-deterministic failures/races.

---

# 10. Presentation / Documentation Content Plan

The rubric places **80%** on presentation quality/content/Q&A, so Codex should help produce presentation-ready evidence, not just source code.

## 10.1 Task 1 section — ~1 minute
Must show:
- serial algorithm;
- sample output;
- optimisation decisions;
- evidence of the performance impact of the optimisations.

Recommended visual:
- a short flow diagram or concise pseudocode;
- one small sample output box;
- one mini graph/table showing naive vs optimised serial timing.

## 10.2 Task 2 section — ~3 minutes
Must show:
- Pthreads implementation approach;
- partitioning/workload distribution;
- why the scheme is balanced;
- bottleneck reduction;
- required graphs 1–4.

Use explicit terminology:
- thread creation/join;
- work partitioning;
- load balance;
- synchronisation overhead;
- critical section / mutex if applicable;
- serial fraction;
- speedup;
- efficiency;
- oversubscription;
- memory contention / cache effects where supported by observations.

## 10.3 Task 3 section — ~2 minutes
Must show:
- OpenMP approach;
- scheduling/workload distribution;
- OpenMP features used;
- required graphs 5–8;
- direct comparison with Pthreads.

Use explicit terminology:
- work-sharing construct;
- scheduling policy;
- runtime overhead;
- implicit barrier;
- shared/private data;
- thread team;
- scalability.

## 10.4 Conclusion / recommendations — ~1 minute
Must summarise:
- which implementation performed best on the tested hardware;
- how performance changed with `n`;
- how performance changed with thread count;
- why speedup was not exactly equal to thread count;
- observed bottlenecks;
- limitations;
- future work.

Do not make generic claims. Tie every conclusion to measured data.

## 10.5 Appendix
The specification explicitly allows appendix slides after Q&A.

Useful appendix content:
- hardware/compiler details;
- extra schedule comparisons;
- raw benchmark table;
- correctness validation evidence;
- source-code excerpts;
- formula definitions;
- answers to likely Q&A questions.

---

# 11. Required Graph Checklist

Codex should refuse to consider Task 4 complete until all eight graphs exist.

- [ ] Graph 1 — Serial vs Pthreads runtime as `n` increases
- [ ] Graph 2 — Pthreads speedup as `n` increases
- [ ] Graph 3 — Serial vs Pthreads runtime as thread count increases
- [ ] Graph 4 — Pthreads speedup as thread count increases
- [ ] Graph 5 — Serial vs OpenMP runtime as `n` increases
- [ ] Graph 6 — OpenMP speedup as `n` increases
- [ ] Graph 7 — Pthreads vs OpenMP runtime as `n` increases
- [ ] Graph 8 — Pthreads vs OpenMP runtime as thread count increases

### Graph quality rules
Every graph should include:
- descriptive title;
- labelled x/y axes with units;
- legend where multiple series exist;
- readable scale;
- no cherry-picked missing points;
- the same underlying benchmark data used in tables/conclusions.

For speedup graphs, consider a reference line for ideal linear speedup when the x-axis is thread count, if useful and clearly labelled.

---

# 12. Q&A Preparation — 40% of the Rubric

The Q&A component alone is **40%**, so the code must be built to be understood by the students, not merely executed.

Codex should generate a `Q_AND_A.md` or appendix section covering at least the following.

## 12.1 Core questions from the specification
Be able to answer concisely:

1. What is speedup?
2. Why might measured speedup be unreasonable?
3. How does increasing `n` affect speedup?
4. How does increasing the number of threads affect speedup?
5. Why does speedup stop improving after some thread count?
6. How is work distributed in the Pthreads implementation?
7. Why is the partitioning scheme load-balanced or not?
8. How is work distributed in the OpenMP implementation?
9. What OpenMP scheduling strategy was selected and why?
10. What happens when the number of threads exceeds the number of CPU cores?
11. Why is speedup not exactly equal to the number of threads?
12. What parts of the program are still serial?
13. What synchronisation is required and why?
14. How does the code guarantee a sorted final list?
15. How was correctness verified across implementations?
16. Why should I/O be excluded from the computation speedup measurement?
17. What are the key bottlenecks in each parallel implementation?
18. Which implementation, Pthreads or OpenMP, would you recommend on this task and why?

## 12.2 Alternative-assessment questions stated explicitly in the specification
Even for the regular assessment, these are excellent Q&A preparation:

- Is speedup exactly equal to the number of threads used in the POSIX Threads implementation? Why are they the same/different? Explain at least two reasons.
- Would you recommend OpenMP over POSIX Threads? Why/why not?

## 12.3 HD answer style
Rubric expectation:
- correct;
- concise;
- precise;
- no unnecessary details;
- uses parallel-computing terminology.

Codex should prepare answers that fit roughly **20–40 seconds**, then optional deeper follow-up details.

---

# 13. Likely Performance Explanations the Team Must Understand

These are implementation/performance concepts Codex should ensure the team can explain from their own measured results.

Speedup may be below the thread count because of:
- serial portions of the program;
- thread creation/join overhead;
- OpenMP runtime/scheduling overhead;
- workload imbalance;
- synchronisation overhead;
- memory/cache contention;
- OS scheduling noise;
- oversubscription when thread count exceeds useful hardware parallelism;
- too-small problem sizes where overhead dominates useful computation.

As `n` grows:
- useful computation increases;
- fixed parallel overhead can become less significant;
- speedup may improve until another bottleneck dominates.

As thread count grows:
- runtime may initially drop;
- speedup may flatten;
- it may eventually degrade due to overhead/contention/oversubscription.

Do not present these as explanations unless the measured data is consistent with them.

---

# 14. Repository / File Structure Recommended for Codex

```text
lab1/
├── task1.c
├── task2.c
├── task3.c
├── Makefile
├── README.md
├── AGENTS.md                 # optional: project-local Codex instructions
├── benchmark/
│   ├── run_benchmarks.sh
│   ├── raw_results.csv
│   ├── processed_results.csv
│   └── plot_results.py
├── tests/
│   ├── validate_outputs.sh
│   └── expected_small_cases.txt
├── results/
│   ├── graph1_serial_vs_pthread_n.png
│   ├── graph2_pthread_speedup_n.png
│   ├── graph3_serial_vs_pthread_threads.png
│   ├── graph4_pthread_speedup_threads.png
│   ├── graph5_serial_vs_openmp_n.png
│   ├── graph6_openmp_speedup_n.png
│   ├── graph7_pthread_vs_openmp_n.png
│   └── graph8_pthread_vs_openmp_threads.png
└── docs/
    ├── PRESENTATION_NOTES.md
    ├── Q_AND_A.md
    └── AI_DECLARATION_NOTES.md
```

Only submit the files required by the assessment, but keep supporting files in the working repository to make results reproducible.

---

# 15. Makefile / Build Requirements

Codex should produce a small Makefile with separate targets for all tasks and a clean target.

Expected logical targets:

```text
make task1
make task2
make task3
make all
make clean
```

Build commands should use consistent optimisation and warnings.

---

# 16. Codex Work Plan

Codex should implement the project in this order.

## Phase A — Requirements lock
- [ ] Read this guide fully.
- [ ] Preserve the exact requirement: primes are **strictly less than `n`**.
- [ ] Decide and document timing boundaries.
- [ ] Decide output behaviour.

## Phase B — Serial baseline
- [ ] Implement `task1.c`.
- [ ] Validate edge cases.
- [ ] Optimise unnecessary divisibility checks.
- [ ] Record sample output.
- [ ] Benchmark naive vs optimised serial if a naive baseline is retained for evidence.

## Phase C — POSIX Threads
- [ ] Implement `task2.c`.
- [ ] Choose partitioning scheme explicitly.
- [ ] Avoid hot-loop mutex bottlenecks.
- [ ] Guarantee sorted output.
- [ ] Cross-check exact output against serial.
- [ ] Test thread counts from 1 through and above core count.

## Phase D — OpenMP
- [ ] Implement `task3.c`.
- [ ] Use meaningful OpenMP work-sharing/scheduling features.
- [ ] Compare at least two schedule choices if practical.
- [ ] Guarantee sorted output.
- [ ] Cross-check against serial/Pthreads.

## Phase E — Benchmarking
- [ ] Detect/document hardware core count.
- [ ] Choose at least 30 `n` values.
- [ ] Use repeated runs per configuration.
- [ ] Export raw CSV.
- [ ] Compute aggregate timings and speedups.
- [ ] Generate all eight required graphs.

## Phase F — Presentation support
- [ ] Produce concise implementation summary.
- [ ] Select graph findings worth discussing.
- [ ] Write conclusion based only on measured evidence.
- [ ] Prepare appendix.
- [ ] Generate Q&A preparation.
- [ ] Ensure students can explain every major code block and performance claim without AI.

---

# 17. Definition of Done — HD-Oriented Gate

Codex should not declare the project complete unless every item below is satisfied.

## Correctness
- [ ] `task1.c`, `task2.c`, `task3.c` compile cleanly.
- [ ] All find exactly the same primes for test inputs.
- [ ] All outputs are ascending.
- [ ] All enforce `< n` rather than `<= n`.
- [ ] Task 1 supports at least `n = 10,000,000`.
- [ ] No races, deadlocks, crashes, memory errors, or invalid output.

## Optimisation
- [ ] Serial code eliminates obvious unnecessary checks.
- [ ] Pthreads version avoids avoidable shared-state bottlenecks.
- [ ] OpenMP version uses meaningful features/scheduling.
- [ ] Parallel speedup > 1 is demonstrated for sufficiently large workloads.
- [ ] Scaling behaviour is analysed against hardware core availability.

## Experimental evidence
- [ ] At least 30 distinct `n` values are tested.
- [ ] Thread counts include 1 through at least core count and some oversubscription tests.
- [ ] Multiple runs are collected per configuration.
- [ ] Raw benchmark results are retained.
- [ ] All 8 required graphs are produced.
- [ ] Graphs are complete, readable, and consistent with raw data.

## Presentation
- [ ] Task 1 approach + sample output + optimisations + evidence.
- [ ] Task 2 approach + partitioning + all required graphs.
- [ ] Task 3 approach + partitioning/OpenMP features + all required graphs.
- [ ] Conclusion covers major issues, limitations, and future work.
- [ ] Slides use parallel-computing terminology.
- [ ] Presentation is timed to the assessment window.

## Q&A
- [ ] Every student can explain the serial algorithm.
- [ ] Every student can explain the Pthreads partitioning.
- [ ] Every student can explain the OpenMP scheduling choice.
- [ ] Every student can explain the speedup formula.
- [ ] Every student can explain why speedup is not exactly equal to thread count.
- [ ] Every student can explain oversubscription.
- [ ] Every student can justify the observed Pthreads vs OpenMP results.
- [ ] Answers are concise, precise, and use parallel-computing terminology.

## Administrative
- [ ] Required names/IDs/emails added where appropriate.
- [ ] AI use declared.
- [ ] Prompt records prepared for PDF upload if required.
- [ ] Required files submitted before deadline.

---

# 18. Instructions to Codex While Editing

Use the following behavioural rules while working on this assessment:

1. **Never change the problem definition.** Find primes strictly less than `n`.
2. **Correctness before optimisation.** Any speedup result is meaningless if outputs differ.
3. **Do not optimise away explainability.** The team must defend the code orally without AI.
4. **Avoid hidden global state.** Make ownership of shared data explicit.
5. **Minimise serial bottlenecks in hot paths.** Especially per-prime locking or printing.
6. **Keep timing fair.** Do not compare different work between serial and parallel versions.
7. **Do not fabricate benchmark results.** Only graph measured data from the actual machine.
8. **Do not fabricate hardware details.** Query them on the benchmark machine.
9. **Do not claim near-linear speedup unless the measurements support it.** Explain deviations precisely.
10. **Do not omit required graphs.** All eight are mandatory for the presentation requirement.
11. **Retain raw benchmark data.** Every reported point should be traceable.
12. **Use parallel-computing terminology in documentation.** Prepare the team for rubric-aligned Q&A.
13. **Make every optimisation measurable.** Prefer evidence over claims.
14. **Flag assessment ambiguities.** Do not silently guess when specification sections conflict.
15. **Keep the final submitted code compact and readable.** Supporting benchmark/plot scripts may live separately.

---

# 19. Source-Derived Requirements vs Implementation Guidance

## Directly required / explicitly signalled by the assessment documents
- serial, Pthreads, and OpenMP C implementations;
- sorted primes strictly below user-supplied `n`;
- stdout/file output capability;
- Task 1 supports at least 10,000,000;
- runtime measurement;
- speedup calculation;
- parallel workload partitioning discussion;
- varying `n` and thread counts;
- at least 30 distinct `n` values for convincing evidence;
- testing thread counts up to at least CPU core count and considering values above core count;
- eight specified graphs;
- optimised code with unnecessary work/bottlenecks reduced;
- industry-level coding/documentation for HD;
- near-linear speedup as an HD aspiration/criterion;
- clear, concise, terminology-rich presentation;
- sample output and serial optimisation evidence;
- no external tools during Q&A;
- AI declaration/prompt records.

## HD-oriented implementation guidance added in this file
These are practical engineering recommendations inferred from the assessment goals, not literal mandated algorithms:
- skip even candidate numbers / divisors;
- use strided Pthreads partitioning or another measured load-balancing design;
- use thread-local buffers or per-candidate flags instead of per-prime locking;
- benchmark OpenMP scheduling policies;
- separate computation timing from large output I/O;
- repeat benchmark runs and aggregate them;
- emit CSV for reproducibility;
- use a Makefile and automated cross-implementation tests;
- record hardware/compiler metadata;
- calculate efficiency as an optional explanatory metric.

If the teaching team has given additional instructions in lectures/labs, those instructions should be incorporated before implementation is finalised.

---

# 20. Final Priority Order

If trade-offs arise, optimise in this order:

1. **Correctness**
2. **Ability of students to explain the implementation**
3. **Fair and reproducible measurement**
4. **Load balance / reduction of parallel bottlenecks**
5. **Measured speedup / scalability**
6. **Code quality and documentation**
7. **Presentation clarity and evidence selection**

The assessment rewards not merely a fast program, but a correct and defensible parallel-computing investigation supported by high-quality experimental evidence and concise technical explanation.
