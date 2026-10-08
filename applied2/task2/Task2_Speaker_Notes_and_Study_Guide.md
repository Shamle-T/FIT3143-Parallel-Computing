# Task2: current presentation companion

Generated from the current combined presentation on 8 October 2026.
All assessment explanations, code excerpts, research comparisons, references and a brief AI declaration are visible in the presentation; this companion is optional.

The current timed presentation path is in ../Presentation_Timing_and_Notes.md and in the PowerPoint speaker notes. Q&A remains outside this revision.

## Combined slide 1: GPU image rotation and responsible HPC for AI

FIT3143 • Applied 2 • Tasks 1 and 2
GPU image rotation
and responsible HPC for AI
CUDA design and performance • Evidence, trade-offs and accountable use
Savin Vindiv De Alwis · 35221631
sdea0018@student.monash.edu
Shamle Thilaksiri · 35512075
wthi0003@student.monash.edu
8 October 2026 • Focused answers first; full implementation and evidence in the appendix

## Combined slide 7: Electricity demand and environmental costs

Task 2(a)
Electricity demand and environmental costs
Global data-centre electricity (TWh/year)
Electricity growth during 2025
IEA 2026: ≈96% increase to 2030. These cover the whole sector, not AI training alone; growth categories overlap and
cannot be added.
Research comparison: Green AI [10] emphasizes efficiency; water research [9] adds site/time constraints. Track energy,
carbon and local cooling impacts.
Sources: [6] IEA 2026: sector estimates and projection; [9] Li et al. 2025 revision: water impacts; [10] Schwartz et al. 2019: efficiency reporting
7 / 46

Chart values: 2025 estimate 485 TWh/year; 2030 projection 950 TWh/year. 2025 growth: all data centres 17%; AI-focused subset 50%. Source [6].

## Combined slide 8: Privacy, security and model fairness

Task 2(b)
Privacy, security and model fairness
Security: protect distributed intermediates
Fairness: evaluate affected groups
Workers exchange gradients
and save checkpoints
Least privilege, isolation,
protected transport/storage
Data coverage and model outputs
may differ across groups
Disaggregated, harm-specific tests
+ mitigation and reevaluation
Zhu et al. [11]: exposed gradients can reconstruct
inputs in studied settings. Not every production
collective leaks.
Gallegos et al. [12]: metrics, datasets and mitigations
differ. Apply these to cluster-trained models; scale alone
does not guarantee fairness.
Provenance/permissions, replica access and dual-use review need owners. Encryption alone cannot establish fair outputs.
Protect distributed data and evaluate harms; more compute is not a fairness guarantee.
Sources: [11] Zhu et al. 2019: exposed gradients; [12] Gallegos et al. 2024: fairness evaluation; [15] OECD: rights, fairness and accountability; [16] NIST: Govern, Map, Measure,
Manage
8 / 46

## Combined slide 9: Compute concentration limits participation

Task 2(c) • Access and socioeconomic impact
Compute concentration limits participation
Evidence and consequences
Ahmed & Wahed [13]: institutional participation.
Strubell et al. [8]: workload cost. Different scopes
reveal barriers to training and replication.
Historical studies do not establish current global
access.
NAIRR: >600 teams and >6,000 students [7]; US
reach does not establish worldwide equality.
An equitable shared-cluster policy
Publish GPU-hour rules; reserve capacity for
smaller institutions and replication.
Offer training and appeals; audit waiting times and
awards with privacy safeguards.
Trade-off: reservations can lower utilization or delay
urgent jobs. Prevent starvation.
Sources: [7] NSF 2026: US programme reach; [8] Strubell et al. 2019: cost and equity; [13] Ahmed and Wahed 2020: compute divide
9 / 46

## Combined slide 10: Approve scale with evidence and ownership

