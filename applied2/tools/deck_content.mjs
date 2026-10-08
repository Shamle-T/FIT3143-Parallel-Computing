// Canonical visible content for all three editable assessment decks.
// Citations are numbered consistently across the combined and task decks.
export const students = [
  { name: 'Savin Vindiv De Alwis', id: '35221631', email: 'sdea0018@student.monash.edu' },
  { name: 'Shamle Thilaksiri', id: '35512075', email: 'wthi0003@student.monash.edu' },
];

export const refs = [
  { n: 1, text: 'NVIDIA, “Programming model,” CUDA Programming Guide, online documentation, accessed Oct. 8, 2026.', url: 'https://docs.nvidia.com/cuda/cuda-programming-guide/01-introduction/programming-model.html', short: 'NVIDIA CUDA programming model' },
  { n: 2, text: 'NVIDIA, CUDA C++ Best Practices Guide, online documentation, accessed Oct. 8, 2026.', url: 'https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html', short: 'NVIDIA CUDA Best Practices' },
  { n: 3, text: 'NVIDIA, GPUDirect Storage Overview Guide, online documentation, accessed Oct. 8, 2026.', url: 'https://docs.nvidia.com/gpudirect-storage/overview-guide/index.html', short: 'NVIDIA GDS overview' },
  { n: 4, text: 'NVIDIA, GPUDirect Storage Design Guide, online documentation, accessed Oct. 8, 2026.', url: 'https://docs.nvidia.com/gpudirect-storage/design-guide/index.html', short: 'NVIDIA GDS design' },
  { n: 5, text: 'NVIDIA, “Parallelism strategies guide,” Megatron Core, online documentation, accessed Oct. 8, 2026.', url: 'https://docs.nvidia.com/megatron-core/developer-guide/latest/user-guide/parallelism-guide.html', short: 'NVIDIA Megatron parallelism' },
  { n: 6, text: 'International Energy Agency, Key Questions on Energy and AI, executive summary, Apr. 16, 2026, CC BY 4.0, accessed Oct. 8, 2026.', url: 'https://www.iea.org/reports/key-questions-on-energy-and-ai/executive-summary', short: 'IEA 2026: sector estimates and projection' },
  { n: 7, text: 'U.S. National Science Foundation, “NAIRR at 2 years: Advancing American artificial intelligence innovation and leadership,” Mar. 19, 2026, accessed Oct. 8, 2026.', url: 'https://www.nsf.gov/cise/updates/nairr-2-years-advancing-american-artificial-intelligence', short: 'NSF 2026: US programme reach' },
  { n: 8, text: 'E. Strubell, A. Ganesh, and A. McCallum, “Energy and policy considerations for deep learning in NLP,” in Proc. ACL, 2019, pp. 3645–3650, doi: 10.18653/v1/P19-1355.', url: 'https://aclanthology.org/P19-1355/', short: 'Strubell et al. 2019: cost and equity' },
  { n: 9, text: 'P. Li, J. Yang, M. A. Islam, and S. Ren, “Making AI less ‘thirsty’: Uncovering and addressing the secret water footprint of AI models,” arXiv:2304.03271v5, Mar. 26, 2025; first submitted 2023.', url: 'https://arxiv.org/abs/2304.03271v5', short: 'Li et al. 2025 revision: water impacts' },
  { n: 10, text: 'R. Schwartz, J. Dodge, N. A. Smith, and O. Etzioni, “Green AI,” arXiv:1907.10597, 2019.', url: 'https://arxiv.org/abs/1907.10597', short: 'Schwartz et al. 2019: efficiency reporting' },
  { n: 11, text: 'L. Zhu, Z. Liu, and S. Han, “Deep leakage from gradients,” arXiv:1906.08935v2, 2019.', url: 'https://arxiv.org/abs/1906.08935v2', short: 'Zhu et al. 2019: exposed gradients' },
  { n: 12, text: 'I. O. Gallegos et al., “Bias and fairness in large language models: A survey,” Computational Linguistics, vol. 50, no. 3, pp. 1097–1179, 2024, doi: 10.1162/coli_a_00524.', url: 'https://aclanthology.org/2024.cl-3.8/', short: 'Gallegos et al. 2024: fairness evaluation' },
  { n: 13, text: 'N. Ahmed and M. Wahed, “The de-democratization of AI: Deep learning and the compute divide in artificial intelligence research,” arXiv:2010.15581v1, 2020.', url: 'https://arxiv.org/abs/2010.15581v1', short: 'Ahmed and Wahed 2020: compute divide' },
  { n: 14, text: 'A. Radovanovic et al., “Carbon-aware computing for datacenters,” arXiv:2106.11750v1, 2021.', url: 'https://arxiv.org/abs/2106.11750v1', short: 'Radovanovic et al. 2021: flexible scheduling' },
  { n: 15, text: 'OECD, “OECD AI Principles,” adopted 2019, updated 2024, accessed Oct. 8, 2026.', url: 'https://www.oecd.org/en/topics/ai-principles.html', short: 'OECD: rights, fairness and accountability' },
  { n: 16, text: 'NIST, AI Risk Management Framework 1.0 Playbook, framework 2023, current online resource, accessed Oct. 8, 2026.', url: 'https://airc.nist.gov/airmf-resources/playbook/', short: 'NIST: Govern, Map, Measure, Manage' },
  { n: 17, text: 'NVIDIA, “Rotate,” NPP Image Geometry Transforms, CUDA 13.0.3 archive documentation, accessed Oct. 8, 2026.', url: 'https://docs.nvidia.com/cuda/archive/13.0.3/npp/image_geometry_transforms.html#rotate', short: 'NVIDIA NPP: library alternative' },
];

