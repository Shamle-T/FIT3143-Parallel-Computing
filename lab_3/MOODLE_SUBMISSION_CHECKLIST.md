# Exact Moodle submission checklist

Savin Vindiv De Alwis 35221631 / Shamle Thilaksiri 35512075

The supplied specification sets no mandatory filenames or ZIP pattern. If Moodle permits ZIP, upload `FIT3143_Lab3_35221631_35512075_Moodle.zip`; it contains this full prepared folder. Otherwise upload the required files below and retain their directory structure where the upload interface permits. Actual Moodle file-type/size restrictions were not inspected.

## Required

- `docs/AI_Prompt_Records.pdf`
- `docs/Lab3_Presentation_Documentation.pdf`
- `docs/Lab3_Technical_Report.pdf`
- `results/analysis/ack.png`
- `results/analysis/alert_roundtrip.png`
- `results/analysis/control.png`
- `results/analysis/flags.png`
- `results/analysis/query.png`
- `results/analysis/redirect.png`
- `results/analysis/reply.png`
- `results/analysis/report.png`
- `results/analysis/sync.png`
- `results/caas/all_base_station_logs.log`
- `results/caas/all_timing_measurements.csv`
- `results/caas/analysis/ack.png`
- `results/caas/analysis/alert_roundtrip.png`
- `results/caas/analysis/control.png`
- `results/caas/analysis/flags.png`
- `results/caas/analysis/query.png`
- `results/caas/analysis/redirect.png`
- `results/caas/analysis/reply.png`
- `results/caas/analysis/report.png`
- `results/caas/analysis/sync.png`
- `results/caas/candidate_benchmark.csv`
- `results/caas/commands.jsonl`
- `results/caas/diagnostic/srun_none.base0.log`
- `results/caas/diagnostic/srun_none.metrics.csv`
- `results/caas/diagnostic/srun_pmi2.base0.log`
- `results/caas/diagnostic/srun_pmi2.metrics.csv`
- `results/candidate_benchmark.csv`
- `results/cluster/all_base_station_logs.log`
- `results/cluster/all_timing_measurements.csv`
- `results/cluster/timed_source_snapshot.c`
- `results/sample_realistic.base0.log`
- `results/validation/all_base_station_logs.log`
- `results/validation/all_timing_measurements.csv`
- `scripts/caas_audit.job`
- `scripts/caas_diagnostic.job`
- `scripts/caas_experiments.py`
- `scripts/caas_run.sh`
- `src/wsn.c`
- `tests/benchmark_candidates.c`
- `tests/test_candidates.c`

## Supporting/conditional

- `docs/AI_RECORD.md`
- `docs/Lab3_Presentation_Editable.pptx`
- `Makefile`
- `README.md`
- `results/analysis/all_message_delays.csv`
- `results/analysis/audit.txt`
- `results/analysis/comparison.json`
- `results/analysis/independent_verification.txt`
- `results/analysis/recommendation.json`
- `results/analysis/run_means.csv`
- `results/analysis/scaling_trends.json`
- `results/audit_sanitizer.txt`
- `results/audit_validation.txt`
- `results/caas/all_station_logs.log`
- `results/caas/analysis/all_message_delays.csv`
- `results/caas/analysis/run_means.csv`
- `results/caas/analysis/summary.json`
- `results/caas/analysis_run.txt`
- `results/caas/attempts/job199444_accounting.txt`
- `results/caas/attempts/job199444_environment.txt`
- `results/caas/attempts/job199460_accounting.txt`
- `results/caas/attempts/job199460_commands.jsonl`
- `results/caas/attempts/job199460_environment.txt`
- `results/caas/attempts/job199520_accounting.txt`
- `results/caas/attempts/job199520_environment.txt`
- `results/caas/attempts/job199520_slurm-199520.err`
- `results/caas/attempts/job199520_slurm-199520.out`
- `results/caas/attempts/README.txt`
- `results/caas/attempts/slurm-199444.err`
- `results/caas/attempts/slurm-199444.out`
- `results/caas/attempts/slurm-199460.err`
- `results/caas/attempts/slurm-199460.out`
- `results/caas/consolidated_reproduction.txt`
- `results/caas/correctness.txt`
- `results/caas/diagnostic/mpirun_tcp.stderr`
- `results/caas/diagnostic/mpirun_tcp.stdout`
- `results/caas/diagnostic/srun_none.node0.log`
- `results/caas/diagnostic/srun_none.stderr`
- `results/caas/diagnostic/srun_none.stdout`
- `results/caas/diagnostic/srun_pmi2.node0.log`
- `results/caas/diagnostic/srun_pmi2.stderr`
- `results/caas/diagnostic/srun_pmi2.stdout`
- `results/caas/diagnostic/srun_pmix.stderr`
- `results/caas/diagnostic/srun_pmix.stdout`
- `results/caas/diagnostic-199513.err`
- `results/caas/diagnostic-199513.out`
- `results/caas/diagnostic_accounting.txt`
- `results/caas/environment.txt`
- `results/caas/evidence/caas_execution.png`
- `results/caas/evidence/caas_ties.png`
- `results/caas/host_placement.csv`
- `results/caas/job_accounting.txt`
- `results/caas/job_history.txt`
- `results/caas/manifest.csv`
- `results/caas/runs.jsonl`
- `results/caas/slurm-199603.err`
- `results/caas/slurm-199603.out`
- `results/caas/verification.txt`
- `results/cluster/environment.json`
- `results/cluster/host_placement.csv`
- `results/cluster/manifest.csv`
- `results/evidence/provided_folder_structure.png`
- `results/final_artifact_verification.json`
- `results/validation/provenance.json`
- `scripts/analyze.py`
- `scripts/analyze_caas.py`
- `scripts/benchmark.py`
- `scripts/cluster.py`
- `scripts/Dockerfile`
- `scripts/validate.py`
- `SHA256SUMS.txt`
- `SUBMISSION_CHECKLIST.csv`

## Not necessary to upload

- The original package outside `submission_ready`, old presentation/report versions or the old `AI_Declaration_and_Prompt_Records.pdf` (its available prompt content is retained in the final prompt PDF).
- A separate AI declaration PDF: the final technical report and presentation already declare AI use.
- Executables, caches, per-run duplicates already consolidated without losing values, intermediate renderings, SSH helper/dependencies, downloaded raw TAR or build/debug scripts in `audit_work/`.
- The official specification/rubric, teaching materials, CAAS instructions or demo ZIP: reference inputs, not team deliverables.
- A recording unless a special-consideration alternative assessment actually applies.

Raw timing CSVs, C sources, available CAAS scripts and generated graph PNGs are required. Derived CSV/JSON tables, manifests, screenshots and validation records are supporting; they are retained in the ZIP to make the evidence reviewable. A PPTX is an optional editable alternative when the final presentation PDF is uploaded.
