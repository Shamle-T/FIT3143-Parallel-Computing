# FIT3143 Lab 3 final audit — 22 October 2026

The definitive folder is `Lab3_35221631_35512075_Submission_With_PPTX/submission_ready`. The original package outside it is preserved. The prepared upload is `FIT3143_Lab3_35221631_35512075_Moodle.zip`.

The technical work and evidence are complete for the checks below. I do not know the eventual grade. A file audit cannot establish the students' live presentation or Q&A performance, which together carry 50% of the rubric.

## Criterion-by-criterion assessment

“Fully satisfied” means the implementation/materials meet the stated requirement within the inspected code and executed cases. It is not a guarantee that no other input can expose a defect or that the marker will award every HD descriptor.

Primary criteria: `docs/Lab #3 - Assessment Specification.pdf` (all nine pages), `docs/Lab #3 Rubric.pdf` (all five pages), and the supplied CAAS instructions, including the newer `caas_usage.txt`. Evidence paths below are relative to `submission_ready`.

| Criterion | Assessment | Verified basis / remaining limit |
|---|---|---|
| Task 1 (15%): MPI process per station and correct virtual topology | Fully satisfied | `src/wsn.c`: separate base/worker communicators, nonperiodic Cartesian grid, correct rank mapping and boundary neighbours. Isolated nodes, horizontal/vertical lines, squares and rectangles executed. |
| Task 1: one thread per port, random occupancy, shared array and synchronization | Fully satisfied | Persistent OpenMP team, per-port writer and private random state, atomic free-port count and barriers. `MPI_THREAD_FUNNELED`; only initial thread enters MPI. Random and deterministic workloads executed. |
| Task 1: congestion query/reply, immediate redirection and base escalation | Fully satisfied | Independent checks cover full, empty, mixed, ties and random scenarios across all three versions, including simultaneous alerts and different thresholds. |
| Task 1: extensive report information | Fully satisfied | Base logs contain source/rank/coordinates, cycle, discovery UTC, free ports, neighbour capacities, message counts and payload bytes. Discovery time now precedes the global gather. |
| Task 1: graceful termination without user interruption | Fully satisfied | STOP, port-team exit, completed requests, all station ACKs, log closure, communicator release and MPI_Finalize. Every final-study run exits normally; no Ctrl+C/kill is used to finish a successful simulation. |
| Task 1: clear diagrams and code/pseudocode | Fully satisfied | Main slides 2–5 and report pages 1–3 explain process/thread mapping, matching, congestion protocol and variants. |
| Task 1: blocking/nonblocking versions and experimentally justified optimisation | Fully satisfied — evidence provided | V0 blocking Sendrecv; V1/V2 preposted nonblocking neighbour exchanges. All are compared over the full local study and CAAS. CAAS median matched V0/V2 query ratios: grid 1.806, line 2.492. Nonblocking is not claimed to guarantee overlap or improve every metric. |
| Task 2 (15%): one base process, periodic report handling | Fully satisfied | World rank 0 only. A phased alert gather identifies the complete batch, then receives it using the selected method. This is a synchronized simulator, rather than a continuously asynchronous service. |
| Task 2: all nearest eligible nodes and recent-report expiry | Fully satisfied | Task 2's two-hop frontier is implemented: source 0 in a 3×3 gives all ties {2,4,6}. Independent enumeration checks every station in all 1..7 by 1..7 shapes, four cache states and all variants; separate expiry-boundary/eight-tie cases pass. Missing evidence is labelled inferred availability. |
| Task 2: required base logs and performance metrics | Fully satisfied | Consolidated incident logs, per-rank raw timing CSV, message/byte summaries and complete shutdown records retained. Actual placement and command manifests support them. |
| Task 2: base-directed graceful shutdown and explanation | Fully satisfied | All 119 final CAAS simulations have complete ACKs. Main slide 12 speaker notes and report pages 2/7 explain shutdown and matching. |
| Task 2: at least three implementations, including blocking/nonblocking | Fully satisfied | V0 rank-ordered blocking receive/send; V1 Probe/ANY_SOURCE with blocking replies; V2 preposted Irecv and batched Isend/Waitall. Every version executed in both studies. |
| Task 2: computation and communication optimisation evidence | Fully satisfied — evidence provided | V0 scans N nodes; V1/V2 inspect at most eight offsets. CAAS candidate benchmark has correct checksums at N=10,100,1000,10000. At N=10000 the isolated V0/V2 ratio is 1709.9; this is explicitly not whole-program speedup. CAAS alert round-trip ratios favour V2 by 1.093 (grid) / 1.302 (line). |
| Task 3 (20%): correct timing and communication-delay calculations | Fully satisfied | Same-rank MPI_Wtime intervals; operation/cycle normalization is explicit; no subtraction between host clocks. Buffered send completion, queueing, base barrier/tick waiting and collective payload accounting limits are documented. All original values remain available. |
| Task 3: at least 30 grid sizes | Fully satisfied | Supplied local cluster has 30 distinct grid node counts × 3 versions × 3 launches. Every plotted size is retained; no selected subset substitutes for the 30-size study. |
| Task 3: at least 30 line sizes | Fully satisfied | Supplied local cluster has 30 line instances × 3 versions × 3 launches. |
| Task 3: convincing answers to both scaling questions | Fully satisfied — analysis supplied | Report page 4 provides first/last-five quantitative comparisons for all nine classes, bounded station degree versus growing aggregate load, line degree/diameter and actual direct MPI message paths. It avoids inferring a universal latency law from oversubscribed data. |
| Task 3: clear graphs/tables, no measurement omissions | Fully satisfied | All 107208 supplied local raw records retained; 4860 scaling run means and 1620 plotted summaries independently reproduced. Nine local and nine CAAS PNGs, 54 editable native charts, underlying workbooks and derived tables checked. Three-launch medians and observed min/max are labelled. |
| Task 3: extensive CAAS/local cluster evidence | Fully satisfied | Local 540-run scaling study plus fresh CAAS 90-run scaling study and 29 additional simulations. The rubric permits a local cluster; supplied CAAS guidance permits a representative 4–5-grid CAAS subset. Five CAAS grids and five CAAS lines were executed. Local containers share one physical host; distinct allocated CAAS hostnames are verified without claiming unobserved physical hardware independence. |
| Presentation (10%): organization, diagrams, accurate terminology, limitations and future work | Fully satisfied for prepared materials | Updated existing deck: 13 main slides + 22 clearly labelled appendices. Preserved original diagrams/charts. Limitations, expiry, synchronization, blocking/nonblocking completion, portable serialization and future workload/confirmation work are included. Native PowerPoint opened/rendered all 35 slides; final PDF checked. |
| Presentation: actual 6–7 minute delivery and exceptional oral clarity | Partially satisfied | Speaker notes plan approximately 6:45; actual rehearsal, pace and live delivery are unverified. Both students must rehearse the 13 main slides and use appendices only when needed. |
| Q&A (40%): understand and answer every question correctly without tools | Unverified | Report page 8 supplies focused preparation questions. Actual individual understanding, concise parallel-computing terminology and unaided answers cannot be verified from submitted files. |
| Submission: sources, logs, scripts, graphs and presentation/documentation | Fully satisfied in prepared folder | Exact required/supporting inventory is in `SUBMISSION_CHECKLIST.csv` and the external `MOODLE_SUBMISSION_CHECKLIST.md`. Archive integrity and every content hash are checked. |
| AI declaration and available prompt records | Fully satisfied for available records; historical completeness unverified | Single current declaration on presentation slide 13; former Questions slide removed. `docs/AI_Prompt_Records.pdf` preserves all four supplied historical pages and adds current visible/retrieved requests. No separate declaration PDF and no fabricated conversations. Only the students can confirm whether other earlier AI histories exist. |
| Both students submit the same files before class, attend and demonstrate | Unverified | Human submission/attendance requirements. Actual Moodle accepted formats, size limit and session-specific deadline have not been inspected. |