const overviewSlides = [
  { id: 'cover', task: 'both', kind: 'cover', title: 'GPU image rotation\nand responsible HPC for AI', subtitle: 'FIT3143 • Applied 2 • Tasks 1 and 2', citations: [],
    takeaway: 'CUDA design and performance • Evidence, trade-offs and accountable use' },
  { id: 'transfer', task: 't1', kind: 'transfer', label: 'Task 1(a)', title: 'Host–GPU image data flow', citations: [1, 2],
    takeaway: 'The CPU orchestrates I/O and launches; the GPU rotates decoded pixels.' },
  { id: 'rotation', task: 't1', kind: 'rotation', label: 'Task 1(b)', title: 'Rotation as independent output work', citations: [1],
    takeaway: 'One thread owns one output pixel: gather a source value, then write once.' },
  { id: 'launch', task: 't1', kind: 'launch', label: 'Task 1(b)', title: 'CUDA launch and execution hierarchy', citations: [1],
    takeaway: 'A grid specifies logical work; finite SMs schedule its blocks and warps.' },
  { id: 'features', task: 't1', kind: 'features', label: 'Task 1(b)', title: 'CUDA features that affect throughput', citations: [1, 2],
    takeaway: 'Concurrency helps only when memory access and resource use support it.' },
  { id: 'speed', task: 't1', kind: 'speed', label: 'Task 1(b)', title: 'Expected speedup and its limits', citations: [2],
    takeaway: 'Expect gains for large or resident workloads; measure the whole pipeline.' },
  { id: 'gds', task: 't1', kind: 'gds', label: 'Task 1(c)', title: 'GPUDirect Storage and rotation', citations: [3, 4],
    takeaway: 'GDS can reduce storage staging; it does not accelerate rotation arithmetic.' },
  { id: 'hpc', task: 't2', kind: 'hpc', label: 'Task 2 • HPC and AI', title: 'Parallel training across a compute cluster', citations: [5, 2],
    takeaway: 'Scaling adds capacity and communication: ethical costs depend on useful work.' },
  { id: 'environment', task: 't2', kind: 'environment', label: 'Task 2(a)', title: 'Electricity demand and environmental costs', citations: [6, 9, 10],
    takeaway: 'Efficiency per task can improve while demand grows; site conditions matter.' },
  { id: 'governance', task: 't2', kind: 'governance', label: 'Task 2(b)', title: 'Privacy, security and model fairness', citations: [11, 12, 15, 16],
    takeaway: 'Protect distributed data and evaluate harms; more compute is not a fairness guarantee.' },
  { id: 'access', task: 't2', kind: 'access', label: 'Task 2(c)', title: 'Unequal access to HPC resources', citations: [7, 8, 13],
    takeaway: 'Shared capacity helps; allocation, expertise and reproducibility still need support.' },
  { id: 'framework', task: 't2', kind: 'framework', label: 'Task 2(d)', title: 'An accountable HPC approval process', citations: [10, 14, 15, 16],
    takeaway: 'Approve useful work with explicit budgets, safeguards and an accountable owner.' },
];

