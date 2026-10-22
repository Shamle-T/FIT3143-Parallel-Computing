# Applied 2 Q&A rehearsal guide

Based on the current files in `applied2/submission`, especially the submitted 46-slide PowerPoint and its two CUDA source files. Checked against FIT3143 Topics 7A, 7B and 8 and the public sources linked below. Prepared 9 October 2026.

These are suggested answers to say aloud, not quotations from sources. Questions about the hardest part and greatest concern are proposed judgments; adapt them to your own experience. Repeated questions are merged. "See vibes" contains no specific question, so I do not know what was intended by that phrase.

**Your actual submission:** illustrative, uncompiled and unbenchmarked CUDA examples; contiguous 8-bit RGB; one thread per output pixel; inverse mapping; nearest-neighbour sampling; fixed input/output dimensions; black outside the source; separate input/output buffers. The basic host example uses a 16 x 16 block. The performance example uses a 32 x 8 block and asynchronous operations in a supplied stream. Neither establishes measured correctness or speedup. Task 2 proposes policies rather than reporting a deployed cluster framework.

**Reasoning to use in technical answers:** identify the dependency, identify the relevant resource limit, then state what measurement would confirm the choice.

## 1. If computation takes too long, where would you optimise?

**Say:** "I would first measure loading, transfers, kernel execution and saving separately. If transfers dominate, I would keep images on the GPU across operations and reuse buffers. For independent images, I would test overlapping copies and computation. If the kernel dominates, I would examine memory access and compare block shapes. I do not know our actual bottleneck because the examples have not been benchmarked."

**If asked for specifics:** your examples offer 16 x 16 and 32 x 8 blocks, both with 256 threads. A 32-wide block places a warp along one output row, but rotated source reads and three-byte RGB layout still affect memory transactions. No block shape is guaranteed to win. CPU load/decode/save is also part of the complete pipeline. GDS is relevant only if storage I/O is the limiting stage and a supported direct path is usable.

For the sequential GPU path:

`T_total = T_setup + T_H2D + T_kernel + T_D2H`

Include file I/O consistently on both CPU and GPU paths. With overlap, measure completion of the whole batch rather than adding overlapping durations. This matches the profiling approach in [NVIDIA CUDA Best Practices](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html).

## 2. What would be the hardest technical part to implement?

**Say:** "The hardest part would be building a correct pipeline that overlaps transfers with GPU work. Each image needs correctly managed buffers and dependencies. I must not read an unfinished output or reuse a buffer while the GPU is using it. Rotation also needs consistent coordinate conventions and boundary checks. Our submission illustrates these ideas but does not implement a complete tested application."

**If asked about completed work:** do not claim you debugged or benchmarked a working CUDA application. The files explicitly say they have not been compiled or executed. The performance helper queues work in a caller-supplied stream; it does not create and manage a complete multi-image pipeline.

## 3. What if one thread processes ten pixels?

**Say:** "The advantage is that one thread can reuse some calculations across several pixels, and fewer logical threads are needed. The disadvantage is that it performs more work sequentially, so a small image may expose too little parallelism. The indexing can also make neighbouring threads access widely separated pixels. I would compare it against our one-pixel-per-thread design rather than assume it is faster."

**If asked further:** register use depends on how the loop is written; more pixels do not automatically mean ten times as many registers. Giving each thread a contiguous ten-pixel chunk makes corresponding accesses from neighbouring threads ten pixels apart. A suitably designed strided loop can keep neighbouring threads on neighbouring pixels at each iteration. Both versions can use one kernel launch, so ten pixels per thread does not inherently reduce kernel-launch count.

## 4. What are the limitations of your presentation?

**Say:** "The main limitation is that our CUDA examples have not been compiled, verified or benchmarked, so we show expected behaviour rather than measured speedup. The rotation uses nearest-neighbour sampling and a fixed canvas, which can produce jagged edges and cropping. GDS benefits depend on system support. Our ethics framework is a proposal, and sector forecasts do not measure the footprint of our own workload."

**Other scope limits:** fixed three-channel RGB; no complete image loading/saving application; no empirical CPU/GPU comparison; no tested multi-GPU rotation; no deployed governance policy. Historical compute-access research is not a current worldwide census. More GPUs do not establish either privacy or fair model outputs.

## 5. How does the image move from CPU to GPU for rotation?

