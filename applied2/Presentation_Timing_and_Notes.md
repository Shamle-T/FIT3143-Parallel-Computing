# Presentation timing and notes

Revised 9 October 2026. Present slides 1–11 and stop at the conclusion. Supporting code, calculations and full research comparisons are in the appendix starting at slide 12.

Target: 6:40, with 20 seconds available before seven minutes. The script contains 793 spoken words. At an assumed 130 words/minute, speech alone takes approximately 6.1 minutes. These are planning estimates, not measured rehearsal results.

Savin: slides 1–6 (3:00). Shamle: slides 7–11 (3:40). Speak the core explanation and the brief appendix pointers. Merely displaying an explanation does not establish spoken coverage.

| Slide | Topic | Target | Finish by |
| --- | --- | --- | --- |
| 1 | Introduction | 8 s | 0:08 |
| 2 | Moving the image to the GPU | 25 s | 0:33 |
| 3 | How the rotation works | 30 s | 1:03 |
| 4 | CUDA threads, blocks and grids | 42 s | 1:45 |
| 5 | Expected speedup | 45 s | 2:30 |
| 6 | Where GPUDirect Storage helps | 30 s | 3:00 |
| 7 | Energy and environmental costs | 60 s | 4:00 |
| 8 | Security, governance and fairness | 60 s | 5:00 |
| 9 | Who gets access to compute? | 35 s | 5:35 |
| 10 | Our responsible-use framework | 45 s | 6:20 |
| 11 | Conclusion | 20 s | 6:40 |

See [spoken coverage audit](HD_SPOKEN_COVERAGE.md) for the rubric-to-slide-and-script mapping.

## Slide 1: Introduction

We’re Savin and Shamle. We’ll cover GPU rotation and responsible HPC scaling.

## Slide 2: Moving the image to the GPU

The CPU decodes the image into host RAM and allocates GPU buffers. DMA copies the pixels over PCI Express; the GPU rotates them, and completed output returns for saving. Pinned RAM helps DMA; pageable RAM may need staging. Device bandwidth and transfer bandwidth are different. We wait before saving or reusing buffers.

## Slide 3: How the rotation works

Each thread owns one output pixel. We rotate backwards about the image centre, with the angle in radians, to find the nearest source pixel and copy its RGB values. Outside the image, we write black. Independent writes need no atomics or barrier. Positive angles are counterclockwise with our y-down coordinates. Gathering avoids holes, but a fixed canvas can crop corners and nearest-neighbour sampling can alias.

## Slide 4: CUDA threads, blocks and grids

The CPU launches a kernel with grid and block settings. Sixteen-by-sixteen blocks contain two hundred and fifty-six threads, or eight warps. This sixty-four-by-sixty-four grid describes about a million threads. They don’t all run at once: finite streaming multiprocessors schedule blocks in waves. SIMT means a warp follows common instructions, with thirty-two threads per warp. Threads compute pixel indices and guard image edges; other sizes need ceiling division. Launches are asynchronous, so check errors and completion. Full rotation, brightness, block-shape and stream examples are in the appendix.

## Slide 5: Expected speedup

Many GPU execution units and high memory bandwidth process independent pixels together. Coalescing combines nearby addresses into fewer transactions; rotated reads can limit bandwidth. Thirty-two-by-eight blocks keep a warp on one row, so we’d profile them against sixteen-by-sixteen. Resident warps hide latency, but registers and divergent branches limit execution. Large images amortize setup; keeping pixels on the GPU avoids repeat copies. Streams overlap independent images with separate pinned buffers and supporting hardware, but need more memory. Compare identical rotation against serial and multithreaded CPUs, timing setup, transfers and completed work. Serial work bounds speedup; no measured result is claimed.

## Slide 6: Where GPUDirect Storage helps

GPUDirect Storage uses storage-side DMA to bypass host staging; the CPU still issues cuFile requests. It doesn’t decode images or accelerate the rotation kernel. CPU-decoded or already-in-RAM pixels gain little from a storage bypass. GPU-ready, storage-bound batches may benefit, but GPU, filesystem, drivers and topology must support the direct path. Fallback may still stage through RAM. We’d measure completed batch time and CPU overhead. Shamle will cover responsible scaling.

## Slide 7: Energy and environmental costs

In data-parallel training, GPUs process different batches, and gradient all-reduce synchronizes updates. Communication and slow workers limit scaling. The IEA projects near-doubling of data-centre electricity by 2030, but that’s whole-sector context, not one model’s footprint. For a job, energy depends on power over time; carbon also depends on local carbon intensity. Faster can still use more energy. Count CPUs, networking, storage, cooling, tuning and failed runs, plus manufacturing impacts. Green AI measures efficiency per useful result; Li’s water research adds local resource pressure. Lower carbon alone can miss water costs. The appendix has the detailed calculation, growth charts and research comparison.

## Slide 8: Security, governance and fairness

Distributed replicas, gradients and checkpoints add assets to protect. Zhu’s study shows input reconstruction from exposed gradients in studied settings; it doesn’t prove every collective leaks. Use least privilege, isolation and protected transport and storage. Named owners check data provenance, permissions, replica retention and deletion, and dual-use risks. Fairness needs different controls. For a multilingual LLM, reserve compute to compare harmful-output rates across languages, examine uncertainty, improve underrepresented-language coverage and retest. Gallegos’s survey explains why metrics and datasets matter; more GPUs can’t fix unrepresentative data. Security protects intermediates; fairness tests outcomes. The full risk register and evaluation details are in the appendix.

## Slide 9: Who gets access to compute?

Compute access affects who can train, reproduce and shape AI research. Ahmed and Wahed study historical participation; Strubell examines workload costs. Their scopes differ, but both reveal barriers. NAIRR’s US reach doesn’t prove global equality. Publish GPU-hour rules, reserve smaller-institution and replication capacity, offer training and appeals, and audit awards and waiting times with privacy safeguards. Reservations can reduce utilization or delay urgent jobs; prevent starvation.

## Slide 10: Our responsible-use framework

OECD supplies rights and accountability; NIST structures risk decisions. Govern assigns ownership throughout. Map defines the use, affected people and alternatives. Measure checks quality, energy, security and group harms. Manage approves, defers, reduces scale or redesigns. Green AI tells us to measure efficiency; Radovanovic’s scheduling research shows how forecasts can shift flexible work. Measurement and action serve different needs. Set an energy budget and shift jobs only within deadlines, capacity, water limits and fair waiting commitments. Record urgent or forecast exceptions and review outcomes. This is an iterative proposal, not a certification; the detailed policy is in the appendix.

## Slide 11: Conclusion

CUDA suits independent pixels, but performance still needs measurement. Responsible scaling needs useful outcomes, protected data and fair access. Next, compile and validate the rotation, then measure a real cluster job and review its impacts. Our code examples are uncompiled and unbenchmarked.
