# Task 2: Ethical implications of scaling HPC for AI

Four main slides, three minutes. Environmental impact is the detailed focus.

## Slide 1: HPC, parallel computing and AI (40 seconds)

We've seen how one GPU divides an image into parallel work. HPC takes that idea across many processors, with fast networks and storage. In data-parallel AI training, each GPU holds the same model but processes different examples. They synchronise gradients before updating it. Larger models can also split layers across GPUs. This increases capacity, but communication means doubling the GPUs won't necessarily halve the runtime. The ethical question is who benefits from that scale, and who pays for it.

Sources: [11].

## Slide 2: Electricity demand and environmental risk (65 seconds)

The environmental impact is our main focus. According to the IEA's 2026 report, all data centres used an estimated four hundred and eighty-five terawatt-hours in 2025. Its 2030 projection is nine hundred and fifty: roughly ninety-six percent more. That's the whole sector, including AI and other workloads, and the future bar is a forecast. On the right, electricity use grew seventeen percent across all data centres in 2025, versus fifty percent for the AI-focused subset. Those groups overlap, so we can't add the rates. Efficiency per task can improve while total demand grows, because usage grows faster. The actual harm also depends on the electricity source and local cooling-water pressure. A faster training run alone doesn't tell us its total energy or emissions.

Sources: [12], [13], [18].

## Slide 3: Data governance and access to compute (35 seconds)

Scaling also raises privacy, fairness and access questions. Research has shown that exposed gradients can reveal training data under some conditions. So distributing work doesn't automatically protect privacy, and more compute doesn't remove dataset bias. Expensive clusters also shape who can do research. Historical compute-divide research describes that concentration; NAIRR offers a current US response, supporting over six hundred teams and six thousand students. Shared capacity helps, but doesn't prove equal access worldwide.

Sources: [14], [15], [16], [20], [21], [22].

## Slide 4: Responsible HPC policies (40 seconds)

Our proposal combines performance targets with an energy budget. Green AI argues that efficiency should be evaluated alongside accuracy. Carbon-aware scheduling can move flexible jobs to cleaner periods, although urgent jobs can't always wait. OECD principles and NIST's risk framework guide privacy, fairness and accountability. We would also publish clear allocation rules. Next, measure a real training job's energy, scaling efficiency and useful output. These charts show sector trends, not that job's footprint. Responsible HPC means checking benefits against resource costs and who bears them. Thank you; we're happy to take questions.

Sources: [14], [15], [16], [17], [18], [19].

## Study explanations

### Connection to parallel computing

An HPC system combines processors, memory, storage and interconnects. Parallel algorithms divide work across these resources. In the data-parallel diagram, each GPU processes a different batch partition using the same model. Gradient synchronisation communicates results before a consistent update. Tensor parallelism divides work within layers; pipeline parallelism assigns different layers to stages. Scaling increases useful capacity only if computation, memory and communication remain balanced. The diagram simplifies training and does not claim a measured scaling efficiency. [11]

### Topic (a): environmental and energy impact, explained in detail

The main evidence is the IEA's 2026 update, which supersedes the older 2025 forecast often quoted as 415 TWh in 2024 and 945 TWh in 2030. This presentation uses one consistent 2026 source: 485 TWh in 2025 and a central projection of 950 TWh in 2030. It treats these as electricity consumption for the whole data-centre sector, not a measurement of all HPC or AI training alone. Source scope matters because data centres also support storage, conventional cloud workloads and inference. [12, 13]

### How to explain the two charts

The left chart compares annual electricity consumption in TWh. The right chart compares year-on-year growth rates during 2025. The AI-focused category overlaps the whole-sector category, so the 17% and 50% rates cannot be added or used to infer the AI share. A 50% growth rate on a subset does not mean that subset consumes 50% of the total. The left chart's increase is (950 - 485) / 485 * 100 = 95.876%, or approximately 96%. The compound annual rate implied across five years is (950 / 485)^(1/5) - 1 = approximately 14.39%, which is a derived endpoint rate, not five observed annual rates. The plotted future value depends on assumptions and bottlenecks.