**Say:** "The CPU loads and decodes the image into RGB pixels in host RAM and allocates separate GPU input and output buffers. A host-to-device copy transfers the pixels over PCIe. The CPU launches the CUDA kernel, and the GPU writes the rotated output. The program then copies the completed result back to host RAM for saving."

**Your data flow:** storage -> CPU decode -> host RGB buffer -> H2D copy -> GPU input -> rotation kernel -> GPU output -> D2H copy -> host output -> save.

The CPU also supplies `cos(theta)` and `sin(theta)` in your design. DMA engines move the bytes; the CPU does not need to copy every transferred byte using its own instructions.

## 6. What data or computation is independent inside the GPU?

**Say:** "The independent unit is one output pixel. Each thread calculates that pixel's source coordinates, reads the source RGB values, and writes its own output location. It does not need another thread's result. Different threads may read the same input pixel, which is safe because the input is read-only. Their output writes remain separate."

**If asked what is stored:** the GPU holds the input and output pixel arrays. The kernel receives dimensions and rotation parameters; each thread computes its coordinates. Independence means no dependency on another thread's output, not that all threads need private copies of the image.

## 7. What is inverse mapping, and why use it?

**Say:** "Instead of pushing each input pixel to a new position, I start with each output pixel and work backwards to find its source. That gives every output location a defined sampling step and avoids scatter holes and competing output writes. Our code samples the nearest source pixel, or writes black when its rounded coordinates are outside the input."

**Your equations**, with x increasing rightward and y downward:

```text
cx = (W - 1)/2; cy = (H - 1)/2
c = cos(theta); s = sin(theta)
sx = c*(x - cx) - s*(y - cy) + cx
sy = s*(x - cx) + c*(y - cy) + cy
ix = floor(sx + 0.5); iy = floor(sy + 0.5)
```

Positive theta is visually counterclockwise under these conventions. The forward rotation matrix is `[c s; -s c]`; its inverse is `[c -s; s c]`. Inverse mapping does not remove nearest-neighbour aliasing or fixed-canvas cropping.

## 8. What happens at 90 degrees?

**Say:** "For a square image and an exact 90-degree counterclockwise rotation, the mapping simplifies to source x equals N minus one minus output y, and source y equals output x. In our five-by-five example, output pixel zero, one reads input pixel three, zero. A full rectangular 90-degree rotation would swap output width and height; our current fixed canvas does not do that."

**Follow-up:** the square-image formula is `ix = N - 1 - y; iy = x`. A specialised integer kernel can avoid general-angle floating-point calculations and rounding. Your current kernel is the general-angle version.

## 9. Why are separate input and output buffers needed?

**Say:** "If I rotate in place, one thread could overwrite a source pixel before another thread has read it. Separate buffers keep the original image unchanged until all reads finish. Each thread writes only to its assigned output pixel, so this kernel needs neither atomics nor a block barrier."

**Follow-up:** two device RGB buffers use `6*W*H` bytes in total, excluding other allocations. Host input/output buffers require additional memory. A safe in-place algorithm would need a different design; simply adding a block barrier cannot coordinate arbitrary threads in different blocks.

## 10. Would you choose a CPU or GPU for image rotation?

**Say:** "For large images, batches, or images already on the GPU, I would test a GPU because many output pixels can be processed independently. For a small one-off image in CPU memory, the CPU may finish sooner because it avoids GPU setup and transfers. I would choose using completed end-to-end time and equivalent output quality."

**Follow-up:** your presentation calls for both serial and multithreaded CPU comparisons. Use the same angle, dimensions, sampling and boundary rules. A faster kernel alone does not prove a faster application.

## 11. If a CPU had 1,000 cores, would a GPU still help?

**Say:** "It could, but core count alone cannot decide that. CPU cores and GPU execution units have different designs. A GPU may offer useful memory bandwidth and throughput for regular pixel work, while a many-core CPU avoids offload transfers for host-resident data. I do not know which would be faster without the actual hardware and measurements."

**Unit connection:** Topic 8 contrasts processor architectures and memory bandwidth; Topic 7A explains why serial work limits gains from adding processors. Neither guarantees that a GPU beats every many-core CPU.

## 12. Can image rotation run completely on a GPU without the CPU?

**Say:** "The pixel calculations can run entirely on the GPU once the data and parameters are available. Our complete program still needs the CPU to load and decode the image, allocate buffers, submit transfers, launch the kernel and save the result. CUDA uses a host-device model. We can reduce CPU involvement, but our design is not a standalone GPU-only system."

