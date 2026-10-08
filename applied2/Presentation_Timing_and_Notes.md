# Presentation timing and notes

Present slides 1–11, ending at the conclusion. The appendix is for assessment/supporting detail and is not included in the spoken path.

Target: 6:20, leaving 40 seconds within the seven-minute limit. These are planning targets, not a measured rehearsal.

The optional narration contains 763 words. At an assumed 130 words/minute, speech alone takes approximately 5.9 minutes. Allow the remaining time for pointing at diagrams, slide changes and pauses.

| Slide | Topic | Target | Finish by |
| --- | --- | --- | --- |
| 1 | GPU image rotation and responsible HPC for AI | 10 s | 0:10 |
| 2 | Host–GPU image data flow | 30 s | 0:40 |
| 3 | Rotation as independent output work | 35 s | 1:15 |
| 4 | CUDA maps pixels to scheduled threads | 40 s | 1:55 |
| 5 | Throughput gains depend on the bottleneck | 30 s | 2:25 |
| 6 | GPUDirect Storage and rotation | 30 s | 2:55 |
| 7 | Electricity demand and environmental costs | 55 s | 3:50 |
| 8 | Privacy, security and model fairness | 55 s | 4:45 |
| 9 | Compute concentration limits participation | 35 s | 5:20 |
| 10 | Approve scale with evidence and ownership | 40 s | 6:00 |
| 11 | Scale useful work, then verify the outcome | 20 s | 6:20 |

Follow the concepts and diagrams; do not read every equation, code line or citation aloud. Full code, calculations and source comparisons remain in the appendix.

## Slide 1: GPU image rotation and responsible HPC for AI

We address GPU image rotation and responsible HPC scaling. The appendix retains our full implementation, research and calculations.

## Slide 2: Host–GPU image data flow

The CPU loads and decodes an image into host RAM, allocates separate device input and output buffers, and launches the GPU work. DMA transfers decoded pixels to GPU memory over PCIe. The kernel rotates the pixels, and completed output returns to host RAM for saving. GPU memory bandwidth is different from link bandwidth. Pinned RAM supports efficient transfer; pageable memory may require staging.

## Slide 3: Rotation as independent output work

Each thread owns one output pixel. Inverse rotation gathers the nearest source pixel, avoiding scatter holes. With y pointing down, positive angles are visually counterclockwise. Copy three RGB channels, or write black when rounded source indices fall outside the image. Separate output locations require no atomics or barrier. The fixed canvas can crop corners and nearest-neighbour sampling can alias. Full kernels and a worked example remain in the appendix.

## Slide 4: CUDA maps pixels to scheduled threads

The CPU launches a grid of blocks; each block contains threads. A sixteen-by-sixteen block has two hundred and fifty-six threads, or eight warps. For a one-thousand-and-twenty-four-square image, the grid is sixty-four by sixty-four blocks. Each thread computes its global pixel coordinate and checks the image boundary. Streaming multiprocessors schedule resident warps using SIMT. The brightness excerpt shows a second independent pixel operation with clamping. More resident warps can hide latency, but registers and divergence affect execution; these examples are uncompiled.

## Slide 5: Throughput gains depend on the bottleneck

Compare identical rotation against one CPU thread and a multithreaded CPU. GPU execution units and memory bandwidth favor parallel work. Coalesced output writes help; rotated gathers can limit bandwidth. Include setup, both transfers and completed kernel execution in timing. Streams can overlap work with supporting hardware and pinned buffers. Serial work and transfers bound gains. No measured speedup is claimed.

## Slide 6: GPUDirect Storage and rotation

Conventional storage reads stage bytes in host RAM. GPUDirect Storage can move bytes directly into GPU memory using storage-side DMA; the CPU still issues cuFile requests. GDS does not decode images or speed rotation arithmetic. Our CPU-decoded path needs host RAM. Consider supported storage-bound GPU-ready batches or compatible GPU decoding only after measuring completed workload benefits. Fallback may still stage through RAM.

## Slide 7: Electricity demand and environmental costs

Scaling a cluster increases both compute capacity and communication costs. Faster completion need not mean lower energy. The IEA estimates four hundred and eighty-five terawatt-hours of data-centre electricity in twenty twenty-five and projects nine hundred and fifty in twenty thirty. These are whole-sector figures, not one model’s footprint. The growth categories overlap and must not be added. Green AI emphasizes efficiency per useful outcome; water-footprint research adds place and time constraints. We propose measuring whole-job energy, carbon intensity and local cooling impacts, including tuning and failed runs. Hardware manufacture adds embodied impacts. Job-level equations, a faster-but-higher-energy example and research limits are in the appendix.

## Slide 8: Privacy, security and model fairness

Distributed training exchanges gradients and stores checkpoints across workers, increasing the assets and replicas to protect. Zhu and colleagues show input reconstruction from exposed gradients in studied settings; this is not proof that every production collective leaks. Use least privilege, isolation, protected transport and storage, plus provenance, permissions, deletion and dual-use review with named owners. Fairness is a separate concern. Gallegos and colleagues distinguish evaluation metrics, datasets and mitigation stages. Large compute enables evaluation but does not guarantee representative data or fair outputs. Reserve evaluation compute for affected groups, choose harm-specific criteria, test disaggregated outcomes and reevaluate after mitigation. Encryption does not establish fairness, and HPC itself is not claimed to cause bias.

## Slide 9: Compute concentration limits participation

Compute concentration affects who can train models and reproduce findings. Ahmed and Wahed examine historical institutional participation, while Strubell and colleagues examine costs of selected workloads; their scopes differ. NAIRR is a current US shared-resource response, but participation numbers do not prove worldwide equality. We propose published GPU-hour allocation rules, smaller-institution and replication capacity, training and appeals. Track waiting times and awards. Reserved access can reduce immediate utilization, so prevent resource starvation while balancing urgent work.

## Slide 10: Approve scale with evidence and ownership

Our proposal combines OECD rights and accountability, NIST risk management, Green AI measurement and carbon-aware scheduling. Govern assigns ownership throughout. Map defines the use, stakeholders and alternatives. Measure records quality, energy, security and group harms. Manage approves, defers, reduces scale or redesigns. Shift flexible jobs toward lower-carbon windows only when deadlines, water constraints and equitable waiting limits remain satisfied. An accountable owner records exceptions; forecast uncertainty and urgent work can override deferral. This is an iterative proposed policy, not a certification.

## Slide 11: Scale useful work, then verify the outcome

CUDA is promising for independent pixels, but performance needs measurement. Responsible HPC requires useful outcomes, protected data and equitable access. Next, compile and validate the rotation, then measure a real cluster job and revise decisions using evidence.