The rubric's judgment of “extensive” optimisation and “strong and convincing” evidence remains with the marker. Three repeated launches provide descriptive comparisons; significance or universal superiority is not established. Those limits are visible in the final deliverables.

## Changes completed

- Preserved the simulator and its three MPI variants. Moved discovery-time capture to the congestion event, added a finite-but-out-of-range tick rejection before time conversion, and corrected a misleading flush/close comment. No broad refactor.
- Expanded the independent candidate oracle and invalid-input checks. Final source passed strict C11/OpenMP compilation, 25 local integration runs, the exhaustive small-grid oracle and ASan/UBSan checks. Consolidated 1773 final-source local timing rows without losing the raw records.
- Restored missing local analysis tooling, independently verified supplied timing aggregation and expected message counts, and regenerated nine clearly labelled local graphs. The old local measured source is retained; local measurements are not falsely attributed to byte-identical final code.
- Prepared and executed real CAAS jobs under the supplied module/SLURM guidance. Diagnosed launcher incompatibilities and used the demonstrated `srun --mpi=none --exact` route. The final successful job uses two allocated nodes and explicit cores/ranks.
- Collected authentic output, source/script hashes, scheduler accounting, command journals, placement, per-rank measurements and complete incident logs. Added reproducible consolidation/validation and nine CAAS graphs.
- Updated the existing deck, retained all 36 original native chart workbooks byte-for-byte, and added 18 native CAAS charts/workbooks. Added two authentic browser screenshots of downloaded CAAS records. The supplied screenshot shows only local folders and is labelled as source context; the original deck contained no local execution screenshots to remove.
- Rebuilt the eight-page technical report and eight-page available prompt-record PDF. The current declaration is on presentation slide 13; it has been moved out of the technical report. Removed obsolete Ready documents, superseded validation duplicates, executables and Python caches into `audit_work/obsolete_final_package/` outside the upload folder.
- Created the exact file classifications, integrity manifest and verified Moodle archive. No password, SSH helper, runtime dependencies, temporary viewer or build/render intermediates are included.

