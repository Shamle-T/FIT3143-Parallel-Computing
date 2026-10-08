# Applied 2: HD review and executable action plan

**Revision status, 8 October 2026:** the subsequent student request narrowed this work to Task 1/2 content and a professionally formatted, self-contained Task 3 presentation. Delivery and Q&A criteria are excluded. See the revision closure record at the end; the audit below describes the original baseline.

Reviewed 8 October 2026 against the supplied assessment specification and HD rubric. Baseline: main at commit 2cdb56e6d6f592f209727d423d4e9032dd191616. This document records findings and future work; it does not implement the solution changes.

## Verdict and scope

Both tasks have a sound foundation. All required topics are present, the slides are readable, and the checked rotation example, launch arithmetic and chart data are correct. The principal HD risks are insufficient use of the existing CUDA examples during the assessed talk, compressed Task 2 analysis and comparisons, and an incomplete explanation of the conditions under which this particular rotation pipeline would benefit from GPUDirect Storage (GDS). There are also definite local citation defects.

An HD result cannot be certified from files. The rubric assigns 40% to independent Q&A and 20% to presentation quality, including actual delivery between six and seven minutes. I do not know whether either presenter currently meets those performance criteria. The review covers every written criterion below; it cannot establish every possible marker question or future delivery outcome.

Evidence labels used here:

- **Defect:** directly observed inconsistency or broken dependency.
- **HD risk:** a specific weakness against an HD descriptor, not a prediction of a deduction.
- **Verification gate:** requires subsequent measurement, presenter evidence or submission confirmation.

## Material reviewed and checks performed

The review covered all root context PDFs, both workshop decks and the root C examples, with detailed examination of Weeks 7–9. It covered the assessment specification (seven pages), rubric (six pages), both task decks, all 19 combined-deck slides and their speaker notes, both study guides and AI-record PDFs, both CUDA example files, the script in Markdown/PDF, chart JSON and embedded chart workbooks, and README.

All 19 combined slides were rendered and visually inspected. Standalone and combined slide text agrees apart from slide numbering. PDF text was checked against its Markdown counterpart; representative study-guide/script pages were rendered for readability. The external numbered references were checked against primary documentation, original papers and official institutional sources. The missing notebook could not be checked because it is absent from this checkout.

Verified examples:

- A 1024 × 1024 image with 16 × 16 blocks needs a 64 × 64 grid: 4,096 blocks and 1,048,576 logical threads. These threads are scheduled on finite SM resources; they do not all execute simultaneously.
- The illustrated 5 × 5, 90-degree counterclockwise inverse mapping sends output (0,1) to input (3,0), consistent with the code's image-coordinate convention.
- Chart JSON, cached chart values and embedded workbooks agree: 485/950 TWh and growth fractions 0.17/0.50. The displayed increase is approximately 95.88%. Estimated and projected values are distinguished correctly.
- Both CUDA files were inspected. No substantive algorithmic error was found within their stated assumptions. No CUDA compiler/GPU run was available, so compilation and runtime correctness have not been demonstrated.

There is no requirement to produce measured speedup or a complete executable prototype. The specification encourages a prototype or a relevant library reference. The rubric does require crafted sample code and an explanation of performance effects. Do not invent benchmark results to fill that requirement.

## Complete rubric and specification coverage

Page numbers below refer to the supplied PDFs, not an online version. Slide numbers refer to the current combined deck.