Task 2(d) • Responsible HPC framework
Approve scale with evidence and ownership
Map
Use, stakeholders,
alternatives
Measure
Quality, energy, security,
harms
Manage
Approve / defer / redesign
Govern throughout: accountable owner, rights, data rules and equitable access
Green AI [10]: efficiency per useful outcome. OECD [15] + NIST [16]: rights, safeguards and review.
Carbon-aware scheduling [14]: shift flexible jobs within deadlines, water/site limits and equitable waiting
commitments.
Record exceptions and outcomes; urgent work and forecast uncertainty can override deferral. Prevent starvation.
NIST Govern spans iterative review; unacceptable risks require redesign or rejection.
Sources: [10] Schwartz et al. 2019: efficiency reporting; [14] Radovanovic et al. 2021: flexible scheduling; [15] OECD: rights, fairness and accountability; [16] NIST: Govern, Map,
Measure, Manage
10 / 46

## Combined slide 11: Scale useful work, then verify the outcome

Tasks 1 and 2 • Conclusions
Scale useful work, then verify the outcome
Task 1: conditional performance
Independent inverse-gather pixels suit CUDA;
locality, transfers and SM scheduling determine
speedup.
GDS helps only a compatible storage-bound path.
Next: compile, compare CPU/GPU output and
measure completed workloads.
Task 2: accountable scaling
Measure energy and useful output; protect data,
evaluate group harms and allocate access
transparently.
Next: test a real cluster job and review outcomes.
Sector projections and proposed policies are
evidence limits, not measured results.
Sources: [2] NVIDIA CUDA Best Practices; [3] NVIDIA GDS overview; [4] NVIDIA GDS design; [10] Schwartz et al. 2019: efficiency reporting; [16] NIST: Govern, Map, Measure,
Manage
11 / 46

## Combined slide 12: Detailed answers and supporting evidence

Appendix • Not part of the seven-minute presentation
Detailed answers and supporting evidence
Task 1: implementation and analysis
Complete rotation and brightness examples,
thread/grid calculations, host launch and stream
completion.
Worked rotation, coordinate assumptions,
launch-shape trade-offs, Amdahl bound, profiling
and GPUDirect Storage decision.
Task 2: research and policy
Parallel training, energy/carbon calculations,
environmental accounting and research limitations.
Risk owners, fairness example, allocation policy,
framework roles, carbon-aware scheduling and
cross-topic comparisons.
FIT3143 Applied 2
12 / 46

## Combined slide 21: Parallel training across a compute cluster

Appendix • Task 2 • Task 2 • HPC and AI
Parallel training across a compute cluster
GPU 0
same model
data partition A
GPU 1
same model
data partition B
GPU 2
same model
data partition C
Gradient all-reduce: average + share → consistent update on every GPU
Interconnect:
latency and bandwidth
Gradient synchronization
before each update
HPC combines processors, memory, networks and
storage. Tensor/pipeline parallelism can also split a
model.
More GPUs enable larger work; communication,
serial work and imbalance limit speedup. Evaluate
useful output against resource cost.
Scaling adds capacity and communication: ethical costs depend on useful work.
Sources: [5] NVIDIA Megatron parallelism; [2] NVIDIA CUDA Best Practices
21 / 46

## Combined slide 22: A faster job can consume more energy

Appendix • Task 2 • Task 2(a) • Illustrative energy calculation
A faster job can consume more energy
Assumed configuration
Completed runtime
Whole-job energy
Lower parallel scale; 1 kW
2.0 hours
1 × 2.0 = 2.0 kWh
Higher parallel scale; 2 kW
1.2 hours
2 × 1.2 = 2.4 kWh
Illustrative constant power assumptions, not measurements. Same useful output is assumed; faster completion uses 20% more
energy here.
E = ∫ P(t) dt • Carbon ≈ Σ Einterval × carbon intensity
Sources: [8] Strubell et al. 2019: cost and equity; [10] Schwartz et al. 2019: efficiency reporting; [2] NVIDIA CUDA Best Practices
22 / 46

## Combined slide 23: Environmental accounting and evidence limits