### Why energy efficiency does not settle the environmental question

A faster or more efficient device can lower electricity per useful task while total demand increases through adoption, more tasks or more demanding models. Job time alone is also insufficient: energy integrates power over time. The same electricity use can imply different operational emissions depending on carbon intensity. A first-order estimate sums kWh used in each period multiplied by that period's kg CO2e/kWh. State whether accounting includes cooling, storage, networking, repeated experiments and failed runs. Operational electricity accounting does not include all embodied impacts of manufacturing infrastructure. [13, 17]

### Local environmental trade-offs

Large concentrated loads can stress local grid connections and trigger investment needs. These outcomes depend on the supply-demand balance and policy, so electricity prices do not automatically rise everywhere. Cooling and electricity generation can involve water withdrawal or consumption. Their impacts depend on the site, technology and season; those water measures are different and should not be conflated. A low-carbon location may still face local water stress. This is why an operator should assess local constraints as well as a global energy total. [13, 18]

### Topic (b): governance, security and fairness

A proposed cluster policy should document dataset provenance and permissions, control who can access replicas and checkpoints, and keep audit records. Assess whether training data represent relevant groups and compare outcomes across them. Larger datasets and faster training do not, by themselves, establish fairness. OECD principles provide the values; NIST AI RMF functions can organise responsibility and risk assessment. The suggested controls are this presentation's application of those frameworks, not a claim that they automatically certify an AI system. [14, 15]

### Topic (c): socio-economic and accessibility challenges

Buying or renting large accelerator clusters requires resources, and that can shape who runs research, reproduces results and audits systems. The US NAIRR initiative offers a documented shared-resource example. NSF reports more than 600 research teams and 6,000 students supported since 2024, as of its March 2026 update. These counts describe that programme and do not quantify worldwide inequality or universal access. Suggested measures include transparent allocations, support for smaller institutions and training for applicants. [16]

### Topic (d): proposed responsible HPC framework

Before approving a job, define its useful outcome, responsible owner and energy budget. Measure total energy, explain carbon accounting and assess privacy/fairness risks. Where feasible, schedule flexible workloads in lower-carbon periods using a forecast and a deadline constraint, following the approach demonstrated in primary research. Use transparent access rules and publish enough method information for scrutiny without exposing private datasets. Review outcomes and stop or redesign wasteful experiments. This proposed process combines OECD principles, NIST risk management, shared access and carbon-aware scheduling. [14-17]

### Limitations and future work

The talk has three minutes, so topic (a) receives the most detail while (b), (c) and (d) receive concise coverage. The global electricity figures are sector-level estimates/projections and do not establish a causal footprint for an individual model. The presentation uses no invented training statistics or local benchmark results. A future extension could measure a representative training job's wall time, whole-system energy, network activity and accuracy at several GPU counts, then report both scaling efficiency and cost per useful outcome.

### Topic (b): compare privacy mechanisms with fairness evaluation

Zhu et al. demonstrate reconstruction from exposed gradients in experimental distributed-learning settings. This connects directly to the gradient-synchronisation diagram: protection must cover intermediate updates and checkpoints as well as raw datasets. It does not establish that every production cluster exposes readable gradients. Gallegos et al. distinguish fairness metrics, evaluation datasets and mitigation stages; access control alone therefore cannot establish fairness. Our proposed response combines protected communication and least-privilege access with evaluation of harms across relevant groups. The papers concern different failure mechanisms, so neither solution substitutes for the other. [21, 22]

### Topic (c): compare evidence of concentration with a shared-access response

Ahmed and Wahed analyse historical research participation and argue that access to compute contributed to concentration among firms and elite institutions. Their preprint supports a mechanism rather than a current worldwide inequality percentage. NSF reports NAIRR participation in 2026, demonstrating that public shared capacity reaches research teams and students. These sources measure different things: conference participation versus programme reach. The comparison motivates transparent allocation and support for smaller institutions, but does not establish that a programme has eliminated the compute divide. [16, 20]