| Criterion | Current evidence | Assessment and required action |
|---|---|---|
| Task 1(a), 6%: complete, correct RAM/GPU transfers and readable flow diagrams; precise parallel terminology (rubric p1; specification p3) | Slide 1 shows storage, host RAM, GPU memory, H2D/D2H and CPU/device roles. Guide explains pinned memory and copies. | Correct basic path; main diagram/explanation gives little transfer mechanism detail. F03: distinguish host DDR, device GDDR/HBM, PCIe transfer, allocation, copy engine/DMA and completion. |
| Task 1(b), 7%: CUDA host/device roles, launch configuration, grids/blocks/threads and readable diagrams/code (rubric p2; specification p3) | Slides 2–4 show gather rotation, launch, warp/SM scheduling. Full kernels and host snippets exist in the companion files. | Core model is sound. F04: use several short code examples in the timed presentation, tie them to the diagrams and explain SIMT, bounds checks, memory access and synchronization precisely. |
| Task 1(b): expected modern-GPU speedup and how distinct CUDA features affect it | Slide 5 gives total-time components and qualitative performance factors; slide 16 and performance .cu show block-shape and asynchronous-copy choices. | F05: add a course-grounded performance bound and crossover explanation; explicitly explain the effects of the code variants. Avoid a claimed universal speedup or treating CUDA cores as independent ideal CPU processors. |
| Task 1(c), 7%: researched GDS operation, complete diagram, rotation applicability and academic references (rubric p3; specification p3) | Slide 6 compares staged/direct paths, notes CPU control and conditional usefulness; guide has compatibility and decoding caveats. NVIDIA sources are included. | F06: make API/control path, storage-side DMA, supported direct path versus fallback, and the decision for this CPU-decoded example explicit. F01–F02 repair/normalize references. |
| Task 2(a): environmental/energy consequences (specification p4) | Slide 8 has correctly scoped charts; guide explains energy, carbon, cooling/water and embodied versus operational impacts. | Strongest topic. F07–F08: connect macro trends to a specific HPC job decision, include a research comparison and distinguish energy from time and emissions. Preserve source caveats. |
| Task 2(b): governance, security and fairness | Slide 9 mentions provenance, protection and group fairness; guide compares gradient reconstruction with fairness evaluation. | Present but compressed in the talk. F07–F09: explain one parallel-training threat/control/residual risk and one separate fairness harm/evaluation, with research comparison and a useful diagram. |
| Task 2(c): socioeconomic/accessibility challenges | Slide 9 and guide compare historical compute concentration with NAIRR reach. | Present but not deep. F07–F08: strengthen research breadth, identify remaining barriers and policy trade-offs; distinguish research evidence from programme statistics. |
| Task 2(d): responsible HPC frameworks | Slide 10 combines Green AI, carbon-aware scheduling, OECD/NIST and transparent allocation. Guide describes a proposed process. | Sound proposal, lightly operationalized in the talk. F08–F09: show the distinct functions of the sources and one accountable decision process with an exception/trade-off. |
| Task 2, 20%: all four topics, no significant gaps, up-to-date research, extensive credible research articles **for each topic**, informative comparisons, academic references, extensive useful visuals and parallel terminology (rubric pp3–4) | Four topics, current official statistics and original research exist. Most explicit comparisons are study-guide additions. Main visuals concentrate on training and energy; topics b/c/d are mainly text. | F02, F07–F09: make comparisons visible/audible, balance evidence across topics, normalize citations, add explanatory visuals and connect ethics to communication, synchronization, resource allocation and scaling. No fixed paper/diagram count is specified. |
| Presentation quality, 20%: six to seven minutes, exceptional clarity/organization, concise selection, summaries, issues/limitations/future work, parallel terminology (rubric pp4–5) | Six Task 1 and four Task 2 main slides, shared Q&A, clearly labeled appendices. Limitations/future work are already in the scripts/guides. | F10–F11: improve visible section synthesis and verify actual delivery. No general slide-legibility defect was observed. |
| Q&A, 40%: correct, full, concise independent answers using parallel terminology, without external tools (rubric p5; specification pp1–2) | Useful question banks and Task 1 reflection answers exist. | F12 is essential: both presenters must rehearse and handle follow-ups without tools. Written answers cannot establish this criterion. |
| Presentation/submission rules (specification pp1–2, pp5–6) | Combined main talk followed by Q&A and appendices is correctly ordered. Existing PDFs contain AI records. | F13: extend AI records for this review and later changes. F14: verify both students' contributions, actual Moodle file submission and attendance; fill identifying information. Repo links alone are insufficient. |

The specification gives approximate Task 1/Task 2 allocations of three/four minutes, but the existing recorded user preference is six minutes total, three each. Retain that preference initially and verify six-to-seven-minute delivery; do not reinterpret the indicative allocations as a compulsory four-minute Task 2 talk.

## Findings and exact remedies

### F01 — Broken local references and unverified notebook claims [Defect]

**Evidence:** Task1_Speaker_Notes_and_Study_Guide.md, performance section around line 129 and references around line 145; corresponding Task 1 PDF pp11–12 and deck references/notes.

Reference [1] points to a CUDA Colab notebook under ../../W9/Week 9 Applied/Week 9 Applied/. That notebook is absent. Reference [4] points to ../Applied #2 - Assessment Specification.pdf, whereas the actual PDF is under ../assessment/. The guide asserts particular timing/allocation behavior in the unavailable notebook. Those notebook-specific assertions could not be independently verified.

