# Applied 2: Six-minute spoken script

**You: Task 1, 3 minutes. Your friend: Task 2, 3 minutes.**

Speak the paragraphs. Headings and cues are not spoken. Timing is a rehearsal target; the rubric expects 6-7 minutes.

## You - Task 1

### Slide 1: Image rotation with CUDA | 0:00-0:30

*Cue: Point to the host-to-device and return arrows.*

I'll explain how we'd rotate a large image using CUDA. The CPU, or host, loads and decodes the image, allocates buffers and launches the work. The GPU, or device, calculates the output pixels. In this design, system RAM and GPU memory are separate. We copy the pixels across PCIe using a host-to-device cudaMemcpy, then copy the result back. Those transfers take time, even when the rotation kernel is fast.

### Slide 2: One thread computes one output pixel | 0:30-1:05

*Cue: Point to the highlighted pixel and its sampled source.*

Brightness is data-parallel because each pixel gets the same independent adjustment. Rotation also gives each thread independent work, but the source position changes. We start with an output pixel and map backwards to its source, so every destination gets written. Here, that produces a ninety-degree counterclockwise rotation. We use nearest-neighbour sampling, keep the original dimensions and fill uncovered pixels with black. It's simple, but diagonal edges can look jagged and corners can be cropped.

### Slide 3: Threads, blocks and the image grid | 1:05-1:35

*Cue: Point to the launch line and thread calculation.*

CUDA organises threads into blocks, and blocks into a grid. The launch here specifies both. For a one-thousand-and-twenty-four square image, sixteen-by-sixteen threads gives two hundred and fifty-six threads per block. We need sixty-four-by-sixty-four blocks: four thousand and ninety-six blocks, launching just over one million threads. Our bounds check handles images that don't divide evenly by sixteen.

### Slide 4: Millions of threads share finite hardware | 1:35-2:00

*Cue: Point to the warp scheduler and execution units.*

That doesn't mean we need a million CUDA cores. An SM, or streaming multiprocessor, keeps blocks resident and schedules warps of thirty-two threads. The execution units are reused over time. More blocks enter as resources become available. Register use limits residency, so a bigger block doesn't automatically improve performance.

### Slide 5: Image size and transfer overhead | 2:00-2:30

*Cue: Point to the total-time equation.*

Larger images usually keep more SMs busy and spread launch overhead across more pixels. Coalesced writes use memory bandwidth efficiently, while rotation's irregular reads can limit performance. Keeping intermediate images on the GPU avoids repeated copies. For a fair speed-up, compare CPU time against the complete GPU time: setup, both transfers and the kernel. Our next step is to measure those stages; we haven't claimed a benchmark result.

### Slide 6: GPUDirect Storage and image rotation | 2:30-3:00

*Cue: Trace the direct storage path, then make eye contact for the handover.*

GPUDirect Storage can transfer data from supported storage into GPU memory through DMA, removing CPU RAM staging. The CPU still controls the I/O. It could help repeated large batches when storage is the bottleneck. It may offer little benefit for small jobs or CPU-only image decoding. So our design separates kernel performance from the complete pipeline. My teammate will now discuss what scaling that computing power means for AI.

## Your friend - Task 2

### Slide 7: HPC, parallel computing and AI | 3:00-3:40

*Cue: Point to the different data partitions and gradient synchronisation.*

We've seen how one GPU divides an image into parallel work. HPC takes that idea across many processors, with fast networks and storage. In data-parallel AI training, each GPU holds the same model but processes different examples. They synchronise gradients before updating it. Larger models can also split layers across GPUs. This increases capacity, but communication means doubling the GPUs won't necessarily halve the runtime. The ethical question is who benefits from that scale, and who pays for it.

### Slide 8: Electricity demand and environmental risk | 3:40-4:45

*Cue: Point to each chart; pause after explaining the forecast and overlapping categories.*

The environmental impact is our main focus. According to the IEA's 2026 report, all data centres used an estimated four hundred and eighty-five terawatt-hours in 2025. Its 2030 projection is nine hundred and fifty: roughly ninety-six percent more. That's the whole sector, including AI and other workloads, and the future bar is a forecast. On the right, electricity use grew seventeen percent across all data centres in 2025, versus fifty percent for the AI-focused subset. Those groups overlap, so we can't add the rates. Efficiency per task can improve while total demand grows, because usage grows faster. The actual harm also depends on the electricity source and local cooling-water pressure. A faster training run alone doesn't tell us its total energy or emissions.

### Slide 9: Data governance and access to compute | 4:45-5:20

*Cue: Point to gradient protection and the NAIRR example.*

Scaling also raises privacy, fairness and access questions. Research has shown that exposed gradients can reveal training data under some conditions. So distributing work doesn't automatically protect privacy, and more compute doesn't remove dataset bias. Expensive clusters also shape who can do research. Historical compute-divide research describes that concentration; NAIRR offers a current US response, supporting over six hundred teams and six thousand students. Shared capacity helps, but doesn't prove equal access worldwide.

### Slide 10: Responsible HPC policies | 5:20-6:00

*Cue: Pause at the limitations; finish together and invite questions.*

Our proposal combines performance targets with an energy budget. Green AI argues that efficiency should be evaluated alongside accuracy. Carbon-aware scheduling can move flexible jobs to cleaner periods, although urgent jobs can't always wait. OECD principles and NIST's risk framework guide privacy, fairness and accountability. We would also publish clear allocation rules. Next, measure a real training job's energy, scaling efficiency and useful output. These charts show sector trends, not that job's footprint. Responsible HPC means checking benefits against resource costs and who bears them. Thank you; we're happy to take questions.

## HD rubric review

### Task 1a: memory transfer

Separate RAM/GPU memory; H2D/D2H; PCIe and DMA; pinned-memory explanation; readable data-flow diagram. Main slide 1 and Task 1 study guide.

### Task 1b: CUDA and performance

Host/device roles; explicit grid/block launch; bounds checks; thread/block/grid and warp/SM diagrams; brightness, rotation and stream examples. Main slides 2-5 and CUDA appendices compare coalescing, locality, occupancy and transfer costs.

### Task 1c: GPUDirect Storage

Direct/fallback distinction, supported-platform requirement, CPU I/O control, compressed-image decoding caveat and workload-dependent benefit. Main slide 6 and cited NVIDIA research.

### Task 2: all four topics

Energy in detail; privacy/security/fairness; compute access; accountable frameworks. Research comparisons cover each topic, with sector chart scope and forecast limits explicitly explained.

### Clarity, limitations and future work

Natural scripts, consistent diagram order and handover. Task 1 proposes stage timings; Task 2 proposes measuring a real job. Neither claims measurements that were not performed.

### Q&A: 40% of rubric

Study and explain in your own words. Both guides include short answers. Prepared artifacts cannot establish your oral understanding or guarantee a grade.

## AI declaration

AI assistance helped draft and revise this spoken script. The full declarations and all preparation prompts are in the Task 1 and Task 2 companion PDFs; submit both records.