const detailSlides = [
  { id: 'scope', task: 'both', kind: 'columns', title: 'Supporting detail and limitations', label: 'Appendix • Reading guide', citations: [],
    left: { heading: 'Task 1: design evidence', body: [
      'Rotation assumptions, a worked example and full CUDA teaching excerpts are included on the following slides.',
      'Performance explanations use a stated CPU baseline and theoretical bounds. No measured speedup or compiled CUDA result is claimed.',
      'GDS is evaluated for the actual CPU-decoded pipeline and a conditional storage-bound batch alternative.',
    ] },
    right: { heading: 'Task 2: research and decisions', body: [
      'Each required topic has evidence, a comparison, proposed measures and limitations. A cluster scenario is illustrative, not an observed experiment.',
      'Sector electricity data do not establish a particular training job’s footprint. Historical research is distinguished from current programme reach.',
      'References and a brief AI use declaration are part of this presentation. There is no separate report dependency.',
    ] } },
  { id: 'semantics', task: 't1', kind: 'columns', title: 'Rotation coordinates and sampling', label: 'Appendix A1', citations: [17],
    left: { heading: 'Coordinate convention', body: [
      'Input/output: contiguous 8-bit RGB; positive width W and height H; x points right and y points down. Input and output buffers are distinct.',
      'Rotate around cx = (W − 1)/2 and cy = (H − 1)/2. Positive θ is visually counterclockwise; the host supplies c = cos θ and s = sin θ in radians.',
      'Forward image-coordinate rotation uses [c s; −s c]. Inverting it gives source offsets [c −s; s c] × output offsets, then restores the center.',
    ] },
    right: { heading: 'Deliberate design choices', body: [
      'Nearest-neighbour indices: ix = floor(sx + 0.5), iy = floor(sy + 0.5). Copy three channels if the rounded indices are in bounds; otherwise write black.',
      'The fixed W × H canvas can crop rotated corners. Nearest-neighbour sampling can alias; bilinear interpolation improves smoothness but adds source reads/arithmetic.',
      'Forward scatter can leave holes or competing writes after rounding. Inverse gathering assigns every output exactly one writer. NPP offers library rotation with explicit interpolation/ROI choices.',
    ] } },
  { id: 'worked', task: 't1', kind: 'worked', title: 'A 90-degree rotation example', label: 'Appendix A2', citations: [],
    takeaway: 'The matrix values are illustrative pixel identifiers, not channel intensities.' },
  { id: 'brightness-code', task: 't1', kind: 'sourcecode', source: 'brightness', title: 'Crafted CUDA example: RGB brightness', label: 'Appendix A3', citations: [1],
    explanation: ['Same operation on independent pixels: a data-parallel kernel.', 'Assume brightness in [−255,255]. Integer addition avoids uint8 wrap; clamp each channel to [0,255].', 'Adjacent threads touch adjacent pixel locations, providing useful locality.', 'Illustrative CUDA C++; uncompiled and unbenchmarked.'] },
  { id: 'rotate-code-1', task: 't1', kind: 'sourcecode', source: 'rotation1', title: 'Crafted CUDA example: inverse mapping', label: 'Appendix A4', citations: [1],
    explanation: ['Global coordinates come from block/thread indices.', 'The guard handles partial edge blocks before any memory access.', 'The host computes sine/cosine once per angle.', 'Nearest-neighbour mapping uses the documented y-down convention.'] },
  { id: 'rotate-code-2', task: 't1', kind: 'sourcecode', source: 'rotation2', title: 'Crafted CUDA example: safe output writes', label: 'Appendix A5', citations: [1],
    explanation: ['RGB address = 3 × (row × width + column).', 'Every valid output gets three channels, including black borders.', 'No atomics, mutex or block barrier is needed: writers have disjoint destinations.', 'Total work Θ(W × H); input/output storage Θ(W × H) each.'] },
  { id: 'host-code', task: 't1', kind: 'codecolumns', title: 'Host orchestration and completion', label: 'Appendix A6', citations: [1, 2],
    code: 'dim3 block(16, 16);\ndim3 grid(1u + (unsigned(W)-1u)/block.x,\n          1u + (unsigned(H)-1u)/block.y);\ncudaMemcpy(d_in, h_in, bytes,\n           cudaMemcpyHostToDevice);\nrotate_rgb<<<grid, block>>>(\n    d_in, d_out, W, H, c, s);\ncudaGetLastError();\ncudaMemcpy(h_out, d_out, bytes,\n           cudaMemcpyDeviceToHost);',
    explanation: [
      'Illustrative excerpt, not a complete program: W/H > 0; bytes = 3 × W × H checked in size_t; valid separate buffers; grid/device limits validated.',
      'CPU loads/decodes, allocates host/device buffers, checks every API/launch/completion result, saves output and frees resources safely.',
      'For this ordered synchronous-copy example, D2H supplies completed host data. A launch is asynchronous; launch-error checks alone do not establish successful completion.',
      'Pinned host storage is page-locked and supports efficient DMA. Pageable memory may need runtime staging. Avoid excessive pinning.',
    ] },
  { id: 'shape-table', task: 't1', kind: 'table', title: 'Launch shape and memory access', label: 'Appendix A7', citations: [1, 2],
    columns: ['Choice', 'Mechanism', 'Performance implication'],
    rows: [
      ['16 × 16 block', '256 threads; each warp spans two 16-wide rows', 'Reasonable 2D starting point; row edges/alignment matter.'],
      ['32 × 8 block', '256 threads; each warp spans one 32-wide row', 'May improve row-oriented access; no universal winner.'],
      ['Output gathering', 'Adjacent x threads write adjacent RGB pixels', 'Useful write locality; per-channel byte operations still require efficient transactions.'],
      ['Rotated input reads', 'Angle changes the source addresses visited by a warp', 'Irregular gathers can reduce effective read bandwidth/cache reuse.'],
      ['Higher residency', 'Registers, shared memory and block/thread limits constrain active warps', 'Helps hide latency; maximum occupancy is not automatically fastest.'],
    ],
    note: 'Recompute grid for each block shape. Profile actual transactions, latency and registers before selecting a shape.' },
  { id: 'async-code', task: 't1', kind: 'sourcecode', source: 'async', title: 'Asynchronous work within one stream', label: 'Appendix A8', citations: [2],
    explanation: ['Full enqueue function; rotation_grid uses the ceil-division shown in A6.', 'Pinned host buffers and checked sizes are preconditions.', 'Success means submitted work; wait before reading, reusing or freeing buffers.', 'On error, previously enqueued work may still be pending.'] },
  { id: 'overlap', task: 't1', kind: 'overlap', title: 'Batch overlap and GPU residency', label: 'Appendix A9', citations: [2],
    takeaway: 'Overlap changes the critical path, not the amount of rotation work.' },
  { id: 'timing', task: 't1', kind: 'columns', title: 'Fair speedup comparison and profiling', label: 'Appendix A10', citations: [2],
    left: { heading: 'Comparable baselines', body: [
      'Primary baseline: one CPU thread performs the same RGB inverse mapping, nearest-neighbour rule and fixed canvas. Also compare a multithreaded CPU for a stronger practical baseline.',
      'Use identical inputs/angles and verify identical or justified numerically equivalent outputs. Include file decoding/I/O in both paths or exclude it from both.',
      'For the stated decoded-buffer comparison, include GPU allocation, H2D, launch, completed kernel and D2H; CPU timing covers the corresponding output allocation and rotation.',
    ] },
    right: { heading: 'Measure the bottleneck', body: [
      'Report cold-start and warmed repeated runs separately. Use CUDA events for kernel elapsed time after completion and a host timer through final completion for total latency.',
      'Vary image size, angle, block shape and batch size; record hardware, transfer mode, register use and effective memory/link bandwidth.',
      'The sequential sum does not apply unchanged to overlapping streams: measure the completed batch critical path. Lower kernel time alone cannot establish end-to-end speedup.',
    ] } },
  { id: 'gds-detail', task: 't1', kind: 'columns', title: 'GDS decision for the proposed pipeline', label: 'Appendix A11', citations: [3, 4],
    left: { heading: 'Conventional decoded-image path', body: [
      'CPU decoding requires host-visible compressed bytes and produces pixels in RAM. Those pixels are copied to the GPU; the rotation kernel then runs.',
      'A single small image already in RAM gains little from bypassing a storage step that is no longer on its path.',
      'GDS does not decode an image, bypass an application’s CPU transformation, or make GPU memory access itself faster.',
    ] },
    right: { heading: 'Conditional batch alternative', body: [
      'Large storage-bound batches of GPU-ready pixels, or a compatible GPU decoding pipeline, may avoid host staging via cuFile and storage-side DMA.',
      'Verify the GPU, storage/filesystem, software and PCIe topology support the direct path. Compatibility fallback uses host staging and is not evidence of direct DMA.',
      'Adopt only if completed batch time/CPU overhead improve on the real workload; include extra buffers, decoding and storage bandwidth. Otherwise retain conventional copies.',
    ] } },
  { id: 'parallelism', task: 't2', kind: 'table', title: 'Parallelism, dependencies and scaling limits', label: 'Appendix B1', citations: [5, 2],
    columns: ['Model', 'Partition and communication', 'Scaling limitation'],
    rows: [
      ['Data parallel', 'Replicate model; partition batches; reduce/average gradients before a consistent update', 'Collective latency/bandwidth and stragglers.'],
      ['Tensor parallel', 'Partition computations within a layer; exchange partial results', 'Frequent inter-device communication.'],
      ['Pipeline parallel', 'Assign layers to stages; pass activations/gradients', 'Stage imbalance and pipeline bubbles.'],
      ['Fixed workload', 'More GPUs reduce only the parallelizable portion', 'Serial work and communication bound speedup.'],
      ['Larger workload', 'Scale capacity/model/data with added resources', 'A different claim from halving a fixed job’s runtime.'],
    ], note: 'All-reduce is a collective result-sharing concept, not a claim that every AI framework uses the exact classroom MPI code.' },
  { id: 'jobenergy', task: 't2', kind: 'jobenergy', title: 'A faster job can consume more energy', label: 'Appendix B2 • Illustrative calculation', citations: [8, 10, 2],
    takeaway: 'Compare energy and useful output as well as latency and GPU count.' },
  { id: 'envaccount', task: 't2', kind: 'columns', title: 'Environmental accounting and evidence limits', label: 'Appendix B3', citations: [6, 8, 9, 10],
    left: { heading: 'Define what is counted', body: [
      'Energy (kWh) integrates power (kW) over hours. Operational emissions ≈ Σ energy in each interval × that interval’s carbon intensity (kg CO₂e/kWh).',
      'Report accelerators, host CPUs, storage/networking and allocated cooling overhead; include failed runs and repeated tuning when evaluating a project.',
      'Hardware manufacture and disposal are embodied impacts beyond operational electricity. Water withdrawal and water consumption are different measures.',
    ] },
    right: { heading: 'Compare the evidence', body: [
      'Strubell et al. quantify costs for selected historical NLP training/development workloads; those values are not universal 2026 model footprints.',
      'Green AI motivates reporting efficiency alongside accuracy; Li et al. show why water varies with place/time. Carbon alone misses local water pressures.',
      'IEA sector totals provide context, not causal attribution to one model. Forecasts depend on demand and infrastructure assumptions; local grid/cooling effects differ by site.',
    ] } },
  { id: 'riskregister', task: 't2', kind: 'table', title: 'Governance controls and remaining risks', label: 'Appendix B4 • Proposed cluster policy', citations: [11, 15, 16],
    columns: ['Risk / stakeholder', 'Proposed control', 'Residual limitation / owner'],
    rows: [
      ['Dataset misuse / data subjects', 'Provenance, permission checks, retention limits and access logs', 'A licence alone may not justify every use; data steward reviews purpose.'],
      ['Gradient/checkpoint exposure / participants', 'Least privilege, tenant isolation, protected transport and secure checkpoint storage', 'Authorized endpoints can still see sensitive values; security owner models insider risk.'],
      ['Unsafe dual use / affected public', 'Authorized-use review, restricted access where justified and review of deployment risks', 'Controls cannot prevent every misuse; accountable owner weighs benefits/restrictions.'],
      ['Distributed replicas / operators', 'Track copies, deletion obligations and incident response across workers', 'Replication increases the number of assets to protect; operator audits recovery.'],
    ], note: 'These are proposed mitigations informed by research/frameworks, not a proof of privacy or a compliance certification.' },
  { id: 'fairnessdetail', task: 't2', kind: 'columns', title: 'Fairness evaluation with large-scale compute', label: 'Appendix B5 • Proposed example', citations: [12, 15, 16],
    left: { heading: 'Compute connection', body: [
      'An illustrative multilingual LLM service may have unequal performance or stereotyped outputs across language groups. Identify the actual harm before choosing a metric.',
      'A cluster enables larger datasets and repeated training, but neither data volume nor GPU count guarantees representative data or equitable outcomes.',
      'Reserve evaluation compute for affected groups; document dataset coverage and compare disaggregated error or harmful-output rates using a task-appropriate test.',
    ] },
    right: { heading: 'Evidence and decision', body: [
      'Gallegos et al. separate metrics, evaluation datasets and mitigation stages. A single aggregate accuracy score can hide a relevant subgroup harm.',
      'Specify justified acceptance criteria before training; examine uncertainty and small-group sample limits. Mitigate, reevaluate, and document unresolved harms.',
      'Security and fairness have different objectives: encrypted gradients do not establish fair outputs. The source supports model-bias analysis, not a claim that HPC itself causes bias.',
    ] } },
  { id: 'accessdetail', task: 't2', kind: 'columns', title: 'Access research and allocation choices', label: 'Appendix B6', citations: [7, 8, 13],
    left: { heading: 'Research comparison', body: [
      'Ahmed and Wahed study historical conference participation and a compute divide. Strubell et al. examine financial/resource barriers in selected NLP workloads.',
      'The first concerns institutional participation; the second concerns workload cost. Together they motivate access support, but neither is a current worldwide access census.',
      'NSF’s March 2026 participation figures describe a US shared-resource response. Programme reach is not proof of equal allocation, global access or eliminated inequality.',
    ] },
    right: { heading: 'Proposed shared-cluster policy', body: [
      'Publish eligibility, GPU-hour limits and allocation criteria; reserve some capacity for teaching, smaller institutions and independent replication.',
      'Provide application support and training; allow appeals. Track applicant-to-award outcomes and waiting times across institution groups with privacy safeguards.',
      'Trade-offs: reservations may reduce immediate utilization; queueing can delay urgent work; unrestricted access can conflict with security/budgets. Prevent repeated deferral or resource starvation.',
    ] } },
  { id: 'frameworkdetail', task: 't2', kind: 'table', title: 'Framework roles and accountable decisions', label: 'Appendix B7 • Proposed application', citations: [10, 14, 15, 16],
    columns: ['Function / evidence', 'Concrete action', 'Decision / responsible role'],
    rows: [
      ['Govern / OECD values + NIST', 'Assign ownership, data rules, access policy and escalation', 'Accountable owner accepts obligations and review process.'],
      ['Map / NIST', 'Define use, stakeholders, misuse risks, deadline and alternatives', 'Data/risk stewards establish the application-specific scope.'],
      ['Measure / Green AI + NIST', 'Track energy per useful outcome, model quality, group harms and scaling efficiency', 'Operator/evaluator provides evidence and uncertainty.'],
      ['Manage / scheduling research + NIST', 'Approve, defer, reduce scale or redesign; enforce safeguards and review outcomes', 'Owner authorizes exceptions or rejects unresolved risks.'],
    ], note: 'Our proposal combines measurement, scheduling and risk management. NIST Govern spans the other functions; risk management is iterative, not a certification.' },
  { id: 'schedule', task: 't2', kind: 'schedule', title: 'Carbon-aware scheduling under constraints', label: 'Appendix B8 • Illustrative policy', citations: [9, 14, 16],
    takeaway: 'Flexible jobs may move; urgent jobs, water constraints and access commitments can override deferral.' },
  { id: 'evidence', task: 't2', kind: 'table', title: 'Research comparisons across all four topics', label: 'Appendix B9', citations: [8, 9, 10, 11, 12, 13, 14],
    columns: ['Topic', 'Comparison', 'Interpretation / limit'],
    rows: [
      ['(a) Environment', 'Cost/efficiency research [8,10] versus water-footprint research [9]', 'Useful-output efficiency and local resource pressure need different measures.'],
      ['(b) Governance', 'Gradient reconstruction [11] versus fairness evaluation survey [12]', 'Data exposure and unequal outcomes are distinct failure mechanisms.'],
      ['(c) Access', 'Institutional participation [13] versus workload cost/equity [8]', 'Concentration and financial barriers reinforce a concern; scopes differ.'],
      ['(d) Frameworks', 'Efficiency-reporting argument [10] versus operational scheduling [14]', 'Measurement motivates action; scheduling depends on flexibility and forecasts.'],
    ], note: 'Current institutional evidence [6,7] and governance frameworks [15,16] complement the research; they do not replace its methodological limits.' },
  { id: 'conclusions', task: 'both', kind: 'columns', title: 'Conclusions and next measurements', label: 'Appendix • Synthesis', citations: [2, 3, 4, 10, 14, 16],
    left: { heading: 'GPU rotation', body: [
      'Inverse gathering exposes independent pixel work. SM scheduling, memory locality and transfer costs determine achieved performance.',
      'Choose a CPU baseline and compare completed equivalent pipelines. GDS is conditional on storage bottlenecks, a compatible data path and verified support.',
      'Next: compile the teaching examples, check output against a CPU reference and profile representative image sizes/angles/batches. No result is assumed.',
    ] },
    right: { heading: 'Responsible scaling', body: [
      'More compute can enable valuable research while concentrating access and increasing resource demand. Approve scale for useful outcomes, with safeguards.',
      'Combine job-level energy/resource evidence, separate security/fairness evaluation, transparent allocations and deadline-aware scheduling.',
      'Next: measure a real cluster job’s energy, quality, scaling efficiency and allocation outcomes; revise policy when evidence or conditions change.',
    ] } },
];