**Remedy:** correct the specification path. Retrieve and inspect the exact notebook if it can be supplied; otherwise remove unsupported claims about its implementation/timing and cite the available Topic 8 material or the actual example code for the relevant explanation. User-supplied reflection questions may remain identified as such. Do not silently replace the missing notebook with a different notebook and retain the old claims. Update all affected notes, reference slides and generated PDFs. Preserve historical AI prompts verbatim.

**Closure:** every local source resolves; every retained implementation-specific assertion is traceable to inspected code. Search every submitted artifact for the obsolete paths and notebook timing claims.

### F02 — Academic reference consistency and version clarity [HD risk]

**Evidence:** numbered references across both guides/decks. Traceable URLs and author/year information generally exist, but bibliographic detail and formatting vary. The water paper is labeled a 2023 preprint while its unversioned arXiv page now includes subsequent revisions; the fairness survey is a published journal article.

**Remedy:** use one recognizable academic style, preferably IEEE to preserve numbering. Include correct author/title/year, journal or proceedings information where applicable, stable URL/DOI and actual access date for changing documentation. State whether a claim uses an original version or a later revision. Prefer the published fairness-survey record where appropriate. Align in-slide citation numbers, notes, guides and appendix entries; do not rewrite the historical access date as though a new check happened on 6 October.

**Closure:** one claim/source matrix checks support, version, date, scope and numbering. This is a precision/consistency improvement, not a finding that the online references are fabricated.

### F03 — Transfer explanation leaves mechanisms mostly in the guide [HD risk]

**Evidence:** slide 1 versus the more complete Task 1 study explanation.

**Remedy:** label host RAM (DDR example) and device memory (GDDR/HBM examples) separately from the PCIe interconnect. Show host/device allocation and the conventional H2D/kernel/D2H sequence. Add compact copy-engine/DMA and pinned/pageable staging explanation, with CPU orchestration and the required completion before saving or reusing buffers. Explain that device-memory bandwidth and host-device link bandwidth describe different paths. Keep decoding on the host in this example.

**Closure:** presenter can trace one image through the complete path and explain why an otherwise fast kernel can still lose end-to-end. Do not imply pinned memory eliminates the transfer or that ordinary cudaMemcpy is GDS.

### F04 — Multiple crafted CUDA examples are not substantively used in the main talk [HD risk, high priority]

**Evidence:** main slide 3 contains a launch expression. The actual host-copy, brightness/rotation and performance examples are largely in appendix slides 12–13 and 16 or companion .cu files. The rubric explicitly distinguishes HD by crafted examples of different CUDA features and their performance effects.

**Remedy:** integrate short, readable excerpts into the six Task 1 main slides: host copy/launch, per-thread index plus bounds guard, inverse-gather source read/output write, and a performance variant. Contrast brightness's independent contiguous accesses with rotation's angle-dependent gathered reads. Connect logical threads to warps, SM scheduling and resource-limited residency; distinguish the course's SIMD description from CUDA's SIMT execution and branch divergence. Explain why output gathering avoids competing writes and does not require an inter-thread barrier for this kernel.

Keep the complete examples in the appendix. Align rotate versus rotate_rgb names, spell out teaching aliases H2D/D2H, and show grid recomputation when block shape changes. The performance .cu already recomputes the grid correctly; do not introduce a false code defect.

**Closure:** the timed script actually explains multiple code features and their consequences; mere inclusion of .cu attachments is not the acceptance test. No complete application is required.

### F05 — Speedup explanation needs a stronger model and feature-to-effect links [HD risk, high priority]

**Evidence:** main slide 5 and appendix slide 16; the existing end-to-end accounting is correct but qualitative.

**Remedy:** retain T_GPU,total = setup + H2D + kernel + D2H for the illustrated sequential path. Use Week 7 Amdahl reasoning to explain non-accelerated work as a bound; state that idealized parallelism is a model, not a measured prediction for a GPU core count. Explain image-size crossover, amortizing setup/transfers over batches or resident images, coalesced writes versus irregular reads, latency hiding, register-limited occupancy and finite memory bandwidth. Contrast 16 × 16 and 32 × 8 as hypotheses to test, not a claim that one always wins.