Appendix • Task 2 • Task 2(a) • Environmental accounting
Environmental accounting and evidence limits
Define what is counted
Energy (kWh) integrates power (kW) over hours.
Operational emissions ≈ Σ energy in each interval ×
that interval’s carbon intensity (kg CO₂e/kWh).
Report accelerators, host CPUs, storage/networking
and allocated cooling overhead; include failed runs
and repeated tuning when evaluating a project.
Hardware manufacture and disposal are embodied
impacts beyond operational electricity. Water
withdrawal and water consumption are different
measures.
Compare the evidence
Strubell et al. quantify costs for selected historical
NLP training/development workloads; those values
are not universal 2026 model footprints.
Green AI motivates reporting efficiency alongside
accuracy; Li et al. show why water varies with
place/time. Carbon alone misses local water
pressures.
IEA sector totals provide context, not causal
attribution to one model. Forecasts depend on
demand and infrastructure assumptions; local
grid/cooling effects differ by site.
Sources: [6] IEA 2026: sector estimates and projection; [8] Strubell et al. 2019: cost and equity; [9] Li et al. 2025 revision: water impacts; [10] Schwartz et al. 2019: efficiency
reporting
23 / 46

## Combined slide 24: Governance controls and remaining risks

Appendix • Task 2 • Task 2(b) • Proposed governance controls
Governance controls and remaining risks
Risk / stakeholder
Proposed control
Residual limitation / owner
Dataset misuse / data subjects
Provenance, permission checks, retention limits and access logs
A licence alone may not justify every use; data steward reviews purpose.
Gradient/checkpoint exposure / participants
Least privilege, tenant isolation, protected transport and secure checkpoint storage
Authorized endpoints can still see sensitive values; security owner models insider risk.
Unsafe dual use / affected public
Authorized-use review, restricted access where justified and review of deployment risks
Controls cannot prevent every misuse; accountable owner weighs benefits/restrictions.
Distributed replicas / operators
Track copies, deletion obligations and incident response across workers
Replication increases the number of assets to protect; operator audits recovery.
These are proposed mitigations informed by research/frameworks, not a proof of privacy or a compliance certification.
Sources: [11] Zhu et al. 2019: exposed gradients; [15] OECD: rights, fairness and accountability; [16] NIST: Govern, Map, Measure, Manage
24 / 46

## Combined slide 25: Fairness evaluation with large-scale compute

Appendix • Task 2 • Task 2(b) • Fairness with large-scale compute
Fairness evaluation with large-scale compute
Compute connection
An illustrative multilingual LLM service may have
unequal performance or stereotyped outputs across
language groups. Identify the actual harm before
choosing a metric.
A cluster enables larger datasets and repeated
training, but neither data volume nor GPU count
guarantees representative data or equitable
outcomes.
Reserve evaluation compute for affected groups;
document dataset coverage and compare
disaggregated error or harmful-output rates using a
task-appropriate test.
Evidence and decision
Gallegos et al. separate metrics, evaluation datasets
and mitigation stages. A single aggregate accuracy
score can hide a relevant subgroup harm.
Specify justified acceptance criteria before training;
examine uncertainty and small-group sample limits.
Mitigate, reevaluate, and document unresolved
harms.
Security and fairness have different objectives:
encrypted gradients do not establish fair outputs.
The source supports model-bias analysis, not a
claim that HPC itself causes bias.
Sources: [12] Gallegos et al. 2024: fairness evaluation; [15] OECD: rights, fairness and accountability; [16] NIST: Govern, Map, Measure, Manage
25 / 46

## Combined slide 26: Unequal access to HPC resources

Appendix • Task 2 • Task 2(c)
Unequal access to HPC resources
Evidence
What it shows
What it cannot establish
Ahmed & Wahed [13]
Historical research participation and compute concentration
Current worldwide access or a universal causal effect
Strubell et al. [8]
Selected NLP workload costs and financial barriers
The cost of every current AI workload
Current response: NAIRR supported >600 teams and >6,000 students
NSF, March 2026; US programme since 2024. Reach does not prove global equality.
Proposal: transparent GPU-hour allocation, smaller-institution support and appeals. Trade-off: reserved
access can reduce immediate utilization or delay urgent jobs.
Shared capacity helps; allocation, expertise and reproducibility still need support.
Sources: [7] NSF 2026: US programme reach; [8] Strubell et al. 2019: cost and equity; [13] Ahmed and Wahed 2020: compute divide
26 / 46

## Combined slide 27: Access research and allocation choices