## Actual CAAS execution and provenance

| Item | Verified result |
|---|---|
| Final batch | Job **199603**, COMPLETED, ExitCode **0:0**, elapsed **00:06:54** |
| Compute hosts | `student-caas-n10`, `student-caas-n11` |
| Toolchain | Prescribed Open MPI 4.1.5 / GCC 11.2.0; strict build succeeded |
| Scaling | Grids 2×2, 2×3, 3×3, 3×4, 3×5; lines 4,6,9,12,15; V0/V1/V2 × 3 launches = **90** |
| Scaling protocol | 2 warmups + 12 measured cycles; 2 ports; mixed scenario; rotating variant order |
| Other simulations | **24** correctness runs, **4** expiry runs (0,0.1,1,30 seconds), **1** realistic random 3×3 run with three ten-second ticks |
| Total | **119 simulations**, plus candidate tests and benchmark = **121 recorded commands**, all exit 0 |
| Measurements | **10440 raw metric rows**, **1041 station placement records**, complete ACKs and checked report/message/byte fields |
| Known warnings | **60** launches emitted only the retained uneven-task Slurm per-node-cap warning. Actual placement is balanced and never exceeds eight ranks per node; no unexpected stderr is accepted. |
| Final source SHA-256 | `28cd1d3951e9b4fa523765811bff2f3da3c747916e08a66d0d4e665b562fdcdd` |

Primary evidence: `results/caas/job_accounting.txt`, `environment.txt`, `slurm-199603.out`, `slurm-199603.err`, `commands.jsonl`, `runs.jsonl`, `manifest.csv`, `host_placement.csv`, `all_timing_measurements.csv`, `all_base_station_logs.log`, `verification.txt` and `analysis/summary.json`. `scripts/analyze_caas.py --consolidated` regenerates/validates directly from packaged evidence; plotting requires matplotlib. This reproduction mode was tested.

The two evidence PNGs are genuine browser captures of a local read-only viewer displaying unmodified downloaded CAAS excerpts, accurately captioned as such. They are not claimed to be native remote-terminal screenshots.

Preparation failures remain visible: 199444 failed on forced-PMIx srun; 199460 failed on an MPI daemon callback; diagnostic 199513 completed but contains two successful modes and two timed-out probes; preliminary 199520 hit its five-minute reservation. Its top-level exit field alone was misleading, so both accounting state and exit code were checked. None of these partial/probe timings enters the final 90-run comparisons. Small failure records are retained under `results/caas/attempts/`; complete archived downloads remain outside the upload folder in `audit_work/`.

No required CAAS experiment remains blocked. No failed attempt is represented as a successful run.

## Exact submission and remaining human actions

Use `MOODLE_SUBMISSION_CHECKLIST.md` for the complete filename-level list, split into **Required**, **Supporting/conditional**, and **Not necessary**. The same inventory is machine-readable inside Ready as `SUBMISSION_CHECKLIST.csv`.

- **Required:** all four `.c` files (including tests/measured snapshot), the three final PDFs, available CAAS job/run/driver scripts, raw timing CSVs (including candidate/diagnostic measurements), base logs and all 18 generated graph PNGs. Raw CSV measurements are time logs under specification page 8.
- **Supporting/conditional:** editable PPTX when submitting the presentation PDF, derived CSV/JSON, command/placement/environment provenance, correctness output, screenshots, build/reproduction tools and integrity/navigation files. These remain in the prepared ZIP to substantiate the work. The exact checklist distinguishes individual files.
- **Not necessary:** original package/drafts, separate AI declaration PDF, executables/caches, temporary rendering/SSH/debug artifacts, downloaded TAR, official input documents and an alternative-assessment recording unless it actually applies.

The ZIP contains only the clean `submission_ready/` folder. Upload it if Moodle permits ZIP; otherwise upload the listed required files and applicable supporting files through the actual Moodle interface. No Moodle submission was performed.

Only these actions require the students: confirm any missing earlier prompt histories and add them if they exist; rehearse both students to 6–7 minutes; prepare to explain all code/results without AI; check the actual Moodle upload restrictions/deadline, upload identical final files, and attend/present. There is no remaining authentication or CAAS access action.


23 October revision: replaced the Questions page with a single AI-use declaration slide (slide 13), removed the current declaration from the technical report, updated declaration references, and retained the required authentic prompt-record PDF. CAAS screenshots remain in appendix slides 33–34.

Final integrity check after the 23 October revision: **115 files (43 required, 72 supporting/conditional)**; archive **8,346,904 bytes**. All 114 manifest hashes, ZIP CRCs and every archived file match the final folder. Archive SHA-256: `e012015f995eeb6d8746efc5dcd9dac84bc67856a66e8a77ae5330524128384c`.