### Topics (a) and (d): compare measurement, scheduling and governance

Green AI proposes making efficiency and financial cost evaluation criteria alongside accuracy. Carbon-aware computing provides an operational example of moving flexible work using forecasts while preserving capacity and deadline constraints. NIST and OECD address responsibility, risk and human values at a wider level. Together they answer what to measure, how to schedule and who is accountable. They do not certify sustainability automatically: lower-carbon electricity may still coincide with local water stress, and delaying work can conflict with urgent tasks. These are our reasoned comparisons of the sources. [14, 15, 17, 18, 19]

## Q&A

### Is HPC the same thing as a GPU?

HPC is a system and workload approach. A GPU is one processor type. An HPC cluster can use CPUs and accelerators with networking and storage. Parallel computing is the method of distributing its work.

### Why does doubling GPU count not necessarily halve runtime?

Fixed work, synchronisation, communication, load imbalance and memory limits reduce ideal scaling. Explain the gradient-synchronisation stage in the diagram, rather than claiming a specific efficiency without measurements.

### Do 485 and 950 TWh measure AI training alone?

No. They cover global electricity use by all data centres, including AI and non-AI work. The 2025 figure is the IEA's estimate and the 2030 figure is a projection. There is no training-only claim in the charts.

### Does 50% growth mean AI uses half of data-centre electricity?

No. It is growth within the AI-focused subset relative to its own preceding-year baseline. It is not a share of the whole sector and cannot be added to the whole-sector 17% growth rate.

### Why can total demand grow while energy per task falls?

The number of tasks can grow faster than energy per task falls, and the mix can shift towards more demanding tasks. This is an explanation consistent with the IEA discussion, not proof that every efficiency measure causes rebound.

### What is a concrete mitigation and its trade-off?

Schedule flexible jobs in lower-carbon time windows while meeting a deadline. This can lower electricity-related emissions, but forecasts are uncertain, urgent jobs cannot always wait, and low carbon does not guarantee low water impact. Measure the actual job outcome.

### Why cover all four topics when the guidance says focus on a few?

The written specification and rubric require all four. The presentation meets that by discussing environmental impacts in detail and treating the overlapping governance, access and framework topics concisely.

### How do you connect privacy risk directly to parallel training?

Data-parallel GPUs exchange gradients. Deep Leakage from Gradients demonstrated reconstruction under some exposed-gradient conditions. Therefore protect gradients, checkpoints and communication as well as datasets; do not claim every gradient exchange necessarily leaks data. Fairness is a separate evaluation problem. [21, 22]

## References

[11] NVIDIA, Megatron Core Parallelism Strategies Guide. 2026 documentation. https://docs.nvidia.com/megatron-core/developer-guide/latest/user-guide/parallelism-guide.html. Accessed 6 October 2026. Primary technical documentation for data, tensor and pipeline parallelism and communication considerations.

[12] IEA, Key Questions on Energy and AI, Executive summary. 2026. https://www.iea.org/reports/key-questions-on-energy-and-ai/executive-summary. Accessed 6 October 2026. Published 16 April 2026. Reports 485 TWh in 2025, projects 950 TWh in 2030, and gives 17% and 50% electricity-demand growth figures for 2025. CC BY 4.0.

[13] IEA, Data centre electricity use surged in 2025. 2026. https://www.iea.org/news/data-centre-electricity-use-surged-in-2025-even-with-tightening-bottlenecks-driving-a-scramble-for-solutions. Accessed 6 October 2026. 16 April 2026 press release. Explains efficiency, growing AI use, grid bottlenecks and affordability considerations.

[14] OECD, AI Principles. Updated 2024. https://www.oecd.org/en/topics/ai-principles.html. Accessed 6 October 2026. Fairness, privacy, transparency, robustness and accountability as policy goals.

[15] NIST, AI Risk Management Framework 1.0 Playbook. 2023 framework; current online playbook. https://airc.nist.gov/airmf-resources/playbook/. Accessed 6 October 2026. Voluntary suggested actions aligned with Govern, Map, Measure and Manage. The online resource notes that the framework is being updated.