Explain that same-stream asynchronous copies/kernel remain ordered; copy/compute overlap across independent work needs appropriate hardware, pinned host memory, streams and separate live buffers. If overlap is proposed, use a different critical-path timing model rather than summing overlapped intervals. Define timing boundaries, synchronization and CPU baseline workload before reporting any future measurements.

**Closure:** presenter can explain when the kernel accelerates but total execution does not, and why occupancy/GPU count alone does not determine speedup. Any numerical example is labeled theoretical unless measured.

### F06 — GDS needs a concrete operation and adoption decision [HD risk, high priority]

**Evidence:** slide 6 correctly bypasses the host staging path conceptually, but omits cuFile and the storage DMA/control detail; the conditional conclusion remains general.

**Remedy:** redraw conventional and direct paths with storage-side DMA (NVMe/NIC as appropriate), PCIe fabric and GPU memory. Show CPU-issued cuFile I/O as a separate control path. State that support depends on the actual storage/filesystem/software/topology configuration and that compatibility fallback can use host staging. Avoid an obsolete universal driver/module requirement. Explain that GDS changes I/O, not the rotation arithmetic.

Conclude for this submitted pipeline: a single small image already decoded into host RAM has little reason to gain from GDS; large storage-bound batches with an appropriate GPU-compatible data/decode path may gain, subject to direct-path verification and end-to-end measurement. GDS does not itself decode a compressed image or remove CPU preprocessing. This is a reasoned conditional decision, not a measured claim.

**Closure:** diagram and script distinguish data plane/control plane, direct transfer/fallback, compressed storage/decoded pixels, and kernel speedup/I/O improvement. Sources support each claim.

### F07 — Task 2's research breadth and comparisons are uneven in assessed delivery [HD risk, high priority]

**Evidence:** main slide 9 covers topics b and c in 35 seconds. The guide has explicit comparisons absent from most spoken coverage. Topic c currently has one research preprint (Ahmed/Wahed) and an NSF programme update; a programme update is credible evidence but not an additional research article. Environmental/framework coverage has several papers, while breadth for access is less convincing under the HD wording.

**Remedy:** prepare a four-row evidence matrix: topic, research findings, contrasting/complementary source, methodological scope, implication and limitation. Reuse the guide's existing comparisons: gradient reconstruction versus fairness evaluation; historical concentration versus current programme reach; measurement versus scheduling versus governance. Add verified primary research on access/reproducibility or compute allocation if needed to substantiate the access mechanism and evaluate a response. Assess evidence quality rather than adding citations solely to increase a count. The rubric gives no minimum number of articles.

Expose at least one informative comparison for every topic in slides and spoken delivery. Institutional statistics/frameworks can complement research, but should not be presented as equivalent to research articles. Keep the existing estimate/projection, overlapping growth categories and US programme-scope caveats.

**Closure:** a reviewer can identify the research comparison, reasoning and limitation for each of the four topics from the assessed presentation, with fuller evidence in the guide.

### F08 — Ethical recommendations need an explicit HPC mechanism, choice and residual trade-off [HD risk]

**Evidence:** current policy statements are sensible; stronger reasoning is mostly in the guide. Framework names alone do not show how an operator makes a decision.

**Remedy:** use one small hypothetical cluster scenario across the four topics, clearly labeled as an illustration. Specify useful outcome and GPU-count/scaling choice; measure whole-job energy rather than elapsed time alone. For security, identify who can access gradients/checkpoints and a control plus its remaining risk. For fairness, identify a relevant affected group/harm and an evaluation appropriate to it; access control does not establish fairness. For access, explain cost, allocation, expertise or reproducibility barriers and a transparent allocation/appeal rule with a trade-off. For responsibility, map owner, evidence, decision and review to NIST's Govern/Map/Measure/Manage functions, OECD values and Green AI efficiency reporting. Add a deadline/carbon/water exception to the scheduling decision.

These are proposed controls, not research-proven guarantees, compulsory NIST certification steps or claims that every all-reduce leaks data. Explain diminishing scaling efficiency through communication/synchronization without making up job results.

The specification's introduction also names dual-use risks. The current treatment emphasizes privacy and fairness, with no explicit dual-use discussion. Include a concise misuse/authorized-use consideration in the governance scenario and explain a proportionate review control and its limitation. This strengthens completeness within topic (b); it is not a separately mandated fifth topic. Likewise, balance cluster benefits against resource costs rather than implying all scaling is harmful.

**Closure:** each topic has a mechanism → affected stakeholder → reasoned mitigation → limitation, supported by the appropriate source and a parallel-computing connection.