Appendix • Task 2 • Task 2(c) • Research and allocation decisions
Access research and allocation choices
Research comparison
Ahmed and Wahed study historical conference
participation and a compute divide. Strubell et al.
examine financial/resource barriers in selected NLP
workloads.
The first concerns institutional participation; the
second concerns workload cost. Together they
motivate access support, but neither is a current
worldwide access census.
NSF’s March 2026 participation figures describe a
US shared-resource response. Programme reach is
not proof of equal allocation, global access or
eliminated inequality.
Proposed shared-cluster policy
Publish eligibility, GPU-hour limits and allocation
criteria; reserve some capacity for teaching, smaller
institutions and independent replication.
Provide application support and training; allow
appeals. Track applicant-to-award outcomes and
waiting times across institution groups with privacy
safeguards.
Trade-offs: reservations may reduce immediate
utilization; queueing can delay urgent work;
unrestricted access can conflict with
security/budgets. Prevent repeated deferral or
resource starvation.
Sources: [7] NSF 2026: US programme reach; [8] Strubell et al. 2019: cost and equity; [13] Ahmed and Wahed 2020: compute divide
27 / 46

## Combined slide 28: An accountable HPC approval process

Appendix • Task 2 • Task 2(d)
An accountable HPC approval process
Define outcome
+ accountable owner
Measure quality,
energy and harms
Approve / defer /
reduce scale / redesign
Green AI: efficiency evidence [10] • Carbon-aware scheduling: flexible work [14]
OECD: values and rights [15] • NIST: Govern, Map, Measure, Manage [16]
Use an energy budget, security/group evaluation and transparent access rules. Record owner, rationale,
exceptions and outcomes.
Measurement informs action: shift flexible jobs to cleaner periods within deadlines/capacity. Forecast
uncertainty, water stress and urgent access can change the decision.
Approve useful work with explicit budgets, safeguards and an accountable owner.
Sources: [10] Schwartz et al. 2019: efficiency reporting; [14] Radovanovic et al. 2021: flexible scheduling; [15] OECD: rights, fairness and accountability; [16] NIST: Govern, Map,
Measure, Manage
28 / 46

## Combined slide 29: Framework roles and accountable decisions

Appendix • Task 2 • Task 2(d) • Framework actions and owners
Framework roles and accountable decisions
Function / evidence
Concrete action
Decision / responsible role
Govern / OECD values + NIST
Assign ownership, data rules, access policy and escalation
Accountable owner accepts obligations and review process.
Map / NIST
Define use, stakeholders, misuse risks, deadline and alternatives
Data/risk stewards establish the application-specific scope.
Measure / Green AI + NIST
Track energy per useful outcome, model quality, group harms and scaling efficiency
Operator/evaluator provides evidence and uncertainty.
Manage / scheduling research + NIST
Approve, defer, reduce scale or redesign; enforce safeguards and review outcomes
Owner authorizes exceptions or rejects unresolved risks.
Our proposal combines measurement, scheduling and risk management. NIST Govern spans the other functions; risk
management is iterative, not a certification.
Sources: [10] Schwartz et al. 2019: efficiency reporting; [14] Radovanovic et al. 2021: flexible scheduling; [15] OECD: rights, fairness and accountability; [16] NIST: Govern, Map,
Measure, Manage
29 / 46

## Combined slide 30: Carbon-aware scheduling under constraints

Appendix • Task 2 • Task 2(d) • Carbon-aware scheduling
Carbon-aware scheduling under constraints
Flexible job
ready
Lower-carbon
forecast window
Complete before
deadline
Research [14] demonstrates shifting temporally flexible workloads using forecasts while preserving daily
capacity. It does not show every urgent GPU job can wait.
Proposed rule: defer only if deadline, energy budget, water/site limits and equitable waiting-time
commitments remain satisfied; record exceptions.
No feasible window: reduce scale, redesign or approve a justified urgent exception. Prevent indefinite
deferral. Data residency and transfer costs constrain moving work between sites.
Flexible jobs may move; urgent jobs, water constraints and access commitments can override deferral.
Sources: [9] Li et al. 2025 revision: water impacts; [14] Radovanovic et al. 2021: flexible scheduling; [16] NIST: Govern, Map, Measure, Manage
30 / 46

## Combined slide 31: Conclusions and next measurements