// Essential evidence is placed immediately beside the relevant question's answer.
const byId = new Map([...overviewSlides, ...detailSlides].map(d => [d.id, d]));
const coreRotation = {
  id: 'rotation-core', task: 't1', kind: 'sourcecode', label: 'Task 1(b) • Rotation implementation',
  title: 'CUDA rotation kernel', citations: [1],
  code: `__global__ void rotate_rgb(const unsigned char* in,
    unsigned char* out, int W, int H, float c, float s) {
  int x = blockIdx.x*blockDim.x + threadIdx.x;
  int y = blockIdx.y*blockDim.y + threadIdx.y;
  if (x >= W || y >= H) return;
  float cx = (W-1)*0.5f, cy = (H-1)*0.5f;
  int ix = int(floorf(c*(x-cx)-s*(y-cy)+cx+0.5f));
  int iy = int(floorf(s*(x-cx)+c*(y-cy)+cy+0.5f));
  size_t dst = 3*(size_t(y)*W + x);
  if (ix >= 0 && ix < W && iy >= 0 && iy < H) {
    size_t src = 3*(size_t(iy)*W + ix);
    for (int ch=0; ch<3; ++ch) out[dst+ch] = in[src+ch];
  } else {
    for (int ch=0; ch<3; ++ch) out[dst+ch] = 0;
  }
}`,
  explanation: [
    'One thread owns one output pixel. Guard partial edge blocks.',
    'Inverse gather copies RGB; out-of-bounds indices write black.',
    'Disjoint outputs need no atomics/barrier. Work and storage: O(W × H).',
    'Separate RGB buffers; W/H > 0; valid sizes, c/s. Uncompiled and unbenchmarked.',
  ],
};
const streamCore = {
  id: 'stream-core', task: 't1', kind: 'streamcore', label: 'Task 1(b) • Concurrency and speedup',
  title: 'Streams, overlap and completed output', citations: [2],
};
const mainOrder = [
  ['cover'], ['transfer'], ['rotation'], ['worked','Task 1(b) • Worked rotation'],
  ['launch'], ['rotation-core'], ['host-code','Task 1(b) • Host orchestration'],
  ['features'], ['shape-table','Task 1(b) • Memory access and launch shape'],
  ['speed'], ['stream-core'], ['gds'],
  ['hpc'], ['environment'], ['jobenergy','Task 2(a) • Illustrative energy calculation'],
  ['envaccount','Task 2(a) • Environmental accounting'],
  ['governance'], ['riskregister','Task 2(b) • Proposed governance controls'],
  ['fairnessdetail','Task 2(b) • Fairness with large-scale compute'],
  ['access'], ['accessdetail','Task 2(c) • Research and allocation decisions'],
  ['framework'], ['frameworkdetail','Task 2(d) • Framework actions and owners'],
  ['schedule','Task 2(d) • Carbon-aware scheduling'],
  ['conclusions','Tasks 1 and 2 • Conclusions and limitations'],
];
byId.set(coreRotation.id, coreRotation);
byId.set(streamCore.id, streamCore);
const preservedMainSlides = mainOrder.map(([id,label]) => ({...byId.get(id), ...(label ? {label} : {})}));
const movedIds = new Set(mainOrder.map(([id]) => id));
// The streams diagram is integrated into stream-core; no duplicate is needed.
movedIds.add('overlap');
const preservedAppendixSlides = detailSlides.filter(d => !movedIds.has(d.id)).map(d => {
  if (d.id === 'scope') return {...d, title:'Optional supporting material', left:{heading:'Task 1: implementation expansions',body:[
    'The Task 1 answers already contain the rotation formula, worked calculation, kernel, host launch, launch-shape comparison and stream/overlap example.',
    'The following slides expand coordinate conventions and complete source examples, then give an optional profiling plan and GDS background.',
  ]},right:{heading:'Task 2: supplementary comparisons',body:[
    'The Task 2 answers already contain energy/carbon calculations, research comparisons, governance and fairness decisions, access policy and accountable scheduling.',
    'Additional parallelism and cross-topic comparison tables follow. Website references and one brief AI-use declaration complete the deck.',
  ]}};
  const labels = {semantics:'Appendix A1', 'brightness-code':'Appendix A2', 'rotate-code-1':'Appendix A3', 'rotate-code-2':'Appendix A4', 'async-code':'Appendix A5', timing:'Appendix A6', 'gds-detail':'Appendix A7', parallelism:'Appendix B1', evidence:'Appendix B2'};
  if(d.id==='async-code') return {...d,label:labels[d.id],explanation:[
    'Full enqueue function; rotation_grid uses the ceil-division shown in the host orchestration answer.', ...d.explanation.slice(1),
  ]};
  return {...d,label:labels[d.id]??d.label};
});

