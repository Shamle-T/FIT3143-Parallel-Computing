# FIT3143 Lab 3 - final submission

Savin Vindiv De Alwis | 35221631 | sdea0018@student.monash.edu
Shamle Thilaksiri | 35512075 | wthi0003@student.monash.edu

This is the definitive final folder. The package outside `submission_ready`
is the retained input, and contains older source/documents. Use this folder.

## Assessed documents and exact upload inventory

- `docs/Lab3_Presentation_Documentation.pdf`: final presentation, exported from
  the updated editable PowerPoint. Main slides 1-13 target a rehearsed 6-7
  minutes; subsequent slides are labelled appendices.
- `docs/Lab3_Presentation_Editable.pptx`: the same 49-slide final deck, with all 36
  original native charts/workbooks preserved and 18 new native CAAS charts.
  This editable format is optional if the PDF is submitted.
- The presentation now includes the technical documentation. Slides 35-49
  consolidate protocol details, complexity, timing definitions, experimental
  controls, complete result tables, correctness, provenance, shutdown and limits.
  Slides 14-49 are reference appendices outside the timed presentation.
- The single AI-use declaration is on slide 13. No prompt records or separate
  declaration document are included. The separate technical report is removed.
- `SUBMISSION_CHECKLIST.csv`: EVERY actual filename in this folder, with its
  required/supporting classification and specification-based reason.
- `SHA256SUMS.txt`: integrity hashes for all files except that manifest itself.

The specification (page 8) requires all C/C++ sources, sample base logs,
available CAAS scripts, all timing logs and available graphs, and presentation
slides/documentation.
Raw timing CSVs ARE measurement logs and must be retained. Derived analysis
CSVs/JSON, manifests and placement data support reproducibility. Standalone
screenshots are supporting; their relevant views are embedded in the deck.
The AI declaration appears only in the presentation, on slide 13.
No mandatory filename or ZIP pattern appears in the supplied specification.
Moodle's actual accepted file types/size limits were not inspected.

## Verified evidence

- Supplied local study: 540 scaling runs; 30 grid and 30 line sizes; three
  variants x three launches; 107,208 original raw timing records including
  supplementary runs. Recalculation verifies all 4,860 launch means and
  1,620 plotted records. Two TCP containers share one oversubscribed host.
- Final-source local validation: 25 integration runs and 1,773 timing records
  in `results/validation/`. `results/audit_validation.txt` records the checks.
  `results/audit_sanitizer.txt` records the expanded candidate-oracle sanitizer
  check. These are local WSL results, not CAAS results.
- CAAS job 199603: COMPLETED, exit 0:0,
  nodes student-caas-n10, student-caas-n11. All 119 simulations, 90 scaling
  cases, two-node placement and complete shutdown ACKs were validated.
  Five grid sizes and five line sizes each have all versions x three launches;
  24 correctness runs, four expiry settings and a 30-second realistic run
  supplement the scaling study. CAAS adds 10440 raw metrics.
- `results/caas/verification.txt`, `job_accounting.txt`, `environment.txt`,
  command/run manifests, consolidated base/station logs, raw CSV and nine
  CAAS graphs retain the evidence. Sixty launches emitted only the known
  Slurm warning for uneven per-node task counts; independent placement checks
  confirm balanced placement and at most eight ranks per node.
  `attempts/` records failed jobs 199444/199460 and incomplete job 199520.
  Diagnostic job 199513 distinguishes two successful launcher probes from
  two timed-out probes. These attempts/probes are excluded from the final
  90-run scaling comparison. Fresh job 199603 completed the entire study.
- `results/caas/evidence/` contains real browser screenshots of the local
  read-only evidence viewer displaying downloaded CAAS output. They are
  accurately captioned; no simulated terminal images or folder-only proof.

## Build and run

Linux with Open MPI and an OpenMP-capable C compiler:

    make
    mkdir -p results
    mpirun --oversubscribe --bind-to none -np 10 ./wsn 3 3 6 --variant 2 --rounds 3 --tick 10 --threshold 2 --expiry 30 --output results/demo

There must be exactly rows*cols+1 MPI processes. `make check` runs the
integration suite and candidate oracle. A root-only local container needs
Open MPI's explicit root opt-in environment variables. Final-source SHA-256:

    28cd1d3951e9b4fa523765811bff2f3da3c747916e08a66d0d4e665b562fdcdd

The final C source only changes the supplied simulator by capturing discovery
time before the alert gather, rejecting ticks outside the system time range,
and correcting a misleading flush/close comment.
The retained older local measured source additionally lacks identification
comments and the post-measurement aggregate summary. The supplied local study
is tied to that snapshot; CAAS measures the actual final source hash above.

## CAAS reproduction

Connect Monash VPN, then SSH with your authcate credentials. Upload this
folder and, FROM ITS ROOT on CAAS:

    mkdir -p results/caas
    sbatch scripts/caas_audit.job
    squeue -u "$USER"

The job requests two nodes, 16 rank slots, two cores per rank, 2 GB per node
and 10 minutes in defq. Live defq was checked to allow 20 minutes. It loads the
prescribed Open MPI 4.1.5/GCC 11.2 module, compiles with strict warnings and
uses the verified srun --mpi=none --exact route with explicit task/core
counts and core binding. It never runs the simulation
on the login node. There is one OpenMP thread per port, with two ports in the
CAAS study. `scripts/caas_run.sh` supports a manual run within a fitting
existing allocation; ROWS/COLS/PORTS must be supplied.

The job archives any previous top-level study files under attempts/ before
starting fresh journals, so reruns cannot silently mix command/run records.
The current job's stdout/stderr and launcher diagnostics are preserved.
Download the resulting raw files before
consolidation. `python3 scripts/analyze_caas.py /path/to/raw/results/caas`
validates a fresh raw download. `python3 scripts/analyze_caas.py --consolidated`
revalidates and regenerates graphs directly from the packaged evidence, with
no external files needed. The original per-run files remain
on CAAS and in the retained download outside this final folder.

## Analysis and local-cluster reproduction

`python3 scripts/analyze.py --check-only` verifies the supplied local study
using the standard library. Without `--check-only`, it regenerates all nine
local PNG graphs. Plotting was tested with matplotlib 3.6.3. Install
matplotlib in the Python environment used for graph regeneration if absent.
All original raw values remain unchanged. Query/reply plots are per-cycle
endpoint costs; remaining classes use per-operation normalization.

The original `scripts/cluster.py`, Dockerfile and `benchmark.py --cluster`
reproduce the separate two-container study. Docker must be running; it was
not needed for this audit's local WSL tests or CAAS runs. Both containers
share physical resources. They do not simulate an independent physical link.

## Limits and final student actions

Three launches provide descriptive comparisons, not statistical significance.
Inferred availability is not guaranteed capacity. The source uses native
struct bytes on homogeneous MPI hosts. The phased gather/barrier and single
base can limit scaling. Q&A performance and a 6-7 minute live presentation
cannot be certified by file checks. Both students must rehearse, understand
the code, upload the same files
before the actual Moodle/class deadline, and attend/present without AI.

## Presentation rehearsal

Only slides 1-13 form the timed presentation. The prepared script targets 6:50,
including pauses and handover. Savin presents slides 1-6 (3:30), then Shamily
presents slides 7-13 (3:20). The script is in the editable deck's speaker notes.
The separate practice copy, Presentation_Script.md, is outside the submission
folder. Actual delivery time depends on rehearsal and speaking pace.