**Follow-up:** GPUs can perform more than pixel arithmetic, and specialised pipelines can move decoding and other stages onto them. That does not remove the host and system software from this submitted pipeline. GDS bypasses host staging on its direct data path, but still has a CPU control path.

## 13. What is the PCIe bottleneck, and why is it a bottleneck?

**Say:** "PCIe has a limited rate for moving data between host and GPU memory. If moving the input and output takes most of the total time, the transfers are the bottleneck, even when the rotation kernel is fast. The actual rate depends on the negotiated PCIe generation, lane count and the rest of the transfer path. We have not measured it for this example."

**Follow-up:** PCIe bandwidth is different from GPU-local GDDR/HBM bandwidth. For your RGB format, copying a whole input and output moves `6*W*H` payload bytes across the host-device link. Transfer startup and other overhead add to the cost. PCIe is a potential bottleneck, not an established diagnosis of your unbenchmarked code.

## 14. Why is speed still capped after using pinned memory or streams?

**Say:** "Pinned memory can reduce staging overhead, and streams can hide some transfer time behind independent computation. Neither increases the physical capacity of the PCIe link. Once that path is saturated, more simultaneous transfers share its capacity. To improve further, I would reduce transferred data or use a faster supported path."

**Follow-up:** software can help approach a hardware limit; it cannot make that link exceed its physical bandwidth. Overlap can improve completed batch time even while the bytes-per-second limit of a particular transfer path stays the same.

## 15. Why is bandwidth important?

**Say:** "The processors need data to stay busy. In our rotation, every output pixel requires reading source values and writing RGB values. Limited GPU memory bandwidth or inefficient accesses can restrict the kernel. Limited PCIe bandwidth restricts transfers. In multi-GPU AI training, network bandwidth also affects how quickly workers exchange results."

**Follow-up:** bandwidth is data transferred per unit time; latency is delay for an operation. A rough bulk-transfer estimate is `time >= bytes/effective_bandwidth`, with startup and dependency costs also relevant. Extra cores cannot eliminate a data-supply limit. Topics 7A and 7B also connect communication delays and saturation to parallel speedup.

## 16. What are pinned memory and pageable memory?

**Say:** "Pageable memory is ordinary host memory managed by the operating system, which may page it out. Pinned memory is host memory kept resident so it is suitable for DMA transfers. CUDA may stage pageable data through pinned storage, adding overhead. Pinned buffers support efficient asynchronous transfers, but pinning too much reduces memory available for normal system use."