// A timed presentation path; the previous detailed answers remain intact below.
const conciseSlides = [
  {id:'cuda-brief',task:'t1',kind:'cudabrief',label:'Task 1(b) • Execution model',title:'CUDA maps pixels to scheduled threads',citations:[1,2],
    takeaway:'A logical grid exposes work; SMs execute resident warps with SIMT.'},
  {id:'speed-brief',task:'t1',kind:'columns',label:'Task 1(b) • Expected speedup',title:'Throughput gains depend on the bottleneck',citations:[2],
    left:{heading:'Why a GPU can be faster',body:[
      'Many execution units and high device-memory bandwidth process independent pixels concurrently.',
      'Coalesced writes help; rotated gathers, registers and divergence can limit throughput.',
      'Large/resident images amortize setup. Streams can overlap transfers/compute with separate pinned buffers and supporting hardware.',
    ]},right:{heading:'Compare completed equivalent work',body:[
      'Speedup = TCPU / TGPU,total. Same rotation/sampling on one CPU thread; also test a multithreaded CPU.',
      'Sequential time: setup + H2D + kernel + D2H. For overlapped batches, measure the completed critical path; wait before reusing buffers.',
      'Amdahl: serial work bounds gains. No measured speedup; compile, verify and profile.',
    ]}},
  {id:'access-brief',task:'t2',kind:'columns',label:'Task 2(c) • Access and socioeconomic impact',title:'Compute concentration limits participation',citations:[7,8,13],
    left:{heading:'Evidence and consequences',body:[
      'Ahmed & Wahed [13]: institutional participation. Strubell et al. [8]: workload cost. Different scopes reveal barriers to training and replication.',
      'Historical studies do not establish current global access.',
      'NAIRR: >600 teams and >6,000 students [7]; US reach does not establish worldwide equality.',
    ]},right:{heading:'An equitable shared-cluster policy',body:[
      'Publish GPU-hour rules; reserve capacity for smaller institutions and replication.',
      'Offer training and appeals; audit waiting times and awards with privacy safeguards.',
      'Trade-off: reservations can lower utilization or delay urgent jobs. Prevent starvation.',
    ]}},
  {id:'framework-brief',task:'t2',kind:'frameworkbrief',label:'Task 2(d) • Responsible HPC framework',title:'Approve scale with evidence and ownership',citations:[10,14,15,16],
    takeaway:'NIST Govern spans iterative review; unacceptable risks require redesign or rejection.'},
  {id:'conclusions',task:'both',kind:'columns',label:'Tasks 1 and 2 • Conclusions',title:'Scale useful work, then verify the outcome',citations:[2,3,4,10,16],
    left:{heading:'Task 1: conditional performance',body:[
      'Independent inverse-gather pixels suit CUDA; locality, transfers and SM scheduling determine speedup.',
      'GDS helps only a compatible storage-bound path. Next: compile, compare CPU/GPU output and measure completed workloads.',
    ]},right:{heading:'Task 2: accountable scaling',body:[
      'Measure energy and useful output; protect data, evaluate group harms and allocate access transparently.',
      'Next: test a real cluster job and review outcomes. Sector projections and proposed policies are evidence limits, not measured results.',
    ]}},
];
const briefById=new Map(conciseSlides.map(d=>[d.id,d]));
const timedOrder=[['cover',10],['transfer',30],['rotation',35],['cuda-brief',40],['speed-brief',30],['gds',30],['environment',55],['governance',55],['access-brief',35],['framework-brief',40],['conclusions',20]];
const narrations={
  cover:'We address GPU image rotation and responsible HPC scaling. The appendix retains our full implementation, research and calculations.',
  transfer:'The CPU loads and decodes an image into host RAM, allocates separate device input and output buffers, and launches the GPU work. DMA transfers decoded pixels to GPU memory over PCIe. The kernel rotates the pixels, and completed output returns to host RAM for saving. GPU memory bandwidth is different from link bandwidth. Pinned RAM supports efficient transfer; pageable memory may require staging.',
  rotation:'Each thread owns one output pixel. Inverse rotation gathers the nearest source pixel, avoiding scatter holes. With y pointing down, positive angles are visually counterclockwise. Copy three RGB channels, or write black when rounded source indices fall outside the image. Separate output locations require no atomics or barrier. The fixed canvas can crop corners and nearest-neighbour sampling can alias. Full kernels and a worked example remain in the appendix.',
  'cuda-brief':'The CPU launches a grid of blocks; each block contains threads. A sixteen-by-sixteen block has two hundred and fifty-six threads, or eight warps. For a one-thousand-and-twenty-four-square image, the grid is sixty-four by sixty-four blocks. Each thread computes its global pixel coordinate and checks the image boundary. Streaming multiprocessors schedule resident warps using SIMT. The brightness excerpt shows a second independent pixel operation with clamping. More resident warps can hide latency, but registers and divergence affect execution; these examples are uncompiled.',
  'speed-brief':'Compare identical rotation against one CPU thread and a multithreaded CPU. GPU execution units and memory bandwidth favor parallel work. Coalesced output writes help; rotated gathers can limit bandwidth. Include setup, both transfers and completed kernel execution in timing. Streams can overlap work with supporting hardware and pinned buffers. Serial work and transfers bound gains. No measured speedup is claimed.',
  gds:'Conventional storage reads stage bytes in host RAM. GPUDirect Storage can move bytes directly into GPU memory using storage-side DMA; the CPU still issues cuFile requests. GDS does not decode images or speed rotation arithmetic. Our CPU-decoded path needs host RAM. Consider supported storage-bound GPU-ready batches or compatible GPU decoding only after measuring completed workload benefits. Fallback may still stage through RAM.',
  environment:'Scaling a cluster increases both compute capacity and communication costs. Faster completion need not mean lower energy. The IEA estimates four hundred and eighty-five terawatt-hours of data-centre electricity in twenty twenty-five and projects nine hundred and fifty in twenty thirty. These are whole-sector figures, not one model’s footprint. The growth categories overlap and must not be added. Green AI emphasizes efficiency per useful outcome; water-footprint research adds place and time constraints. We propose measuring whole-job energy, carbon intensity and local cooling impacts, including tuning and failed runs. Hardware manufacture adds embodied impacts. Job-level equations, a faster-but-higher-energy example and research limits are in the appendix.',
  governance:'Distributed training exchanges gradients and stores checkpoints across workers, increasing the assets and replicas to protect. Zhu and colleagues show input reconstruction from exposed gradients in studied settings; this is not proof that every production collective leaks. Use least privilege, isolation, protected transport and storage, plus provenance, permissions, deletion and dual-use review with named owners. Fairness is a separate concern. Gallegos and colleagues distinguish evaluation metrics, datasets and mitigation stages. Large compute enables evaluation but does not guarantee representative data or fair outputs. Reserve evaluation compute for affected groups, choose harm-specific criteria, test disaggregated outcomes and reevaluate after mitigation. Encryption does not establish fairness, and HPC itself is not claimed to cause bias.',
  'access-brief':'Compute concentration affects who can train models and reproduce findings. Ahmed and Wahed examine historical institutional participation, while Strubell and colleagues examine costs of selected workloads; their scopes differ. NAIRR is a current US shared-resource response, but participation numbers do not prove worldwide equality. We propose published GPU-hour allocation rules, smaller-institution and replication capacity, training and appeals. Track waiting times and awards. Reserved access can reduce immediate utilization, so prevent resource starvation while balancing urgent work.',
  'framework-brief':'Our proposal combines OECD rights and accountability, NIST risk management, Green AI measurement and carbon-aware scheduling. Govern assigns ownership throughout. Map defines the use, stakeholders and alternatives. Measure records quality, energy, security and group harms. Manage approves, defers, reduces scale or redesigns. Shift flexible jobs toward lower-carbon windows only when deadlines, water constraints and equitable waiting limits remain satisfied. An accountable owner records exceptions; forecast uncertainty and urgent work can override deferral. This is an iterative proposed policy, not a certification.',
  conclusions:'CUDA is promising for independent pixels, but performance needs measurement. Responsible HPC requires useful outcomes, protected data and equitable access. Next, compile and validate the rotation, then measure a real cluster job and revise decisions using evidence.',
};
export const mainSlides=timedOrder.map(([id,seconds])=>({...((briefById.get(id))??byId.get(id)),seconds,narration:narrations[id]}));
const retainedMainIds=new Set(mainSlides.filter(d=>!briefById.has(d.id)).map(d=>d.id));
const previousSlides=[...preservedMainSlides,...preservedAppendixSlides];
const appendixGuide={id:'appendix-guide',task:'both',kind:'columns',label:'Appendix • Not part of the seven-minute presentation',title:'Detailed answers and supporting evidence',citations:[],
  left:{heading:'Task 1: implementation and analysis',body:[
    'Complete rotation and brightness examples, thread/grid calculations, host launch and stream completion.',
    'Worked rotation, coordinate assumptions, launch-shape trade-offs, Amdahl bound, profiling and GPUDirect Storage decision.',
  ]},right:{heading:'Task 2: research and policy',body:[
    'Parallel training, energy/carbon calculations, environmental accounting and research limitations.',
    'Risk owners, fairness example, allocation policy, framework roles, carbon-aware scheduling and cross-topic comparisons.',
  ]}};
