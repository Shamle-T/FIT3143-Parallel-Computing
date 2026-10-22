import fs from "node:fs/promises";
import path from "node:path";
import { pathToFileURL } from "node:url";
import { Presentation, PresentationFile } from "@oai/artifact-tool";

const workspaceDir = process.cwd();
const skillDir = "C:/Users/savin/.codex/plugins/cache/openai-primary-runtime/presentations/26.909.12148/skills/presentations";
const outputDir = path.join(workspaceDir, "output");
const finalPath = path.join(outputDir, "FIT3143_Week8_Lab2_Presentation.pptx");
const { resolvePresentationFont, finalizePresentation } = await import(pathToFileURL(path.join(skillDir, "container_tools/artifact_tool_utils.mjs")).href);
await fs.mkdir(outputDir, { recursive: true });
const font = resolvePresentationFont();
const deck = Presentation.create({ slideSize: { width: 1280, height: 720 } });

function text(slide, value, left, top, width, height, size = 22, color = "#18263A", bold = false) {
  const shape = slide.shapes.add({ geometry: "textbox", position: { left, top, width, height }, fill: "none", line: { fill: "none", width: 0 } });
  shape.text = value;
  shape.text.style = { typeface: font, fontSize: size, color, bold, autoFit: "shrinkText" };
  return shape;
}
function base(title) {
  const slide = deck.slides.add();
  slide.background.fill = "#F8FAFC";
  slide.shapes.add({ geometry: "rect", position: { left: 0, top: 0, width: 1280, height: 18 }, fill: "#005F73", line: { fill: "none", width: 0 } });
  text(slide, title, 58, 38, 1160, 50, 33, "#005F73", true);
  return slide;
}
async function image(slide, name, left = 110, top = 130, width = 1060, height = 500) {
  const bytes = await fs.readFile(path.join(workspaceDir, "results", "charts", name));
  slide.images.add({ blob: bytes, contentType: "image/png", alt: name, fit: "contain", position: { left, top, width, height } });
}

let s = deck.slides.add();
s.background.fill = "#005F73";
text(s, "FIT3143 Week 8 Lab 2", 70, 120, 1140, 72, 48, "#FFFFFF", true);
text(s, "Open MPI and Hybrid OpenMP Prime Search", 70, 210, 1080, 45, 30, "#D9F0EE");
text(s, "Savin Vindiv De Alwis (35221631)\nWillwara Arachchilage Shamle Imal Thilaksiri (35512075)", 70, 470, 900, 68, 20, "#FFFFFF");
text(s, "7-minute presentation", 70, 570, 500, 28, 18, "#D9F0EE");

s = base("Experimental environment and correctness");
text(s, "Single-node local Open MPI environment", 70, 130, 500, 35, 26, "#0A4C5A", true);
text(s, "• WSL2 Ubuntu, Open MPI 4.1.6\n• Intel Core i7-13700H: 14 physical cores, 20 logical CPUs\n• n = 60,000,000; 4 MPI ranks × 2 OpenMP threads\n• Median of three end-to-end runs for scaling results\n• End-to-end time includes communication, root merge, and file writing", 85, 190, 560, 240, 22);
text(s, "Independent output validation", 700, 130, 450, 35, 26, "#0A4C5A", true);
text(s, "Both Task 1 and Task 2 produced 3,562,115 primes.\nA separate sieve compared every output value against all expected primes below n.\n\nResult: sorted, unique, complete, and prime-only output.", 715, 190, 460, 235, 22);

s = base("Task 1 workload distribution");
text(s, "Odd candidates only. Rank r owns indices r, r+p, r+2p, …", 70, 120, 1130, 42, 25, "#0A4C5A", true);
text(s, "Cyclic ownership balances candidate magnitude as well as count. Each rank receives a mix of cheap composites and expensive large prime candidates. Rank 0 merges sorted rank lists in O(m log p) while writing the output.", 70, 180, 1130, 90, 22);
text(s, "Measured 4-rank balance at n = 60M", 70, 300, 530, 32, 24, "#0A4C5A", true);
text(s, "Rank 0: 6.418 s   Rank 1: 6.393 s\nRank 2: 6.385 s   Rank 3: 6.497 s\nCandidate-count range: 7,499,999–7,500,000\nMax/min computation-time ratio: 1.017", 85, 350, 560, 150, 23);
text(s, "Conclusion: cyclic partitioning is balanced for the non-uniform trial-division kernel.", 70, 565, 1120, 40, 24, "#B54708", true);

