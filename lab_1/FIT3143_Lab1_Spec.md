# FIT3143 Lab #1 — Threads & OpenMP (Week 4)

**Goal:** Implement serial, POSIX Threads, and OpenMP prime-finders, benchmark them, and produce presentation slides/docs. Target: HD grade.

## Deliverables (submit via Moodle before deadline)
- `task1.c` — serial prime finder
- `task2.c` — POSIX Threads prime finder
- `task3.c` — OpenMP prime finder
- Task 4 presentation slides/doc (PDF, pptx, or docx) — ≤ 8 min presentation incl. Q&A (6–8 min for HD on timing criterion)
- AI declaration PDF (if Gen-AI used during prep) with all prompt records — required if AI was used
- All files should include team member names, student IDs, Monash emails

Late penalty: 5%/day. No team member present / missing submission → 0 marks.

---

## Task 1 — Serial Code (4% of methodology, HD-critical)
- Write serial C program: find all primes **strictly less than** integer `n` (user-provided).
- Output: **sorted ascending** list of primes.
  - `n < 100` → print to **stdout**
  - `n > 100` → print to a **text file**
- Must work for `n` up to at least **10,000,000**.
- Use trial division up to `sqrt(k)` only (not `k-1`) — compile with `-lm`, use `math.h` `sqrt()`.
- Must measure and print **execution time**.
- **HD requirements:**
  - Significant optimization to reduce/eliminate unnecessary computation.
  - Industry-level code style: good programming style, well-commented with industry-level documentation.
  - Code must be clean and bug-free (used as baseline for Tasks 2–3 comparison, and for demo/explanation in Task 4).