### F09 — Explanatory visuals are concentrated in two Task 2 slides [HD risk]

**Evidence:** training diagram and energy charts are useful; slides 9–10 rely mainly on text columns/table. The HD descriptor asks for extensive use of useful diagrams/figures/pictures, without prescribing a count.

**Remedy:** add an editable gradient/checkpoint exposure-and-control flow and a compact accountable allocation/scheduling diagram. Use them to explain b/c/d, not as decoration. Add a small visible comparison structure so sources and consequences can be compared. Retain readable charts and avoid misleading comparisons between incompatible quantities.

**Closure:** visuals carry part of the reasoning for the less developed topics, remain readable at presentation size and are actually explained in the script.

### F10 — Section conclusions can be easier to assess [HD risk, moderate]

**Evidence:** limitations and future work already appear in the script, especially the Task 2 closing. They are not wholly missing.

**Remedy:** add a compact Task 1 takeaway to the GDS/performance ending and a Task 2 decision/takeaway to the framework ending. Each should state the main conclusion, its major limitation and the next measurement or investigation. Preserve the existing logical transition between tasks and shared Q&A. Appendices remain after Q&A and clearly labeled.

**Closure:** sections summarize their argument without an extra dense conclusion slide or repeated background.

### F11 — Six-minute label is not measured delivery [Verification gate]

**Evidence:** approximately 385 Task 1 and 367 Task 2 spoken words, 752 total. This is about five minutes at 150 words/minute or six at 125 before pauses; neither calculation establishes actual duration. The script acknowledges rehearsal is required.

**Remedy:** after content edits, complete at least two timed full runs with both presenters, including transitions and slide explanations, excluding Q&A. Aim for a comfortable point within six to seven minutes, initially keeping roughly three minutes each. Adjust selection and pace using actual runs; do not pad with empty text or assume the nominal timers are evidence.

**Closure:** record measured run durations, task splits and corrections; verify readable code can be explained at that pace. Exact optimal split remains a rehearsal decision.

### F12 — Independent Q&A remains the largest unverified HD condition [Verification gate, high priority]

**Remedy:** expand and rehearse a concise question bank with follow-ups. Both presenters should understand the shared work and be able to explain the actual submitted choices. Cover:

- Host allocation, pinned/pageable transfers, completion and buffer lifetimes.
- Grid rounding for non-multiple dimensions; for 1000 × 750 with 16 × 16 blocks: 63 × 47 blocks, 758,016 threads and 8,016 threads rejected by bounds checks.
- Inverse versus forward mapping, coordinate sign, center, rounding, fixed canvas, black borders, RGB layout and interpolation trade-offs.
- Logical threads/warps/SMs, SIMT/divergence, residency, coalescing, memory/transfer bounds, end-to-end timing and asynchronous ordering/overlap.
- GDS control versus data path, fallback, decoding and the actual adoption decision.
- Data/tensor/pipeline parallelism, gradient collectives and why more GPUs need not reduce time/energy proportionally.
- Chart units, date, estimate/projection and why 50% growth is not a 50% sector share.
- Power versus energy versus carbon; operational versus embodied impacts; water trade-offs.
- Exposed-gradient threat assumptions, security versus fairness, research scope and access-measurement limitations.
- Distinct Green AI, carbon-aware scheduling, OECD and NIST roles; accountable decisions and exceptions.

**Closure:** both presenters answer mixed follow-ups correctly and concisely without AI, online tools or consulting the answer bank. Record weak answers and repeat only those that need work. This practice reduces risk; it cannot guarantee every possible interview question.

### F13 — AI disclosure must include this review and subsequent revisions [Submission gate]

**Evidence:** existing declarations record 6 October preparation and explicitly require appending later AI-assisted work. They cannot already include this 8 October review.

**Remedy:** append the actual review requests, this action-plan request and all subsequent relevant preparation/edit prompts to the existing AI records or a clearly cross-referenced continuation PDF. State the tool and dates accurately. Preserve earlier prompts and declarations; distinguish AI assistance from student validation. Do not claim an unseen historical conversation was audited for completeness. Submit the actual PDFs as required by the specification.

**Closure:** a complete prompt record is exported from available conversation evidence and accompanies the final submission. A Markdown review alone does not satisfy the PDF prompt-record requirement.

### F14 — Final student and submission checks [Verification gate]