s = base("Task 1 runtime as n increases");
await image(s, "01_runtime_n.png");
s.speakerNotes.textFrame.setText("Source: results/n_scaling.csv and results/openmp_n_scaling.csv.");

s = base("Task 1 empirical speedup as n increases");
await image(s, "02_speedup_n.png");
s.speakerNotes.textFrame.setText("Source: results/n_scaling.csv and results/openmp_n_scaling.csv.");

s = base("Task 1 process scaling and Amdahl analysis");
await image(s, "06_task1_amdahl.png", 65, 120, 560, 430);
await image(s, "03_task1_process.png", 655, 120, 560, 430);
text(s, "At 16 MPI ranks, empirical speedup is 6.66×. The Amdahl curve is higher because it does not model increasing collective latency or root-only file output.", 70, 585, 1130, 55, 22, "#B54708", true);
s.speakerNotes.textFrame.setText("Source: results/task1_process_scaling.csv.");

s = base("Task 2 hybrid decomposition");
text(s, "MPI distributes cyclic candidate sets across processes. Each process applies OpenMP dynamic scheduling to its local candidates.", 70, 120, 1130, 60, 25, "#0A4C5A", true);
text(s, "Measured 4-rank × 2-thread balance at n = 60M", 70, 225, 540, 34, 24, "#0A4C5A", true);
text(s, "Rank 0: 4.264 s   Rank 1: 4.434 s\nRank 2: 4.493 s   Rank 3: 4.431 s\nCandidate-count range: 7,499,999–7,500,000\nMax/min computation-time ratio: 1.054", 85, 275, 560, 150, 23);
text(s, "Dynamic OpenMP scheduling smooths residual cost variation after MPI’s cyclic partitioning.", 70, 485, 560, 70, 22);
text(s, "Task 2 result", 720, 225, 380, 34, 24, "#0A4C5A", true);
text(s, "4 ranks × 2 threads\n5.054 s representative run\n6.97 s Task 1 representative run\n\nHybrid reduces elapsed time while retaining globally ordered output.", 735, 275, 420, 180, 23);

s = base("Hybrid scalability");
await image(s, "04_hybrid_threads.png", 60, 120, 560, 400);
await image(s, "05_hybrid_openmp.png", 660, 120, 560, 400);
text(s, "Four ranks × four threads reached 6.63× speedup. Larger thread counts oversubscribe the available cores and lower efficiency.", 70, 570, 1130, 48, 22, "#B54708", true);

s = base("Hybrid Amdahl analysis, limitations, and Q&A");
await image(s, "07_hybrid_amdahl.png", 65, 105, 580, 410);
text(s, "Why empirical speedup is lower", 700, 120, 430, 30, 25, "#0A4C5A", true);
text(s, "• MPI collective latency rises with rank count\n• Rank 0 still merges and writes the result\n• Memory bandwidth and scheduling contend at high worker counts\n• Amdahl assumes an ideal fixed serial fraction", 715, 165, 450, 160, 21);
text(s, "Q&A", 700, 370, 180, 30, 25, "#0A4C5A", true);
text(s, "Open MPI enables distributed-memory execution across nodes, but this fine-grained kernel pays communication and root-aggregation overhead. Hybrid OpenMP improves local compute throughput; oversubscription reduces efficiency.", 715, 415, 450, 125, 20);
s.speakerNotes.textFrame.setText("Source: results/hybrid_scaling.csv.");

const staging = path.join(workspaceDir, ".codex-finalizer");
await fs.mkdir(staging, { recursive: true });
const candidate = path.join(staging, "candidate.pptx");
await (await PresentationFile.exportPptx(deck)).save(candidate);
await finalizePresentation({
  workspaceDir, candidatePath: candidate, finalPath,
  pythonExecutable: "C:/Users/savin/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe",
  integrityValidatorPath: path.join(skillDir, "container_tools/inspect_presentation_package_integrity.py"),
  layoutValidatorPath: path.join(skillDir, "container_tools/inspect_presentation_layout_geometry.py"),
  layoutArgs: ["--expected-slide-size-emu", "12192000,6858000", "--validate-heading-fit"],
  explicitTotalSlideCount: 9,
  requiredNativeTableOwnerSlides: [],
  requiredNativeChartOwnerSlides: [],
  fontPolicy: { basis: "design", families: [font] },
  verifyArtifactToolImport: true,
  receiptPath: path.join(staging, "presentation-validation.json"),
});
