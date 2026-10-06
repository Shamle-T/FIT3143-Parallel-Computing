# Applied 2: GPU image rotation and the ethics of scaling HPC for AI

## Task 1

The `task1` folder contains a three-minute presentation, a study guide with answers to all seven workshop reflection questions and additional performance questions, editable speaker notes, and illustrative CUDA examples. Present the six main slides. Keep the five clearly labelled appendix slides for Q&A and references.

The proposed rotation uses one thread per output pixel, inverse mapping about the image centre, nearest-neighbour sampling, the original output dimensions, and black fill for uncovered pixels.

### Is a CUDA program compulsory?

The assessment specification encourages a CUDA prototype or reference to an established image-processing library. It does not require a working program. The marking rubric rewards sample code that explains the programming model. The provided kernels support that explanation, but they are not a standalone program and have not been compiled, executed or benchmarked. No measured speed-up is claimed.

### Submission preparation

Add both students' names, IDs and Monash email addresses. Rehearse the presentation and prepare to answer questions without AI tools. The study-guide PDF includes the AI declaration and prompt record required by the specification.

The latest confirmed allocation is **three minutes each, six minutes total**. The rubric expects six to seven minutes; rehearse with a timer and leave natural pauses. Q&A is separate.

## Task 2

The `task2` folder contains four main slides, three reference appendix slides, a detailed study guide, editable notes, and the chart data with source URLs. It connects HPC, parallel computing and AI training, then covers all four required ethical topics. Environmental and energy impacts receive the most detail; governance, security, fairness, access and policy receive concise coverage.

The two native PowerPoint charts have embedded editable workbooks. IEA 2026 figures distinguish the 2025 estimate from the 2030 projection and distinguish whole-sector electricity totals from the AI-focused subset. The growth percentages describe overlapping categories and must not be added or interpreted as shares. Research comparisons cover gradient privacy, fairness evaluation, the compute divide, Green AI and carbon-aware scheduling.

## Combined presentation and spoken script

Use `Applied2_Combined_Presentation.pptx` for the team presentation:

| Slides | Presenter / purpose | Planned time |
| --- | --- | --- |
| 1-6 | You: Task 1 | 3 minutes |
| 7-10 | Your friend: Task 2 | 3 minutes |
| 11 | Shared Q&A | Separate 1-2 minutes |
| 12-19 | Clearly labelled CUDA and reference appendices | As needed |

`Applied2_Six_Minute_Spoken_Script.pdf` and its editable Markdown version provide conversational wording, slide cues, time targets and a handover. The scripts contain 385 and 367 words. Timing is a rehearsal target, not a measured performance.

## HD rubric review

| Criterion | Supporting material |
| --- | --- |
| Task 1a: memory transfer | H2D/D2H diagram; PCIe, DMA, RAM and GPU buffers; pinned-memory explanation |
| Task 1b: CUDA and speed-up | Launch and hierarchy diagrams; brightness/rotation kernels; block-shape and asynchronous-stream examples; coalescing, occupancy and end-to-end timing discussion |
| Task 1c: GPUDirect Storage | Conventional/direct paths; workload-dependent benefit; platform support, fallback and image-decoding limitations |
| Task 2: four topics and research | Current primary sources, research comparisons for each topic, two cited charts and a parallel-training diagram |
| Clarity, limitations and future work | Six-minute natural scripts; stated limitations; proposed measurement plans |
| Q&A: 40% | Concise study answers and rehearsal prompts in both guides |

The material targets the HD descriptors. The grade also depends on your oral explanation and independent Q&A performance. The CUDA examples remain uncompiled and unbenchmarked; no measured speed-up is claimed.

Submit both companion PDFs to preserve the complete AI declaration and preparation prompt record. Add both students' identifying details. Moodle requires the actual files, not just this repository link.

## Assessment sources

The `assessment` folder preserves the supplied specification and marking rubric. Sources used in explanations are cited in the slides and study guide.