**Evidence:** identifying fields in the study-guide PDFs are blank; presentation files do not supply both students' identifying details. The specification says identifying details are ideally included, so this is a readiness improvement, not an invented mandatory HD content rule. Actual Moodle submission, contribution and attendance cannot be inferred from Git.

**Remedy:** obtain names, IDs and Monash emails when preparing the final package; record contributions accurately. Both students must submit the same required actual files and attend/present under the assessment instructions. Confirm the relevant class deadline from the supplied course information. Only apply special-consideration alternatives if actually approved.

**Closure:** verified student details and final upload manifest, with both students confirming submission and presentation readiness. Do not substitute repository links for file uploads.

## How to use the root context

| Material | Relevant concepts and location | Planned application |
|---|---|---|
| Topic 7A | Dependencies pp6–10; Amdahl pp12–15; communication fabric/serial overhead pp18–27 | F05 performance bound; F08 why scale can waste resources when communication/synchronization dominates. |
| Topic 7B | Network congestion, variability and saturation pp13–21 | GPU-cluster communication contention and latency; explain why nominal network bandwidth is not guaranteed application throughput. |
| Topic 7B supplement | Queueing primer, explicitly non-examinable | Optional qualitative support for contention only; no need to add queueing derivations to this short talk. |
| Topic 7C and Workshop Week 7 | Alternative scaling models, partitioning and dependencies; Amdahl/Gustafson/LogP | Distinguish fixed-work speedup from larger-workload capacity; connect decomposition/communication to model training. No obligation to teach every model. |
| Topic 8 | GPU/CPU memory and controller/worker material pp18–23; locality pp32–35; CUDA pp41–44; bandwidth/scaling pp45–46 | F03–F05 host/device model, GDDR/HBM versus link bandwidth, launch/indexing, locality and performance limits. |
| Topic 9A | Synchronization/dependency latency pp2–7; causality pp21–25 | Completion/order before host reads and gradient-dependent updates. Clock synchronization is a different problem; do not insert NTP/Lamport algorithms into CUDA rotation. |
| Topic 9B | Mutual exclusion pp4–11; deadlock pp13–16; resource allocation/prevention pp24–32 | F12 explain why independent output writes need no mutex; optional cluster-allocation/liveness discussion with an actual mechanism. |
| Workshop Week 9 | GPU/controller-worker slide 10; memory slides 11–13; peak FLOPS/transfers slide 18; GPU allocation/deadlock slides 43–44 | Reinforce device model, peak-versus-achieved performance and resource-management reasoning. This deck does not contain the missing brightness notebook. |
| Weeks 1–5 and root C examples | Parallel taxonomy, locality, threads/barriers, MPI collectives; Topic 5 pp148–155 and MPI_Allreduce examples | Supporting foundations: parallel decomposition, row-major pixel indexing, synchronization and collective gradient reduction. Do not claim a production framework necessarily uses this exact MPI example. |
| Week 6 material | Earlier distributed wireless-sensor assessment/rubric | Background only. It is not the current Applied 2 specification and must not supply additional marking requirements. |

The two Topic 8 PDF copies are byte-identical and were treated as one source. Older course hardware figures should not be presented as current 2026 specifications. The course does not replace specialist GDS documentation or ethics research; use it to explain mechanisms, then cite the appropriate primary sources for new claims.

The specification's GPU/NPU learning objectives provide background. Its assessed Task 1 is CUDA rotation and Task 2 is HPC ethics; the rubric does not impose an additional standalone NPU comparison or require every Week 9 clock/deadlock algorithm.

## Execution sequence for the later revision

### Phase 1: establish evidence and repair citations

1. Preserve a baseline and create a claim/source matrix covering every substantive slide claim and all four Task 2 topics. Include research type, date/version, scope, limitation and destination slide.
2. Resolve F01 without retaining unverifiable notebook assertions. Normalize references under F02. Research the additional access evidence needed for F07 and verify it before drafting a claim.
3. Keep the correct current chart values and their scopes. Change chart data only if new, inspected source evidence justifies a change; synchronize JSON, embedded workbooks, labels and notes together.

Targets: both study-guide Markdown files, slide reference lists/notes, README, Task2_Chart_Data_and_Sources.json only if necessary. Dependencies: actual notebook retrieval if retaining notebook-specific analysis; otherwise the removal/replacement route needs no missing file.

### Phase 2: revise Task 1's explanation and examples