Appendix • Supporting material • Tasks 1 and 2 • Conclusions and limitations
Conclusions and next measurements
GPU rotation
Inverse gathering exposes independent pixel work.
SM scheduling, memory locality and transfer costs
determine achieved performance.
Choose a CPU baseline and compare completed
equivalent pipelines. GDS is conditional on storage
bottlenecks, a compatible data path and verified
support.
Next: compile the teaching examples, check output
against a CPU reference and profile representative
image sizes/angles/batches. No result is assumed.
Responsible scaling
More compute can enable valuable research while
concentrating access and increasing resource
demand. Approve scale for useful outcomes, with
safeguards.
Combine job-level energy/resource evidence,
separate security/fairness evaluation, transparent
allocations and deadline-aware scheduling.
Next: measure a real cluster job’s energy, quality,
scaling efficiency and allocation outcomes; revise
policy when evidence or conditions change.
Sources: [2] NVIDIA CUDA Best Practices; [3] NVIDIA GDS overview; [4] NVIDIA GDS design; [10] Schwartz et al. 2019: efficiency reporting; [14] Radovanovic et al. 2021: flexible
scheduling; [16] NIST: Govern, Map, Measure, Manage
31 / 46

## Combined slide 32: Optional supporting material

Appendix • Supporting material • Appendix • Reading guide
Optional supporting material
Task 1: implementation expansions
The Task 1 answers already contain the rotation
formula, worked calculation, kernel, host launch,
launch-shape comparison and stream/overlap
example.
The following slides expand coordinate conventions
and complete source examples, then give an
optional profiling plan and GDS background.
Task 2: supplementary comparisons
The Task 2 answers already contain energy/carbon
calculations, research comparisons, governance
and fairness decisions, access policy and
accountable scheduling.
Additional parallelism and cross-topic comparison
tables follow. Website references and one brief
AI-use declaration complete the deck.
FIT3143 Applied 2
32 / 46

## Combined slide 40: Parallelism, dependencies and scaling limits

Appendix • Task 2 • Appendix B1
Parallelism, dependencies and scaling limits
Model
Partition and communication
Scaling limitation
Data parallel
Replicate model; partition batches; reduce/average gradients before a consistent update
Collective latency/bandwidth and stragglers.
Tensor parallel
Partition computations within a layer; exchange partial results
Frequent inter-device communication.
Pipeline parallel
Assign layers to stages; pass activations/gradients
Stage imbalance and pipeline bubbles.
Fixed workload
More GPUs reduce only the parallelizable portion
Serial work and communication bound speedup.
Larger workload
Scale capacity/model/data with added resources
A different claim from halving a fixed job’s runtime.
All-reduce is a collective result-sharing concept, not a claim that every AI framework uses the exact classroom MPI code.
Sources: [5] NVIDIA Megatron parallelism; [2] NVIDIA CUDA Best Practices
40 / 46

## Combined slide 41: Research comparisons across all four topics

Appendix • Task 2 • Appendix B2
Research comparisons across all four topics
Topic
Comparison
Interpretation / limit
(a) Environment
Cost/efficiency research [8,10] versus water-footprint research [9]
Useful-output efficiency and local resource pressure need different measures.
(b) Governance
Gradient reconstruction [11] versus fairness evaluation survey [12]
Data exposure and unequal outcomes are distinct failure mechanisms.
(c) Access
Institutional participation [13] versus workload cost/equity [8]
Concentration and financial barriers reinforce a concern; scopes differ.
(d) Frameworks
Efficiency-reporting argument [10] versus operational scheduling [14]
Measurement motivates action; scheduling depends on flexibility and forecasts.
Current institutional evidence [6,7] and governance frameworks [15,16] complement the research; they do not replace its
methodological limits.
Sources: [8] Strubell et al. 2019: cost and equity; [9] Li et al. 2025 revision: water impacts; [10] Schwartz et al. 2019: efficiency reporting; [11] Zhu et al. 2019: exposed gradients; [12]
Gallegos et al. 2024: fairness evaluation; [13] Ahmed and Wahed 2020: compute divide; [14] Radovanovic et al. 2021: flexible scheduling
41 / 46

## Combined slide 42: References