[16] NSF, NAIRR at 2 years. 2026. https://www.nsf.gov/cise/updates/nairr-2-years-advancing-american-artificial-intelligence. Accessed 6 October 2026. 19 March 2026. More than 600 teams and 6,000 students supported since 2024. A US access initiative, not a global participation statistic.

[17] A. Radovanovic et al., Carbon-Aware Computing for Datacenters. 2021. https://arxiv.org/abs/2106.11750. Accessed 6 October 2026. Primary research describing Google's scheduling of temporally flexible workloads using carbon-intensity forecasts while preserving daily capacity.

[18] P. Li et al., Making AI Less Thirsty. 2023 preprint. https://arxiv.org/abs/2304.03271. Accessed 6 October 2026. Primary research on water withdrawal/consumption and the spatial and temporal variation of AI water footprints. Used qualitatively, without presenting its model-specific numerical estimates as universal measurements.

[19] R. Schwartz et al., Green AI. 2019 position paper. https://arxiv.org/abs/1907.10597. Accessed 6 October 2026. Proposes evaluating efficiency and reporting financial cost alongside accuracy. Use as a policy argument, not a current electricity forecast.

[20] N. Ahmed and M. Wahed, The De-democratization of AI. 2020 research preprint. https://arxiv.org/abs/2010.15581. Accessed 6 October 2026. Studies conference participation and the compute divide. Historical evidence from its dataset, not a 2026 global access census.

[21] I. O. Gallegos et al., Bias and Fairness in Large Language Models: A Survey. 2024 revised survey; Computational Linguistics. https://arxiv.org/abs/2309.00770. Accessed 6 October 2026. Separates fairness metrics, evaluation datasets and mitigation stages. Fairness must be evaluated for specific harms and groups.

[22] L. Zhu, Z. Liu and S. Han, Deep Leakage from Gradients. 2019 research paper. https://arxiv.org/abs/1906.08935. Accessed 6 October 2026. Demonstrates input reconstruction from shared gradients under experimental conditions. Does not show that every production all-reduce is compromised.

## AI declaration

I used OpenAI Codex on 6 October 2026 to research primary IEA, NVIDIA, OECD, NIST and NSF sources and original research papers, draft Task 2 explanations and speaker notes, create editable charts and a parallel-training diagram, and assemble the combined team presentation. The chart values come from cited sources and are not locally measured benchmarks. I must review and understand the material before presenting. The prior Task 1 guide contains the earlier preparation prompts. This guide records the additional prompts for the Task 2 preparation and publication stage. Any later AI-assisted work must also be declared and its prompts appended. The preparation was subsequently revised against the HD rubric with research comparisons for each topic, additional CUDA performance examples, and natural three-minute scripts for each presenter. Timing remains a rehearsal target.

Earlier prompts are recorded in the Task 1 companion. Submit both records.

### Additional prompt 1: GitHub destination and naming

```text
Are you able to push the changes to this git page and create a new folder named applied2 https://github.com/Shamle-T/FIT3143-Parallel-Computing dont include any refernces of codex in the naming when your creating a new folder. Let me know whether this is possible
```

The original message supplied the repository address as a Markdown link. Its displayed text is reproduced here.

### Additional prompt 2: Task 1 publication and Task 2 preparation

```text
using a detailed comit message push the current changes to git and also do we need to do any code for task 1?
once pushed do task 2 as well, and push the task 2 to git are required
```

The initial Task 2 context and the earlier preparation prompts are recorded in the Task 1 companion PDF.

### Additional prompt 3: HD review and natural spoken scripts

```text
1)once you have compelted task 2, make sure you have aimed for HD in rubric yeah? for both task 1 and task 2 - if not update both tasks to cover HD level in the rubric
2) create a 6 minute script for both me (task1) and (task2) for my firend , make sure it doesnt sound overly robotic since we will have to speak this out.
```

Latest preparation request, reproduced with its original wording.

### Timing clarification response

```text
Six minutes total: 3 minutes each
```

Selected user response to the clarification about total versus per-person duration.