Implement F03–F06 within the six main slides: transfer mechanism; rotation mapping/code; launch/indexing; SIMT/SM scheduling; end-to-end performance/features; GDS operation and decision. Distribute short code excerpts across these slides and retain longer appendix examples. Use course terminology in explanations rather than adding a glossary to memorize.

Targets: Task1_GPU_Image_Rotation_Final.pptx, Task1_Speaker_Notes_and_Study_Guide.md, both CUDA files if excerpt alignment or necessary clarification requires changes. Do not build a complete application solely to satisfy an invented requirement.

Checks: validate mapping sign/rounding, RGB offsets, output uniqueness, bounds guards and grid recomputation. If CUDA becomes available, compile and run a small correctness harness against a CPU reference for zero/90-degree rotation, a non-right angle, small/non-square/non-multiple dimensions and borders. If unavailable, retain an explicit uncompiled teaching-example qualification. Do not report runtime or speedup measurements without actually performing them.

### Phase 3: rebalance Task 2 and make comparisons assessable

Implement F07–F09 using a clearly labeled illustrative HPC scenario and a four-topic evidence matrix. A practical starting layout is five main slides: parallel-training mechanism; environmental evidence/decision; governance/security/fairness; access/concentration; responsible allocation/scheduling. Splitting the current combined governance/access slide creates room to explain both without inventing a new requirement for slide count.

Retain approximately three minutes for this section initially. Proposed starting allocations: 25 seconds mechanism, 50 environment, 40 governance, 30 access and 35 framework/closing. For Task 1 a starting allocation is 25/30/35/25/35/30 seconds. These are planning targets, not validated delivery times. Rehearsal decides final timing and selection.

Targets: Task2_HPC_AI_Ethics.pptx and Task2_Speaker_Notes_and_Study_Guide.md; chart JSON only for justified changes. Add editable mechanism/control and scheduling/allocation diagrams. Preserve precise source caveats even when compressing spoken language.

### Phase 4: integrate and regenerate the complete submission

Implement F10, synchronize the combined deck with the standalone decks and update the six-minute script in natural spoken language. Keep one shared Q&A slide and move all labeled appendices after it. Regenerate both guide/AI-record PDFs and the script PDF; align slide numbers and cross-references. Extend the AI record for F13 from actual prompts, not a reconstructed fictional conversation.

Targets: Applied2_Combined_Presentation.pptx, Applied2_Six_Minute_Spoken_Script.md/.pdf, both Task*_Study_Guide_and_AI_Record.pdf files, both task Markdown guides and README. If using a separate AI-continuation PDF, add it to the final manifest and explicitly cross-reference it from both guides.

### Phase 5: audit outputs and complete human verification

1. Re-run the criterion table and close F01–F10 against observable final evidence. Render every final slide and each changed PDF page; inspect readability, clipping, source footers, chart units and diagram correctness.
2. Extract slide/notes/PDF text to check standalone/combined and Markdown/PDF consistency. Verify reference numbering, all local file paths, retained source claims, embedded chart workbooks and any updated arithmetic. Confirm no placeholder identities, unsupported speedup, fabricated job measurements or unlabeled hypothetical comparisons enter the package.
3. Execute F11–F12 with both presenters. Log actual timing and question weaknesses; revise only where results justify it, then recheck affected outputs.
4. Complete F13–F14 and verify a manifest of actual files for both students' Moodle submissions. Remaining unknown identity/deadline/attendance information must be confirmed by the students, not guessed.

A content-repair phase can finish autonomously with available evidence. Rehearsal, identities and actual submission require real student evidence; those gates remain open until verified.

## Primary-source checks supporting this review