**Terminology:** 'pageable' is the useful term here. It does not mean the data is currently on disk. Both are host memory, not GPU memory. Your performance helper requires pinned host buffers, but their allocation is left to the caller. `cudaMallocHost` is mentioned as an example. See [CUDA memory-transfer guidance](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#pinned-memory).

## 17. How do CPU-GPU transfers affect performance?

**Say:** "Transfers add time before and after the kernel. Our sequence uploads the input, rotates it, and downloads the output. For a simple rotation, moving the image can take enough time to outweigh the faster pixel processing. Keeping data on the GPU for several operations spreads that transfer cost across more useful work."

**Follow-up:** speedup is `T_CPU/T_GPU,total`, not just `T_CPU/T_kernel`. This submission has no measured speedup. Ordinary host-device copying does not automatically compress or reduce the pixel payload.

## 18. Why is CPU-GPU synchronisation needed in CUDA?

**Say:** "A kernel launch returns before GPU execution finishes. The CPU must know the output is ready before reading, saving, reusing or freeing its buffers. Our basic synchronous device-to-host copy supplies completed output before returning. With the asynchronous helper, the caller must wait for the relevant stream or completion event."

**Follow-up:** operations submitted to the same stream execute in order, so the helper preserves H2D -> kernel -> D2H dependencies. Successful submission and `cudaGetLastError()` do not prove successful completion. This host-device waiting is separate from `__syncthreads()`, which is unnecessary for this independent-pixel kernel. [NVIDIA API synchronisation behaviour](https://docs.nvidia.com/cuda/cuda-runtime-api/api-sync-behavior.html).

## 19. Why is copying back after one kernel inefficient?

**Say:** "A full-image download may be expensive compared with the small amount of work done by one rotation kernel. It becomes avoidable overhead if the next operation also runs on the GPU, because the image would then be uploaded again. I would keep it on the GPU between operations and download the final result. If the CPU needs the finished image now, that copy is appropriate."

**Follow-up:** your host code requests the copy; the GPU does not automatically send data back after every kernel. Rotation followed by brightness could use the output as the next kernel's input and swap device buffer roles, then copy back once. Your current files illustrate these kernels but do not implement that combined application.

## 20. How does GPUDirect Storage work, and how is it different from traditional I/O?

**Say:** "A conventional GPU storage pipeline reads data into host RAM and then copies it into GPU memory. In a supported GDS direct path, a storage-side DMA engine transfers data into GPU memory without the host staging buffer. The CPU still submits and controls I/O through cuFile. This can reduce staging costs, but it does not remove PCIe limits, decode images or accelerate the rotation arithmetic."

**Your application:** a CPU-decoded image still produces its RGB pixels in host RAM. GDS would need a suitable GPU-ready data pipeline to be useful; using cuFile alone does not replace decoding. Benefits depend on storage, software, filesystem and topology support, and compatibility fallback may use host staging. [NVIDIA GDS overview](https://docs.nvidia.com/gpudirect-storage/overview-guide/index.html), [GDS design guide](https://docs.nvidia.com/gpudirect-storage/design-guide/index.html).

## 21. Which framework would be hard to implement?

**If they mean the technical stack:** "GDS integration would be challenging because it requires a compatible storage and software configuration and a usable direct data path. I would also need to address decoding and verify that storage I/O is actually the bottleneck. Our presentation explains GDS conceptually; it does not implement it."

**If they mean the Task 2 framework:** "Our combined responsible-use framework would be challenging to implement because environmental goals, deadlines, data rules and fair access can conflict. It needs reliable measurements, accountable owners and a process for exceptions. The difficult part is applying the principles consistently to real jobs. We propose that process; we have not deployed it."

**Follow-up:** NIST supplies Govern, Map, Measure and Manage functions; your framework adds efficiency reporting, allocation policies and constrained carbon-aware scheduling. Do not claim NIST alone provides all these components, or that adopting its playbook proves compliance. [NIST AI RMF Playbook](https://airc.nist.gov/airmf-resources/playbook/).

## 22. Why do AI models raise environmental ethical issues?

**Say:** "Training and running AI models use electricity, and the infrastructure also has cooling and hardware impacts. Electricity can cause emissions, while cooling and power generation can place demands on water. The ethical issue is who receives the benefits and who bears these costs. We should measure resource use per useful result and justify whether larger computation is necessary."

**Follow-up:** the model itself is not a moral actor. The ethical decisions concern its development, deployment and infrastructure. Impacts vary with workload, electricity mix, location and cooling method. Water consumption and water withdrawal are distinct measures. Your deck also includes manufacture and disposal in the wider environmental scope. [Green AI](https://arxiv.org/abs/1907.10597), [AI water-footprint research](https://arxiv.org/abs/2304.03271v5).

## 23. What ethical problems could arise from using very large numbers of GPUs?

**Say:** "Large clusters can increase resource demand and favour organisations able to afford them. Distributed AI workloads also create more data and intermediate results to protect. Powerful models may still have unfair outputs or be misused. I would evaluate environmental costs, security, model fairness and access separately, then require evidence that the additional scale creates useful benefits."

**Follow-up:** more GPUs do not inherently cause bias or leakage, and they do not guarantee those problems are solved. The workload, data and controls determine the risks. The same cluster may also support beneficial research; ethical assessment must consider the actual use.

## 24. If AI continues expanding rapidly, what would be your greatest concern?

**Say:** "My main concern would be growth outpacing accountability: organisations increasing compute without adequately measuring environmental costs, protecting data or providing fair access. I would judge expansion by useful outcomes and safeguards, rather than treating a larger model or cluster as an achievement by itself."

**Follow-up:** this is a reasoned judgment, not a claim that one concern is objectively greatest. It connects directly to your proposal's measurement, ownership, review and allocation rules.

## 25. What is the biggest concern AI creates for universities?

**Say:** "For teaching, I would prioritise whether assessment still demonstrates students' own understanding. A student may submit convincing AI-assisted work without being able to explain it. Clear permitted-use rules, appropriate acknowledgement and opportunities to demonstrate reasoning can help. For research, our presentation highlights another concern: unequal access to compute can limit participation and independent replication."

**Follow-up:** distinguish teaching integrity from research access. Permitted AI assistance is not automatically misconduct; conditions depend on the assessment. Privacy and unequal access to learning tools are additional concerns. Monash's guidance explicitly focuses on assessment providing valid evidence of learning. [Monash AI and assessment guidance](https://www.monash.edu/learning-teaching/TeachHQ/Teaching-practices/artificial-intelligence/ai-and-assessment).

## 26. How does scaling HPC affect data governance and security?

**Say:** "Scaling across more workers can create more copies of datasets and checkpoints and more communication of gradients. Governance means tracking where data came from, what uses are permitted, who can access it and when copies must be deleted. Security means protecting those assets and transfers. I would use least privilege, isolation, protected transport and storage, audit logs and clear ownership."

**Follow-up:** a gradient is a training derivative used to update model parameters; a checkpoint stores training/model state for recovery. The research cited in your deck demonstrates that exposed gradients can reconstruct training inputs in studied settings. That does not mean every distributed training system leaks. Encryption helps protect transport and storage but does not solve improper authorised use or fairness. [Deep Leakage from Gradients](https://arxiv.org/abs/1906.08935v2).

## 27. Is HPC access evenly distributed? Why does concentration at the top matter?

**Say:** "Our cited historical research identifies a compute divide benefiting large firms and well-resourced universities. That matters because limited access can restrict who trains models, chooses research questions and independently checks results. I would propose transparent shared-cluster allocation, support for smaller institutions, training and appeals. I do not know the current worldwide distribution from the evidence in our presentation."

**Follow-up:** equal allocation gives everyone the same amount; equitable allocation accounts for justified needs and barriers. Your policy reserves some capacity and monitors waiting times. This may reduce immediate utilisation or delay urgent jobs, so the trade-off needs review. Historical research supports an access concern, not a current global inventory. [Ahmed and Wahed's compute-divide study](https://arxiv.org/abs/2010.15581v1).

If "distribution" means workload distribution instead: "I would aim for balanced work so slow workers do not delay everyone. Equal pixel counts do not always guarantee equal execution time. Work partitioning and fair access are different questions."

## Additional likely follow-ups

**Does faster execution always use less energy?** "No. Energy depends on power over time. Our illustrative slide compares 1 kW for 2 hours, or 2 kWh, with 2 kW for 1.2 hours, or 2.4 kWh. The faster job uses more energy in that example. Those numbers are assumptions, not measurements."

**Can all jobs wait for cleaner electricity?** "No. Scheduling depends on deadlines, forecast uncertainty, water and site limits, and fair waiting times. Our proposal shifts flexible jobs and records justified exceptions." The scheduling research demonstrates delaying temporally flexible workloads, rather than every urgent job. [Carbon-Aware Computing for Datacenters](https://arxiv.org/abs/2106.11750v1).

**Do the electricity charts measure AI training alone?** "No. The main chart covers global data-centre electricity, including AI and non-AI workloads. The AI-focused growth group is a subset of the total group, so their percentages must not be added. The future value is a projection, not an observed result."

**Do more GPUs guarantee a fair model?** "No. More compute can support broader evaluation, but model fairness depends on data coverage, the task and the harms being tested. I would examine affected groups separately and reevaluate after mitigation."

**How does Amdahl's law apply?** "For a fixed workload, accelerating pixel computation does not remove serial loading, setup or communication costs. The ideal bound is `S = 1/(s + (1-s)/N)`. Actual communication and other overhead can make speedup lower. Our CUDA thread count is not automatically the N of a valid measured speedup model."

## Local evidence used

- `applied2/submission/Applied2_Combined_Presentation.pptx`: slides 2-6 and 13-20 for Task 1; slides 7-11 and 21-30 for Task 2; slide 31 for next measurements and limits.
- `applied2/submission/Task1_CUDA_Kernel_Examples.cu`: implementation assumptions, rotation, brightness and commented host launch.
- `applied2/submission/Task1_CUDA_Performance_Examples.cu`: 32 x 8 launch, caller-owned pinned buffers, stream ordering and completion responsibilities.
- `applied2/submission/Task2_Chart_Data_and_Sources.json`: research links and chart scopes.
- `context/FIT3143-Topic-8-2024.pdf`: pages 19-23 on bandwidth and architecture, 32-35 on data parallelism and locality, 42-46 on CUDA and integration.
- `context/FIT3143-Topic-7A-2024.pdf`: pages 8-13 on dependencies and Amdahl's law; 18-27 on distributed communication and granularity.
- `context/FIT3143-Topic-7B-2024.pdf`: pages 19-24 on saturation, traffic and bandwidth.

No CUDA source, submitted presentation or chart data was modified while preparing this guide.