export const appendixSlides=[appendixGuide,...previousSlides.filter(d=>!retainedMainIds.has(d.id)).map(d=>({
  ...d,id:d.id==='conclusions'?'conclusions-detail':d.id,
  label:`Appendix • ${d.task==='t1'?'Task 1':d.task==='t2'?'Task 2':'Supporting material'} • ${d.label??'Detail'}`,
}))];

// Prompt records are loaded from the saved Markdown records before those files
// are refreshed. Their original spelling is preserved by the build preparation.
export const continuationPrompts = [
  { date: '8 October 2026', title: 'Repository synchronization', text: 'pull the latest from the remote repo, push any local changes and merge, local changes arent important. main goal is to get the latest version of the "applied2" dir from the remote' },
  { date: '8 October 2026', title: 'Review request', text: '- review the content in the root level context folder (with an emphasis on week\'s 7-9).\n- Review the solutions to task 1 and task 2 in [applied2](applied2/) based on the specification [Applied #2 - Assessment Specification.pdf](<\\<applied2/assessment/Applied #2 - Assessment Specification.pdf\\>>) . Identify any key HD blockers in the solutions based on the rubric attached.\n- The solutions dont need to only use the theory in the context folder but where possible, they should.' },
  { date: '8 October 2026', title: 'Current preparation instructions', text: '# AGENTS.md instructions\n\n<INSTRUCTIONS>\n- Answer immediately without intro pleasantries, prompt repetitions, or polite filler.\n- Be concise. Avoid conversational fluff, over-explanation, and unsolicited summaries.\n- Rely strictly on verified facts. Never invent URLs, names, statistics, or quotes.\n- If an answer cannot be verified with absolute certainty, state "I do not know" instead of guessing.\n- For complex logic, perform brief step-by-step reasoning before delivering the final answer.\n</INSTRUCTIONS>' },
  { date: '8 October 2026', title: 'Recorded review goal', kind: 'goal', text: 'Review the solutions to task 1 and task 2 in [applied2](applied2/) based on the specification [Applied #2 - Assessment Specification.pdf](<applied2/assessment/Applied #2 - Assessment Specification.pdf>) . Identify any key HD blockers in the solutions based on the rubric attached. Dont stop until you are certain you have identified every HD blocker and created an action plan which you can later execute to update the solutions to fix all problems.' },
  { date: '8 October 2026', title: 'Revision and presentation request', text: 'Now, fix all the identified problems and HD blockers. Don\'t stop until you\'re confident that task 1 and task 2 have a high probability of achieving an HD as per the applied-to rubric. Also refer to teaching_team_guidance.md, which is in the applied2 directory. Also ignore the Q&A and "Presentation Quality" in the rubric as these will be done by me. You need to create the presentation but dont worry about presenting it. Once you are confident with the task 1 and 2 solutions create a presentation as per task 3 (again dont worry about task 3 part c). Ensure the presentation is professionally formatted and contains all key details as I will not be submitting any other report, all info must be present in the presentation.' },
  { date: '8 October 2026', title: 'Identification details supplied', text: 'Savin Vindiv De Alwis (35221631) - sdea0018@student.monash.edu\nShamle Thilaksiri  (35512075) - wthi0003@student.monash.edu' },
  { date: '8 October 2026', title: 'Continuation request', text: 'continue where you left off' },
];