## Task 2 — POSIX Threads (8% of methodology, HD-critical)
- Parallel version (`task2.c`) of Task 1 using **pthreads**.
- Team must design own **parallel partitioning scheme** (workload distribution across threads) — implement and be ready to justify it.
- Must still output sorted ascending list of primes.
- Measure timing for varying `n` (recommend starting `n > 10,000,000`, increasing further) and varying thread counts (1 up to at least # of CPU cores) — compute speedup vs Task 1.
- Tabulate serial vs parallel times; compute speedup for each.
- **HD requirements:**
  - Significant optimization to reduce/eliminate unnecessary computation and bottlenecks, to improve parallel speedup.
  - Speedup strictly > 1 and **close to linear** (close to the number of computing cores available).
  - Industry-level style: good programming style, well-commented with industry-level documentation.
- Questions to be ready to answer:
  - Is the speedup reasonable? Is the run-time measurement correct?
  - How would speedup change with increasing/decreasing thread count and size of `n`? Why?
  - How do you distribute tasks to threads to ensure balanced workload distribution? Is it a good approach?
  - **Alt-assessment specific Q (answer in doc/recording if applicable):** Is the speedup exactly equal to the number of threads created/used? Why are they the same/different? Explain at least 2 reasons.

## Task 3 — OpenMP (8% of methodology, HD-critical)
- Parallel version (`task3.c`) of Task 1 using **OpenMP**.
- Design own OpenMP-based parallel partitioning scheme.
- Same measurement/tabulation requirements as Task 2, compare against Task 1 (and Task 2 for graphs).
- **HD requirements:**
  - Significant optimization with **different OpenMP features** to reduce/eliminate unnecessary computation and bottlenecks, to improve parallel speedup.
  - Speedup strictly > 1 and close to linear (close to the number of computing cores available).
  - Industry-level style: good programming style, well-commented with industry-level documentation.
- Questions to be ready to answer: same pattern as Task 2 (speedup reasonableness, effect of thread count/`n`, workload distribution), applied to OpenMP.
  - **Alt-assessment specific Q:** Would you recommend OpenMP over POSIX Thread? Why/why not? Explain your reasons.

---

## Task 4 — Presentation (80% of total grade — the dominant component)
Format: slides, docx, or PDF. Total ~7 min presentation + 1–2 min Q&A (aim within 8 min overall; HD band = 6–8 min).

### Required sections & content:
1. **Task 1 (Serial code)** — ~1 min
   - Approach to implementing serial code.
   - Sample output.
   - (HD) Improvements/optimizations made + statistically significant experimental results demonstrating them.

2. **Task 2 (POSIX Threads)** — ~3 min
   - Approach + parallel partitioning scheme / workload distribution explanation.
   - Required graphs:
     1. Runtime: serial vs POSIX-parallel, increasing `n`.
     2. Speedup of POSIX-parallel, increasing `n`.
     3. Runtime: serial vs POSIX-parallel, increasing # threads.
     4. Speedup of POSIX-parallel, increasing # threads.

3. **Task 3 (OpenMP)** — ~2 min
   - Approach + parallel partitioning scheme / workload distribution explanation.
   - Required graphs:
     5. Runtime: serial vs OpenMP-parallel, increasing `n`.
     6. Speedup of OpenMP-parallel, increasing `n`.
     7. Runtime: POSIX vs OpenMP, increasing `n`.
     8. Runtime: POSIX vs OpenMP, increasing # threads.

4. **Conclusion & Recommendations** — ~1 min
   - Summarize; discuss major issues, limitations, future work.

5. **Q&A** — 1–2 min
   - Team answers 1–2 questions per member about submitted/presented work, unaided (no AI tools).
   - Extra appendix slides allowed after Q&A section (clearly labeled, don't mix with required sections).

### Data collection notes (for HD-quality graphs):
- Test with **at least 30 different values of n** for statistical significance; increase `n` further if needed for clear trends given hardware.
- Test thread counts from **1 up to at least the number of available CPU cores**; also note/discuss behavior when threads > cores.
- Use **parallel computing terminology** throughout, both in slides and verbally (not general computing terms).

---

## Rubric Summary (HD criteria to hit)

| Component | Weight | HD bar |
|---|---|---|
| Task 1 code correctness/style | 4% | Sorted list correct; significant optimization; industry-style code + comments |
| Task 2 code correctness/style | 8% | Correct sorted output; speedup >1 and near-linear (close to # cores); optimized to remove bottlenecks; industry-style code + comments |
| Task 3 code correctness/style | 8% | Same as Task 2 but for OpenMP, using multiple OpenMP features |
| Presentation quality (clarity/org) | 10% | 6–8 min; exceptionally clear/organized; concise & precise; heavy use of correct parallel-computing terminology; strong conclusion covering issues/limitations/future work |
| Content: Task 1 | 6% | Clear approach + sample output + optimizations shown with statistically significant results |
| Content: Task 2 | 12% | Clear approach + partitioning scheme explained + ALL required graphs present, no flaws/omissions |
| Content: Task 3 | 12% | Clear approach + partitioning scheme explained + ALL required graphs present, no flaws/omissions |
| Q&A | 40% (largest single component) | Understand & correctly answer all questions unaided (no AI/external tools); concise, precise, accurate; correct parallel-computing terminology |

---

## AI Usage Rules
- Gen-AI allowed for **research/prep only** — must declare usage and upload all prompt records as a PDF.
- **AI tools NOT allowed** during: presentation, Q&A, oral/coding interviews, or (for alternate assessment) recording sessions.

## Alternative Assessment (only if special consideration approved)
- Task 4 assessed via recorded video (<9 min, only first 9 min at normal speed marked; speeding up playback can lose marks).
- No live Q&A — instead, must verbally answer in the recording:
  1. Is speedup exactly equal to # of threads used in Task 2? Why/why not (≥2 reasons)?
  2. Would you recommend OpenMP over POSIX Threads? Why/why not?
- Must show face + student ID at start of video.
- Still submit slides (PDF/pptx), source code, AI declarations via Moodle by approved extended deadline.
- 5%/day late penalty still applies; 0 mark if >7 days late.
