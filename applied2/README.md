# Applied 2: GPU image rotation and the ethics of scaling HPC for AI

## Task 1

The `task1` folder contains a four-minute presentation, a study guide with answers to all seven workshop reflection questions, editable speaker notes, and illustrative CUDA kernels. Present the six main slides. Keep the four clearly labelled appendix slides for Q&A and references.

The proposed rotation uses one thread per output pixel, inverse mapping about the image centre, nearest-neighbour sampling, the original output dimensions, and black fill for uncovered pixels.

### Is a CUDA program compulsory?

The assessment specification encourages a CUDA prototype or reference to an established image-processing library. It does not require a working program. The marking rubric rewards sample code that explains the programming model. The provided kernels support that explanation, but they are not a standalone program and have not been compiled, executed or benchmarked. No measured speed-up is claimed.

### Submission preparation

Add both students' names, IDs and Monash email addresses. Rehearse the presentation and prepare to answer questions without AI tools. The study-guide PDF includes the AI declaration and prompt record required by the specification.

The supplied newer guidance allocates four minutes to Task 1 and three to Task 2, within the seven-minute presentation. The written specification gives indicative allocations of three and four minutes. The current materials follow the newer guidance.

## Assessment sources

The `assessment` folder preserves the supplied specification and marking rubric. Sources used in explanations are cited in the slides and study guide.
