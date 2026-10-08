# Applied 2 — Teaching Team Guidance

Compiled from the teaching-team clarifications supplied by the student. This file records that guidance; it does not add new assessment requirements.

## Task 1(a): Host–GPU data transfer

- A clear, accurate **high-level explanation of the data-transfer process is sufficient**.
- Explain how image data moves between host memory and GPU memory, including the main steps and components involved.
- **CUDA transfer code is not required**. A short code snippet may support the explanation if helpful.
- The specification mainly asks for an explanation of the transfer process and relevant diagrams.

## Task 1(c): GPUDirect Storage

- Explain **how GPUDirect Storage works** and **why it may or may not benefit the image-rotation task**.
- **Code is not required**.
- Focus on the concept, data flow, relevant diagrams, and credible references.

## GPU speed-up: Baseline and explanation

The specification does not fix a particular comparison baseline. Choose a baseline and clearly state what the GPU implementation is being compared against.

- A **serial/single-threaded CPU implementation** is a common baseline for speed-up.
- Also comparing against a **multithreaded CPU implementation** can provide a fairer and more convincing comparison.
- Do not simply quote a speed-up figure. Explain **why the GPU is expected to be faster** and which factors affect the result.

For image rotation, relevant discussion points include:

- How well per-pixel operations map onto GPU threads.
- Whether the workload is compute-bound or memory-bound.
- Memory access patterns, including coalescing.
- The overhead of transferring data between host and device memory.

The key requirement is to explain the expected speed-up from modern GPU architectures, rather than restrict the discussion to one particular baseline.

## Task 2(b): Fairness and large-scale compute

- Focus on **fairness when AI models depend on massive compute clusters**.
- Research on AI model bias may be used, but **do not claim that HPC itself causes bias unless the source supports that claim**.
- A paper directly studying fairness in HPC clusters is not necessarily required.
- Make a clear, well-supported connection between the fairness issue and large-scale compute.

## Diagrams and level of detail

- No specific diagram type is prescribed.
- Flowcharts, architecture diagrams, data-flow diagrams, or other appropriate diagrams may be used. **Flowcharts are acceptable**.
- Diagrams must be relevant and help communicate the answer clearly.
- Explain the important concepts relevant to the question, without unnecessary detail about every underlying component.
- Include relevant hardware components where they help explain the process; there is no need to explain hardware details unrelated to the question.