- CUDA model and optimization claims were checked against NVIDIA's [programming model](https://docs.nvidia.com/cuda/cuda-programming-guide/01-introduction/programming-model.html) and [Best Practices Guide](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html). These support the transfer, indexing, memory-access and conditional overlap explanations; they do not prove performance of the submitted examples.
- GDS control/data paths, cuFile and compatibility considerations were checked against NVIDIA's [Overview Guide](https://docs.nvidia.com/gpudirect-storage/overview-guide/index.html) and [Design Guide](https://docs.nvidia.com/gpudirect-storage/design-guide/index.html).
- The sector electricity estimates, projection and growth figures were checked against the IEA's [2026 executive summary](https://www.iea.org/reports/key-questions-on-energy-and-ai/executive-summary). The NAIRR counts and US programme scope were checked against the NSF's [19 March 2026 update](https://www.nsf.gov/cise/updates/nairr-2-years-advancing-american-artificial-intelligence).
- Research scope was checked against [Deep Leakage from Gradients](https://arxiv.org/abs/1906.08935), the published [Bias and Fairness survey](https://aclanthology.org/2024.cl-3.8/), [The De-democratization of AI](https://arxiv.org/abs/2010.15581), [Carbon-Aware Computing for Datacenters](https://arxiv.org/abs/2106.11750), [Green AI](https://arxiv.org/abs/1907.10597), and [Making AI Less Thirsty](https://arxiv.org/abs/2304.03271).
- Framework descriptions were checked against the [OECD AI Principles](https://www.oecd.org/en/topics/ai-principles.html) and [NIST AI RMF Playbook](https://airc.nist.gov/airmf-resources/playbook/). Neither source automatically certifies the proposed cluster policy.

Assessment sources: [Applied 2 specification](<assessment/Applied #2 - Assessment Specification.pdf>) and [Applied 2 rubric](<assessment/Applied #2 Rubric.pdf>). These supplied documents control this review. All finding IDs are open at the reviewed baseline; closing them is the subsequent implementation task.

## Revision closure record — 8 October 2026

The teaching-team guidance overrides the audit's more expansive suggested hardware detail: Task 1(a) and 1(c) are addressed at a clear high level, with optional code supporting 1(b). The primary combined deck and its PDF contain the material needed for content assessment, including supporting detail and a brief AI declaration. No separate report is required.

| Finding | Current resolution / primary combined slides |
| --- | --- |
| F01 | Missing notebook assertions removed; current References slides 42–45 use public websites only. Local course-file citations are excluded at the student's request. |
| F02 | One numbered academic scheme across decks/companions/chart JSON; versioned papers and publication metadata normalized. |
| F03 | Slide 2 shows conventional decoded-pixel flow, host/device memory, H2D/D2H, DMA, PCIe and completion. Teaching guidance's high-level scope is respected. |
| F04 | Main slides 3–4 show inverse rotation, CUDA launch/indexing, a brightness excerpt and the grid/block/warp/SM hierarchy. Detailed calculations and complete crafted implementations remain in appendix slides 13–20 and 33–37. |
| F05 | Main slides 4–5 explain residency, locality, CPU baselines, completed timing and conditional overlap. Detailed launch-shape analysis, Amdahl bound and streams are on slides 18–20; profiling is on slide 38. No measured speedup is invented. |
| F06 | Main slide 6 combines conventional/direct data-flow diagrams, CPU control/fallback, decoding limits and a workload-specific adoption decision. Slide 39 adds background. |
| F07 | Main Task 2 slides 7–10 retain all four required topics and selected research comparisons. Full comparisons, methodological limits and policy expansions remain on slides 21–30 and 40–41. |
| F08 | Main Task 2 slides 7–10 retain resource impacts, security/fairness, access policy and accountable scheduling. Energy/carbon calculations and full governance/policy detail remain in appendix slides 22–30. |
| F09 | Main diagrams/charts explain transfer, rotation, execution, GDS, environmental demand, governance and framework decisions. Detailed tables/diagrams remain in the appendix; all remain native/editable. |
| F10 | Main conclusions, limitations and next measurements are on slide 11; the previous full synthesis is retained on slide 31. |
| F11–F12 | Excluded from this revision by explicit student instruction; no delivery/Q&A certification is asserted. |
| F13 | Slide 46 is one brief AI model/tool and use declaration. Prompt-list slides are excluded from all current decks and companions at the student's request; historical source records remain in tools/. |
| F14 | Both students' supplied names, IDs and emails appear on the cover. Actual Moodle upload, contribution/attendance and delivery remain student responsibilities. |

Mathematical/source/package checks and visual checks are retained privately under .build/. CUDA compilation and hardware execution remain unverified and are explicitly qualified in the deck; neither is imposed by the teaching guidance. Content completeness is assessed against the Task 1/2 HD descriptors, without an unsupported guarantee of a grade.

The student's subsequent timing instruction takes precedence over the earlier placement request: present slides 1–11 with a 6:20 planning target and 40-second buffer. Detailed content was moved rather than deleted. The appendix starts at slide 12. Embedded speaker notes and Presentation_Timing_and_Notes.md contain the current timing path; no measured rehearsal or delivery/Q&A grade is asserted.
