# Presentation script

Present slides 1–13 only. Savin presents slides 1–6, then Shamily presents slides 7–13. The appendix starts at slide 14 and is outside this script.

Aim for about six minutes, including the handover and pauses to point at diagrams and graphs. Times below are rehearsal targets. Say MPI as letters and O(N) as “O of N”.

## Slide 1 — Savin · 0:00–0:15

Hi, I'm Savin, and this is Shamily. Our project simulates an EV charging network using MPI between stations and OpenMP within each station. We compare three implementations and measure how their communication costs change as the network grows.

## Slide 2 — Savin · 0:15–0:50

Each charging station is one MPI process, while world rank zero is the base station. Stations form a Cartesian grid with up to four neighbours and no wraparound.

Within each station, one persistent OpenMP thread manages each charging port. Each thread updates its own port, and barriers make sure the complete snapshot is ready before we count free ports. Only the initial thread calls MPI, using MPI_THREAD_FUNNELED.

## Slide 3 — Savin · 0:50–1:25

Each cycle starts with stations updating their ports and exchanging query intentions. If a station has fewer free ports than its threshold, it checks whether a neighbour has capacity.

If none does, it reports to the base and waits for advice. The report includes its location, discovery time, capacities and communication counts. Every neighbour follows a matching exchange phase, even when it has no query, so the communication stays consistent.

## Slide 4 — Savin · 1:25–1:55

The base receives the full alert batch before updating its cache and choosing candidates. Here, stations two, four and six are all at Manhattan distance two from station zero.

We return every surviving tie. Recent alerts or low-capacity observations exclude candidates, and old evidence expires. Missing evidence only lets us infer availability. It doesn't guarantee a free charging port.

## Slide 5 — Savin · 1:55–2:30

All three versions make the same routing decisions. V0 uses blocking Sendrecv, rank-ordered base receives and a scan of every station.

V1 uses nonblocking neighbour exchanges and checks at most eight frontier positions. V2 also preposts base receives and batches replies.

Candidate-search work falls from O of N to O of one per alert. Waitall confirms completion and safe buffer reuse. Nonblocking calls allow concurrency, although useful overlap with computation wasn't measured.

## Slide 6 — Savin · 2:30–3:00

The local study tested thirty grids and thirty lines, with three versions and three launches each: five hundred and forty runs. Those containers share one host.

Separately, CAAS tested five grids and five lines across two allocated compute nodes. Each MPI_Wtime interval stays on one rank, avoiding differences between host clocks. These timings include operation costs and waiting.

Shamily will now explain the results.

## Slide 7 — Shamily · 3:00–3:25

In the local grid results, each station still has at most four neighbours as the network grows. However, total traffic and synchronization increase, putting more pressure on the shared host.

For V2, query time rises from about three point three milliseconds across the first five sizes to fifty-two point five across the last five. The irregular curves don't establish a simple linear relationship.

## Slide 8 — Shamily · 3:25–3:50

A line gives each station at most two neighbours, but it still uses global collectives and one base. Query times also rise with size.

Each neighbour message crosses one logical edge, and stations contact the base directly through MPI. The delays therefore don't mean every message travels along the entire line. Shared-host scheduling also affects these results.

## Slide 9 — Shamily · 3:50–4:10

These graphs show the control broadcast, alert-flag gather and shutdown acknowledgements. Coordination also contributes to the measured costs.

Broadcast and gather times include collective waiting. Acknowledgement time measures completion of the station's send. Since these operations do different jobs, we interpret their timings separately.

## Slide 10 — Shamily · 4:10–4:35

The barrier measures waiting for other stations to finish the round. The alert round trip measures report sending through advice receipt, including queueing and base processing.

We use round-trip time to compare responsiveness. With eager buffering, a send can complete before the receiver consumes the message, so a quick send alone doesn't mean advice arrived quickly.

## Slide 11 — Shamily · 4:35–5:10

On CAAS, V2's query phase was about one point eight times faster than V0 for grids and two point five times faster for lines. Its round-trip ratios were about one point zero nine and one point three.

V2 had the highest observed response score across those ten sizes. Three launches support a descriptive comparison, rather than a universal winner. The large candidate-search speedup measures isolated computation, not the whole program.

## Slide 12 — Shamily · 5:10–5:40

Expiry controls how long the base trusts congestion evidence. In these accelerated tests, zero and zero point one seconds each produced six recommendations. One and thirty seconds produced none.

Short expiry can recognise recovery sooner, but also forget congestion sooner. Our limits include synthetic arrivals, global barriers and inferred capacity. Next steps include longer controlled runs, portable reports and confirming capacity before routing vehicles.

## Slide 13 — Shamily · 5:40–6:00

Finally, Codex substantially assisted the code, validation, experiments and presentation preparation. We remain responsible for understanding and verifying the submission, and we'll deliver the assessment ourselves without AI.

That concludes the main presentation. The remaining slides contain supporting technical details and evidence.