Appendix • References 1/4
References
[1] NVIDIA, “Programming model,” CUDA Programming Guide, online documentation, accessed Oct. 8, 2026.
https://docs.nvidia.com/cuda/cuda-programming-guide/01-introduction/programming-model.html
[2] NVIDIA, CUDA C++ Best Practices Guide, online documentation, accessed Oct. 8, 2026.
https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html
[3] NVIDIA, GPUDirect Storage Overview Guide, online documentation, accessed Oct. 8, 2026.
https://docs.nvidia.com/gpudirect-storage/overview-guide/index.html
[4] NVIDIA, GPUDirect Storage Design Guide, online documentation, accessed Oct. 8, 2026.
https://docs.nvidia.com/gpudirect-storage/design-guide/index.html
FIT3143 Applied 2
42 / 46

## Combined slide 43: References

Appendix • References 2/4
References
[5] NVIDIA, “Parallelism strategies guide,” Megatron Core, online documentation, accessed Oct. 8, 2026.
https://docs.nvidia.com/megatron-core/developer-guide/latest/user-guide/parallelism-guide.html
[6] International Energy Agency, Key Questions on Energy and AI, executive summary, Apr. 16, 2026, CC BY 4.0, accessed
Oct. 8, 2026.
https://www.iea.org/reports/key-questions-on-energy-and-ai/executive-summary
[7] U.S. National Science Foundation, “NAIRR at 2 years: Advancing American artificial intelligence innovation and
leadership,” Mar. 19, 2026, accessed Oct. 8, 2026.
https://www.nsf.gov/cise/updates/nairr-2-years-advancing-american-artificial-intelligence
[8] E. Strubell, A. Ganesh, and A. McCallum, “Energy and policy considerations for deep learning in NLP,” in Proc. ACL,
2019, pp. 3645–3650, doi: 10.18653/v1/P19-1355.
https://aclanthology.org/P19-1355/
FIT3143 Applied 2
43 / 46

## Combined slide 44: References

Appendix • References 3/4
References
[9] P. Li, J. Yang, M. A. Islam, and S. Ren, “Making AI less ‘thirsty’: Uncovering and addressing the secret water footprint of
AI models,” arXiv:2304.03271v5, Mar. 26, 2025; first submitted 2023.
https://arxiv.org/abs/2304.03271v5
[10] R. Schwartz, J. Dodge, N. A. Smith, and O. Etzioni, “Green AI,” arXiv:1907.10597, 2019.
https://arxiv.org/abs/1907.10597
[11] L. Zhu, Z. Liu, and S. Han, “Deep leakage from gradients,” arXiv:1906.08935v2, 2019.
https://arxiv.org/abs/1906.08935v2
[12] I. O. Gallegos et al., “Bias and fairness in large language models: A survey,” Computational Linguistics, vol. 50, no. 3,
pp. 1097–1179, 2024, doi: 10.1162/coli_a_00524.
https://aclanthology.org/2024.cl-3.8/
FIT3143 Applied 2
44 / 46

## Combined slide 45: References

Appendix • References 4/4
References
[13] N. Ahmed and M. Wahed, “The de-democratization of AI: Deep learning and the compute divide in artificial intelligence
research,” arXiv:2010.15581v1, 2020.
https://arxiv.org/abs/2010.15581v1
[14] A. Radovanovic et al., “Carbon-aware computing for datacenters,” arXiv:2106.11750v1, 2021.
https://arxiv.org/abs/2106.11750v1
[15] OECD, “OECD AI Principles,” adopted 2019, updated 2024, accessed Oct. 8, 2026.
https://www.oecd.org/en/topics/ai-principles.html
[16] NIST, AI Risk Management Framework 1.0 Playbook, framework 2023, current online resource, accessed Oct. 8, 2026.
https://airc.nist.gov/airmf-resources/playbook/
[17] NVIDIA, “Rotate,” NPP Image Geometry Transforms, CUDA 13.0.3 archive documentation, accessed Oct. 8, 2026.
https://docs.nvidia.com/cuda/archive/13.0.3/npp/image_geometry_transforms.html#rotate
FIT3143 Applied 2
45 / 46

## Combined slide 46: AI use declaration

Appendix • AI declaration
AI use declaration
AI model and tool
OpenAI GPT-6, accessed through Codex.
How AI was used
Research and source checks; drafting and revision
of explanations and CUDA teaching examples;
creation of diagrams and charts; presentation
formatting and assembly.
FIT3143 Applied 2
46 / 46
