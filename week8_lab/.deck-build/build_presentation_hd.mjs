import fs from "node:fs/promises";
import path from "node:path";
import { pathToFileURL } from "node:url";
import { Presentation, PresentationFile } from "@oai/artifact-tool";

const workspaceDir = process.cwd();
const skillDir = "C:/Users/savin/.codex/plugins/cache/openai-primary-runtime/presentations/26.909.12148/skills/presentations";
const outputDir = path.join(workspaceDir, "output");
const finalPath = path.join(outputDir, "final", "FIT3143_Week8_Lab2_Presentation_Submission_CAAS_Task2_Run.pptx");
const stagingDir = path.join(workspaceDir, ".codex-finalizer");
const candidatePath = path.join(stagingDir, "candidate-week8-submission-caas-task2-run.pptx");
const receiptPath = path.join(stagingDir, "presentation-week8-submission-caas-task2-run-validation.json");
const { resolvePresentationFont, applyPresentationChartFont, finalizePresentation } = await import(
  pathToFileURL(path.join(skillDir, "container_tools/artifact_tool_utils.mjs")).href
);

await fs.mkdir(outputDir, { recursive: true });
await fs.mkdir(path.dirname(finalPath), { recursive: true });
await fs.mkdir(stagingDir, { recursive: true });

const font = resolvePresentationFont({ preferred: ["Arial"] });
const deck = Presentation.create({ slideSize: { width: 1280, height: 720 } });

const C = {
  ink: "#102A43",
  muted: "#486581",
  teal: "#007C83",
  tealDark: "#005F66",
  cyan: "#18A7B5",
  orange: "#F28E2B",
  green: "#2F855A",
  red: "#C2413B",
  purple: "#7957A5",
  blue: "#3B6FB6",
  paper: "#F7FAFC",
  white: "#FFFFFF",
  line: "#D9E2EC",
  paleTeal: "#E6FFFA",
  paleOrange: "#FFF5E6",
  paleBlue: "#EBF4FF",
  paleRed: "#FFF1F0",
};

function csvParse(raw) {
  const lines = raw.trim().split(/\r?\n/);
  const headers = lines.shift().split(",");
  return lines.map((line) => {
    const values = line.split(",");
    return Object.fromEntries(headers.map((header, i) => [header, values[i]]));
  });
}

async function loadCsv(name) {
  return csvParse(await fs.readFile(path.join(workspaceDir, "results", name), "utf8"));
}

const [nRows, openmpNRows, openmpNBaselineRows, task1Rows, hybridRows, openmpThreadRows, balanceRows] = await Promise.all([
  loadCsv("n_scaling.csv"),
  loadCsv("openmp_n_scaling.csv"),
  loadCsv("task3_n_scaling.csv"),
  loadCsv("task1_process_scaling.csv"),
  loadCsv("hybrid_scaling.csv"),
  loadCsv("openmp_thread_scaling.csv"),
  loadCsv("partition_balance.csv"),
]);

const hybridLookup = new Map(hybridRows.map((row) => [`${row.processes}x${row.threads_per_process}`, row]));
const task1Lookup = new Map(task1Rows.map((row) => [Number(row.processes), row]));
const openmpLookup = new Map(openmpThreadRows.map((row) => [Number(row.threads), row]));

function addText(slide, value, left, top, width, height, options = {}) {
  const shape = slide.shapes.add({
    geometry: "textbox",
    position: { left, top, width, height },
    fill: options.fill ?? "none",
    line: options.line ?? { fill: "none", width: 0 },
  });
  shape.text = value;
  shape.text.style = {
    typeface: font,
    fontSize: options.size ?? 20,
    color: options.color ?? C.ink,
    bold: options.bold ?? false,
    italic: options.italic ?? false,
    alignment: options.align ?? "left",
    verticalAlignment: options.valign ?? "top",
    autoFit: "shrinkText",
  };
  return shape;
}

function addRect(slide, left, top, width, height, fill, radius = 8, line = C.line) {
  return slide.shapes.add({
    geometry: radius > 0 ? "roundRect" : "rect",
    position: { left, top, width, height },
    fill,
    line: { fill: line, width: 1 },
  });
}

function addCard(slide, left, top, width, height, label, value, note, fill = C.white, accent = C.teal) {
  addRect(slide, left, top, width, height, fill, 10);
  addRect(slide, left, top, 7, height, accent, 0, accent);
  addText(slide, label.toUpperCase(), left + 22, top + 16, width - 38, 22, { size: 13, color: C.muted, bold: true });
  addText(slide, value, left + 22, top + 45, width - 38, 42, { size: 27, color: C.ink, bold: true });
  if (note) addText(slide, note, left + 22, top + 91, width - 38, height - 104, { size: 14, color: C.muted });
}

function baseSlide(title, number, appendix = false) {
  const slide = deck.slides.add();
  slide.background.fill = C.paper;
  addRect(slide, 0, 0, 1280, 14, appendix ? C.orange : C.teal, 0, appendix ? C.orange : C.teal);
  addText(slide, appendix ? "APPENDIX" : "FIT3143 • LAB 2", 58, 28, 240, 20, { size: 12, color: appendix ? C.orange : C.teal, bold: true });
  addText(slide, title, 58, 52, 1130, 46, { size: 32, color: C.ink, bold: true });
  addText(slide, appendix ? `Appendix ${number - 9}` : `${number} / 9`, 1150, 676, 78, 20, { size: 12, color: C.muted, align: "right" });
  return slide;
}

function addCaption(slide, value, left, top, width) {
  addRect(slide, left, top, width, 54, C.paleOrange, 8, "#F5C98A");
  addText(slide, value, left + 18, top + 13, width - 36, 30, { size: 16, color: "#8A4B08", bold: true, align: "center" });
}

async function addEvidenceScreenshot(slide, relativePath, alt, position) {
  const image = await fs.readFile(path.join(workspaceDir, relativePath));
  slide.images.add({
    blob: image,
    contentType: relativePath.toLowerCase().endsWith(".jpg") ? "image/jpeg" : "image/png",
    alt,
    fit: "contain",
    position,
  });
}

function chartTextStyle(size = 12, color = C.muted, bold = false) {
  return { typeface: font, fontSize: size, fill: color, bold };
}

function chartNumber(value) {
  return Number(Number(value).toFixed(6));
}

function addScatter(slide, position, title, series, xTitle, yTitle, limits = {}) {
  const chart = slide.charts.add("scatter", {
    position,
    title,
    titlePlacement: "aboveChart",
    titleTextStyle: chartTextStyle(15, C.ink, true),
    series: series.map((item) => ({
      name: item.name,
      xValues: item.x.map(chartNumber),
      values: item.y.map(chartNumber),
      line: { style: "solid", fill: item.color, width: 2.3 },
      marker: { symbol: "circle", size: item.markerSize ?? 4 },
    })),
    hasLegend: true,
    legend: { position: "bottom", overlay: false, textStyle: chartTextStyle(10, C.muted) },
    scatterOptions: { style: "lineWithMarkers", varyColors: false },
    xAxis: {
      title: { text: xTitle, textStyle: chartTextStyle(11, C.muted, true) },
      min: limits.xMin,
      max: limits.xMax,
      majorUnit: limits.xMajor,
      textStyle: chartTextStyle(10, C.muted),
      line: { style: "solid", fill: C.line, width: 1 },
      majorGridlines: null,
    },
    yAxis: {
      title: { text: yTitle, textStyle: chartTextStyle(11, C.muted, true) },
      min: limits.yMin ?? 0,
      max: limits.yMax,
      majorUnit: limits.yMajor,
      numberFormatCode: limits.yFormat ?? "0.0",
      textStyle: chartTextStyle(10, C.muted),
      line: { style: "solid", fill: C.line, width: 1 },
      majorGridlines: { style: "solid", fill: "#E7EDF3", width: 1 },
    },
    chartFill: C.white,
    plotAreaFill: C.white,
    chartLine: { style: "solid", fill: C.line, width: 1 },
    plotAreaLine: { style: "solid", fill: "#EEF2F6", width: 1 },
  });
  applyPresentationChartFont(chart, { fontFamily: font });
  return chart;
}

function addLine(slide, position, title, categories, series, xTitle, yTitle, limits = {}) {
  const chart = slide.charts.add("line", {
    position,
    title,
    titlePlacement: "aboveChart",
    titleTextStyle: chartTextStyle(15, C.ink, true),
    categories,
    series: series.map((item) => ({
      name: item.name,
      values: item.y.map(chartNumber),
      line: { style: "solid", fill: item.color, width: 2.3 },
      marker: { symbol: "circle", size: item.markerSize ?? 5 },
    })),
    hasLegend: true,
    legend: { position: "bottom", overlay: false, textStyle: chartTextStyle(10, C.muted) },
    lineOptions: { grouping: "standard", smooth: false, varyColors: false },
    xAxis: {
      title: { text: xTitle, textStyle: chartTextStyle(11, C.muted, true) },
      textStyle: chartTextStyle(10, C.muted),
      line: { style: "solid", fill: C.line, width: 1 },
      majorGridlines: null,
    },
    yAxis: {
      title: { text: yTitle, textStyle: chartTextStyle(11, C.muted, true) },
      min: limits.yMin ?? 0,
      max: limits.yMax,
      majorUnit: limits.yMajor,
      numberFormatCode: limits.yFormat ?? "0.0",
      textStyle: chartTextStyle(10, C.muted),
      line: { style: "solid", fill: C.line, width: 1 },
      majorGridlines: { style: "solid", fill: "#E7EDF3", width: 1 },
    },
    chartFill: C.white,
    plotAreaFill: C.white,
    chartLine: { style: "solid", fill: C.line, width: 1 },
    plotAreaLine: { style: "solid", fill: "#EEF2F6", width: 1 },
  });
  applyPresentationChartFont(chart, { fontFamily: font });
  return chart;
}

function addTable(slide, values, left, top, width, height, widths, options = {}) {
  const table = slide.tables.add({
    rows: values.length,
    columns: values[0].length,
    left,
    top,
    width,
    height,
    columnWidths: widths,
    values,
  });
  table.styleOptions = { headerRow: true, bandedRows: false };
  table.borders.assign({ style: "solid", fill: C.line, width: 1 });
  table.cells.block({ row: 0, column: 0, rowCount: 1, columnCount: values[0].length }).assign({
    fill: options.headerFill ?? C.tealDark,
    textStyle: { typeface: font, fontSize: options.headerSize ?? 12, color: C.white, bold: true },
    margins: { left: 8, right: 8, top: 6, bottom: 6 },
    anchor: "middle",
  });
  if (values.length > 1) {
    table.cells.block({ row: 1, column: 0, rowCount: values.length - 1, columnCount: values[0].length }).assign({
      fill: C.white,
      textStyle: { typeface: font, fontSize: options.bodySize ?? 12, color: C.ink },
      margins: { left: 8, right: 8, top: 5, bottom: 5 },
      anchor: "middle",
    });
  }
  return table;
}

// Slide 1
{
  const slide = deck.slides.add();
  slide.background.fill = C.tealDark;
  addRect(slide, 0, 0, 1280, 18, C.orange, 0, C.orange);
  addText(slide, "FIT3143 • PARALLEL COMPUTING", 74, 74, 650, 24, { size: 15, color: "#B7E7E5", bold: true });
  addText(slide, "Prime Search at Scale", 74, 135, 1060, 76, { size: 52, color: C.white, bold: true });
  addText(slide, "Open MPI and hybrid MPI + OpenMP", 74, 222, 950, 48, { size: 29, color: "#D4F4F2" });
  addRect(slide, 74, 318, 470, 5, C.orange, 0, C.orange);
  addText(slide, "Correctness • workload balance • measured scalability", 74, 350, 920, 34, { size: 22, color: C.white });
  addText(slide, "Savin Vindiv De Alwis (35221631)\nWillwara Arachchilage Shamle Imal Thilaksiri (35512075)", 74, 505, 820, 64, { size: 19, color: C.white });
  addText(slide, "Main presentation: 6 min 25 s", 74, 620, 420, 24, { size: 15, color: "#B7E7E5", bold: true });
}

// Slide 2
{
  const slide = baseSlide("Experimental setup and correctness", 2);
  addCard(slide, 58, 122, 360, 150, "Platform", "1 node • 14C / 20T", "WSL2 Ubuntu • Open MPI 4.1.6\nIntel Core i7-13700H", C.white, C.blue);
  addCard(slide, 460, 122, 360, 150, "Primary workload", "n = 60,000,000", "3,562,115 primes below n\nall output written by rank 0", C.white, C.teal);
  addCard(slide, 862, 122, 360, 150, "Coverage", "30 n values", "20M–78M • MPI ranks 1–16\nOpenMP threads 1–16 • up to 64 workers", C.white, C.orange);
  addRect(slide, 58, 310, 560, 290, C.white, 10);
  addText(slide, "Measurement discipline", 82, 334, 500, 28, { size: 22, color: C.tealDark, bold: true });
  addText(slide, "Process and hybrid scaling", 82, 386, 230, 22, { size: 16, color: C.ink, bold: true });
  addText(slide, "Median of 3 end-to-end runs; computation, gather and root post-processing recorded separately.", 82, 414, 485, 56, { size: 17, color: C.muted });
  addText(slide, "n sweep", 82, 496, 230, 22, { size: 16, color: C.ink, bold: true });
  addText(slide, "One run at every required n; every runtime exceeds one second. Oversubscription is included explicitly.", 82, 524, 485, 56, { size: 17, color: C.muted });
  addRect(slide, 652, 310, 570, 290, C.paleTeal, 10, "#9ED9D5");
  addText(slide, "Independent exact validation", 680, 334, 500, 28, { size: 22, color: C.tealDark, bold: true });
  addText(slide, "Both programs were compared value-for-value with a separate sieve across MPI ranks 1–5 and thread counts 1–3, including n = 0, 1, 2, 3, 4, 10 and 1000.", 680, 388, 500, 98, { size: 18, color: C.ink });
  addText(slide, "PASS: sorted • unique • complete • prime-only • < n", 680, 522, 500, 44, { size: 19, color: C.green, bold: true, align: "center" });
  slide.speakerNotes.textFrame.setText("Sources: results/local_cluster_execution.txt; scripts/verify_correctness.sh; results/n_scaling.csv; results/task1_process_scaling.csv; results/hybrid_scaling.csv.");
}

// Slide 3
{
  const slide = baseSlide("Task 1: robust block-cyclic MPI partition", 3);
  addRect(slide, 58, 120, 540, 250, C.white, 10);
  addText(slide, "Odd-candidate index space", 82, 142, 460, 25, { size: 20, color: C.tealDark, bold: true });
  addText(slide, "Chunks of 64 odd candidates are assigned round-robin.", 82, 178, 460, 28, { size: 17, color: C.muted });
  const chunks = [
    ["rank 0", ["0", "3", "6", "9"], C.blue],
    ["rank 1", ["1", "4", "7", "10"], C.teal],
    ["rank 2", ["2", "5", "8", "11"], C.orange],
  ];
  chunks.forEach(([label, ids, color], row) => {
    addText(slide, label, 86, 230 + row * 46, 76, 26, { size: 15, color, bold: true });
    ids.forEach((id, col) => {
      addRect(slide, 170 + col * 82, 225 + row * 46, 62, 30, color, 6, color);
      addText(slide, `C${id}`, 170 + col * 82, 230 + row * 46, 62, 20, { size: 13, color: C.white, bold: true, align: "center" });
    });
  });
  addText(slide, "Equal-sized chunks preserve locality while mixing cheap composites and expensive large candidates across ranks.", 82, 388, 490, 62, { size: 18, color: C.ink });
  addText(slide, "Rank 0 performs an O(m log p) k-way merge and buffered output.", 82, 468, 490, 48, { size: 18, color: C.ink, bold: true });

  const ratio = (p, strategy) => Number(balanceRows.find((r) => Number(r.processes) === p && r.strategy === strategy).imbalance_ratio).toFixed(2);
  addText(slide, "Measured computation-time imbalance (max / min)", 636, 126, 586, 28, { size: 20, color: C.tealDark, bold: true });
  addTable(slide, [
    ["Strategy", "p = 3", "p = 4", "p = 5"],
    ["Block-cyclic", ratio(3, "block-cyclic"), ratio(4, "block-cyclic"), ratio(5, "block-cyclic")],
    ["Pure cyclic", ratio(3, "cyclic"), ratio(4, "cyclic"), ratio(5, "cyclic")],
    ["Contiguous block", ratio(3, "block"), ratio(4, "block"), ratio(5, "block")],
  ], 636, 170, 586, 212, [210, 125, 125, 126], { bodySize: 13 });
  addRect(slide, 636, 414, 586, 146, C.paleOrange, 10, "#F5C98A");
  addText(slide, "Why this is the selected partition", 660, 436, 530, 25, { size: 19, color: "#8A4B08", bold: true });
  addText(slide, "It had the lowest worst-case imbalance among the three measured strategies: ≤ 1.06 for p = 3–5. Pure cyclic collapses at odd p because divisibility patterns align with rank strides.", 660, 474, 530, 70, { size: 17, color: C.ink });
  slide.speakerNotes.textFrame.setText("Source: results/partition_balance.csv. Each value is the median of three runs at n=20,000,000. The strategy comparison uses identical primality testing, merge and output code.");
}

// Slide 4
{
  const slide = baseSlide("Graph 1 — runtime across 30 problem sizes", 4);
  const x = nRows.map((r) => Number(r.n) / 1e6);
  addScatter(slide, { left: 70, top: 118, width: 1140, height: 470 }, "End-to-end runtime", [
    { name: "Serial baseline", x, y: nRows.map((r) => Number(r.serial_time)), color: C.blue },
    { name: "Open MPI • 4 processes", x, y: nRows.map((r) => Number(r.mpi_time)), color: C.red },
    { name: "OpenMP • 4 threads", x, y: openmpNRows.map((r) => Number(r.openmp_time)), color: C.green },
  ], "n (millions)", "Runtime (seconds)", { xMin: 20, xMax: 80, xMajor: 10, yMin: 0, yMajor: 10, yFormat: "0" });
  addCaption(slide, "All 30 values are shown; no run is below one second.", 265, 604, 750);
  slide.speakerNotes.textFrame.setText("Sources: results/n_scaling.csv (current serial and MPI sweep) and results/openmp_n_scaling.csv (prior Week 4 OpenMP sweep). Both use the same machine, n values and 4 workers, but were measured in separate sessions; absolute runtime may include session noise.");
}

// Slide 5
{
  const slide = baseSlide("Graph 2 — speedup across 30 problem sizes", 5);
  const x = nRows.map((r) => Number(r.n) / 1e6);
  const mpiSpeedup = nRows.map((r) => Number(r.speedup));
  const openmpSpeedup = openmpNRows.map((r, i) => Number(openmpNBaselineRows[i].serial_total) / Number(r.openmp_time));
  addScatter(slide, { left: 70, top: 118, width: 1140, height: 470 }, "Speedup against each implementation's matched serial baseline", [
    { name: "Open MPI • 4 processes", x, y: mpiSpeedup, color: C.red },
    { name: "OpenMP • 4 threads", x, y: openmpSpeedup, color: C.green },
  ], "n (millions)", "Empirical speedup", { xMin: 20, xMax: 80, xMajor: 10, yMin: 0, yMax: 5, yMajor: 1, yFormat: "0.0x" });
  addCaption(slide, "MPI stays near 3.3–3.6× at four processes; the OpenMP series is noisier but uses its own matched serial session.", 145, 604, 990);
  slide.speakerNotes.textFrame.setText("Sources: MPI speedup from results/n_scaling.csv. OpenMP speedup = results/task3_n_scaling.csv serial_total divided by results/openmp_n_scaling.csv openmp_time for the same n. This avoids mixing serial baselines across sessions.");
}

// Slide 6
{
  const slide = baseSlide("Graphs 3 and 6 — Task 1 scaling and Amdahl comparison", 6);
  const counts = [1, 2, 4, 8, 16];
  const serialRef = Number(task1Rows[0].serial_seconds);
  addLine(slide, { left: 54, top: 122, width: 575, height: 440 }, "Graph 3 • matching worker counts", counts.map(String), [
    { name: "MPI processes", y: counts.map((p) => Number(task1Lookup.get(p).empirical_speedup)), color: C.red },
    { name: "OpenMP threads", y: counts.map((t) => serialRef / Number(openmpLookup.get(t).openmp_time)), color: C.green },
  ], "Processes or threads", "Empirical speedup", { yMin: 0, yMax: 9, yMajor: 2, yFormat: "0.0x" });

  addScatter(slide, { left: 651, top: 122, width: 575, height: 440 }, "Graph 6 • MPI empirical vs Amdahl", [
    { name: "Empirical", x: task1Rows.map((r) => Number(r.processes)), y: task1Rows.map((r) => Number(r.empirical_speedup)), color: C.red },
    { name: "Amdahl ideal", x: task1Rows.map((r) => Number(r.processes)), y: task1Rows.map((r) => Number(r.theoretical_speedup)), color: C.orange },
  ], "MPI processes", "Speedup", { xMin: 1, xMax: 16, xMajor: 3, yMin: 0, yMax: 14, yMajor: 2, yFormat: "0.0x" });
  addCaption(slide, "At p = 16: 6.02× empirical versus 12.67× Amdahl — collectives, contention and root-only output are not in the ideal model.", 115, 590, 1050);
  slide.speakerNotes.textFrame.setText("Graph 3 sources: results/task1_process_scaling.csv and results/openmp_thread_scaling.csv. Graph 6 source: results/task1_process_scaling.csv. Process-scaling points are medians of three runs at n=60,000,000.");
}

// Slide 7
{
  const slide = baseSlide("Task 2: hybrid decomposition and measured balance", 7);
  addText(slide, "Two-level parallelism", 58, 122, 510, 28, { size: 22, color: C.tealDark, bold: true });
  const flow = [
    ["MPI", "block-cyclic chunks\nacross processes", C.blue],
    ["OpenMP", "dynamic schedule(64)\ninside each process", C.teal],
    ["Rank 0", "k-way merge +\nbuffered output", C.orange],
  ];
  flow.forEach(([head, body, color], i) => {
    const left = 58 + i * 183;
    addRect(slide, left, 174, 155, 140, C.white, 10, color);
    addText(slide, head, left + 12, 194, 131, 27, { size: 21, color, bold: true, align: "center" });
    addText(slide, body, left + 12, 235, 131, 58, { size: 15, color: C.ink, align: "center" });
    if (i < 2) addText(slide, "→", left + 157, 222, 28, 30, { size: 28, color: C.muted, bold: true, align: "center" });
  });
  addRect(slide, 58, 348, 521, 210, C.paleTeal, 10, "#9ED9D5");
  addText(slide, "Why dynamic scheduling remains useful", 82, 372, 470, 26, { size: 19, color: C.tealDark, bold: true });
  addText(slide, "Trial division has irregular per-candidate cost. MPI chunks balance rank-level work; OpenMP dynamic scheduling smooths the remaining variation between threads without changing output order.", 82, 418, 470, 112, { size: 18, color: C.ink });

  addText(slide, "Representative local execution at n = 60M", 624, 126, 598, 28, { size: 20, color: C.tealDark, bold: true });
  addTable(slide, [
    ["Program", "Configuration", "min compute", "max compute", "max/min"],
    ["Task 1", "4 ranks", "6.446 s", "6.507 s", "1.009"],
    ["Task 2", "4 × 2", "4.567 s", "4.685 s", "1.026"],
  ], 624, 172, 598, 166, [110, 125, 125, 125, 113], { bodySize: 12 });
  addCard(slide, 624, 376, 282, 180, "Task 1", "6.97 s", "4 MPI processes\nexact output validation: PASS", C.white, C.blue);
  addCard(slide, 940, 376, 282, 180, "Task 2", "5.16 s", "4 MPI × 2 OpenMP\nexact output validation: PASS", C.white, C.teal);
  addCaption(slide, "Hybrid speedup over the same 4-rank Task 1 run: 1.35×", 640, 588, 566);
  slide.speakerNotes.textFrame.setText("Source: results/local_cluster_execution.txt. This representative run records per-rank compute times, candidate-count range, total runtime, commands and exact sorted-output validation.");
}

// Slide 8
{
  const slide = baseSlide("Graphs 4 and 5 — hybrid scaling", 8);
  const threads = [1, 2, 4, 8, 16];
  const fixedFour = threads.map((t) => hybridLookup.get(`4x${t}`));
  addLine(slide, { left: 54, top: 122, width: 575, height: 440 }, "Graph 4 • fixed 4 MPI processes", threads.map(String), [
    { name: "Hybrid / Task 1", y: fixedFour.map((r) => Number(r.vs_mpi_speedup)), color: C.purple },
  ], "Threads per MPI process", "Speedup versus 4-rank Task 1", { yMin: 0, yMax: 2.7, yMajor: 0.5, yFormat: "0.0x" });

  const matchedPath = [[1, 1], [2, 1], [2, 2], [4, 2], [4, 4]];
  const matchedRows = matchedPath.map(([p, t]) => hybridLookup.get(`${p}x${t}`));
  const labels = matchedPath.map(([p, t]) => `${p}×${t}`);
  addLine(slide, { left: 651, top: 122, width: 575, height: 440 }, "Graph 5 • matched total workers", labels, [
    { name: "Hybrid MPI × OpenMP", y: matchedRows.map((r) => Number(r.empirical_speedup)), color: C.purple },
    { name: "OpenMP matched threads", y: matchedRows.map((r) => Number(hybridRows[0].serial_seconds) / Number(openmpLookup.get(Number(r.total_workers)).openmp_time)), color: C.green },
  ], "MPI ranks × threads", "Empirical speedup", { yMin: 0, yMax: 9, yMajor: 2, yFormat: "0.0x" });
  addCaption(slide, "Best measured hybrid point: 8.46× at 1 × 16; 32- and 64-worker configurations (4×8 and 4×16) reduce efficiency.", 140, 590, 1000);
  slide.speakerNotes.textFrame.setText("Sources: results/hybrid_scaling.csv and results/openmp_thread_scaling.csv. Hybrid points are medians of three runs at n=60,000,000. Graph 5 compares equal total worker counts.");
}

// Slide 9
{
  const slide = baseSlide("Graph 7 — hybrid scaling limits", 9);
  const amdahlPath = [[1, 1], [2, 1], [2, 2], [4, 2], [4, 4], [4, 8], [4, 16]];
  const amdahlRows = amdahlPath.map(([p, t]) => hybridLookup.get(`${p}x${t}`));
  addLine(slide, { left: 50, top: 120, width: 655, height: 455 }, "Hybrid empirical versus ideal", amdahlPath.map(([p, t]) => `${p}×${t}`), [
    { name: "Empirical", y: amdahlRows.map((r) => Number(r.empirical_speedup)), color: C.purple },
    { name: "Amdahl ideal", y: amdahlRows.map((r) => Number(r.theoretical_speedup)), color: C.orange },
  ], "MPI ranks × threads", "Speedup", { yMin: 0, yMax: 40, yMajor: 10, yFormat: "0.0x" });
  addRect(slide, 746, 122, 476, 210, C.white, 10);
  addText(slide, "Why the curves diverge", 772, 144, 420, 26, { size: 21, color: C.tealDark, bold: true });
  addText(slide, "MPI collectives and rank-0 aggregation\nMemory and scheduler contention\nOversubscription beyond 20 logical CPUs\nAmdahl assumes a fixed, ideal serial fraction", 772, 190, 420, 118, { size: 18, color: C.ink });
  addRect(slide, 746, 354, 476, 218, C.paleOrange, 10, "#F5C98A");
  addText(slide, "Limitations and future work", 772, 376, 420, 26, { size: 21, color: "#8A4B08", bold: true });
  addText(slide, "The benchmark series is single-node data, so it does not quantify network cost. A two-node CAAS functional run completed successfully. Next: multi-node CAAS measurements, segmented-sieve work units, and distributed output.", 772, 422, 420, 112, { size: 18, color: C.ink });
  addText(slide, "Q&A", 998, 610, 190, 34, { size: 28, color: C.tealDark, bold: true, align: "right" });
  slide.speakerNotes.textFrame.setText("Source: results/hybrid_scaling.csv. Amdahl serial fraction is measured from the one-worker hybrid run: Ts=0.411327 s, Tp=23.749574 s, fs=0.017024. CAAS evidence in Appendices 3-6 verifies setup, Task 1 configuration and execution, and Task 2 configuration; it is not graph or balance data. Main presentation ends here; appendix slides are for Q&A only.");
}

// Slide 10 — Appendix
{
  const slide = baseSlide("Measured partition and Amdahl evidence", 10, true);
  addText(slide, "Partition strategy comparison • n = 20M • median of 3 runs", 58, 116, 1160, 26, { size: 20, color: C.tealDark, bold: true });
  addTable(slide, [
    ["p", "Block-cyclic ratio", "Pure cyclic ratio", "Block ratio", "Block-cyclic total"],
    ...[3, 4, 5].map((p) => {
      const row = (s) => balanceRows.find((r) => Number(r.processes) === p && r.strategy === s);
      return [String(p), Number(row("block-cyclic").imbalance_ratio).toFixed(3), Number(row("cyclic").imbalance_ratio).toFixed(3), Number(row("block").imbalance_ratio).toFixed(3), `${Number(row("block-cyclic").overall_seconds).toFixed(3)} s`];
    }),
  ], 58, 158, 1164, 196, [100, 250, 250, 230, 334], { bodySize: 13 });

  addText(slide, "Amdahl inputs are measured, not assumed", 58, 392, 540, 26, { size: 20, color: C.tealDark, bold: true });
  addTable(slide, [
    ["Program", "Ts", "Tp", "fs = Ts / (Ts+Tp)"],
    ["Task 1 MPI", "0.427566 s", "23.948293 s", "0.017541"],
    ["Task 2 hybrid", "0.411327 s", "23.749574 s", "0.017024"],
  ], 58, 432, 670, 156, [175, 155, 165, 175], { bodySize: 13 });
  addRect(slide, 766, 392, 456, 196, C.paleBlue, 10, "#B6D4FE");
  addText(slide, "Formula", 792, 414, 390, 24, { size: 19, color: C.blue, bold: true });
  addText(slide, "S(W) = (Ts + Tp) / (Ts + Tp / W)", 792, 458, 390, 38, { size: 22, color: C.ink, bold: true, align: "center" });
  addText(slide, "Empirical speedup uses the Week 4 serial end-to-end baseline. Theoretical speedup uses each parallel program's measured one-worker decomposition; this difference is stated explicitly.", 792, 512, 390, 60, { size: 15, color: C.muted });
  slide.speakerNotes.textFrame.setText("Sources: results/partition_balance.csv, results/task1_process_scaling.csv and results/hybrid_scaling.csv.");
}

// Slide 11 — Appendix
{
  const slide = baseSlide("Evidence and Moodle submission readiness", 11, true);
  addText(slide, "Verified artifacts", 58, 116, 1160, 26, { size: 20, color: C.tealDark, bold: true });
  const table = addTable(slide, [
    ["Requirement", "Artifact", "Status"],
    ["Task 1 source", "task1.c", "Ready"],
    ["Task 2 source", "task2.c", "Ready"],
    ["Presentation", "FIT3143_Week8_Lab2_Presentation_Submission.pptx", "Ready"],
    ["Supporting scripts", "supporting/scripts/ (retained for inspection)", "Optional upload"],
    ["Supporting results", "supporting/results/ and supporting/evidence/", "Optional upload"],
    ["AI use declaration", "Appendix 7 in this presentation", "Ready"],
    ["AI prompt record PDF", "Export from the actual AI conversation", "ADD BEFORE MOODLE"],
    ["CAAS screenshots", "Appendices 3-6: setup, Task 1 config/run, Task 2 config", "Ready"],
  ], 58, 156, 1164, 388, [270, 650, 244], { bodySize: 12 });
  table.getCell(7, 2).fill = C.paleRed;
  table.getCell(7, 2).text.style = { typeface: font, fontSize: 12, color: C.red, bold: true };
  addCaption(slide, "Do not submit until the red item is authentic, legible and included. This review cannot fabricate it.", 170, 582, 940);
  slide.speakerNotes.textFrame.setText("Required Moodle files are task1.c, task2.c, and the presentation. Supporting scripts, results, CSVs, and screenshots are retained for inspection but are not listed as compulsory uploads in the assessment specification. The prompt/conversation record must still be exported authentically if AI was used.");
}

// Slide 12 — Appendix
{
  const slide = baseSlide("CAAS evidence — access and job setup", 12, true);
  addText(slide, "CAAS head-node access and source upload", 58, 116, 710, 28, { size: 20, color: C.tealDark, bold: true });
  await addEvidenceScreenshot(
    slide,
    "evidence/screenshots/caas_login_and_setup.jpg",
    "CAAS head-node login, scheduler output, and uploaded Task 1 and Task 2 source files",
    { left: 58, top: 152, width: 650, height: 470 },
  );
  addRect(slide, 748, 166, 452, 346, C.white, 10);
  addText(slide, "What this proves", 778, 194, 370, 26, { size: 20, color: C.tealDark, bold: true });
  addText(slide, "• Authenticated CAAS head-node access\n• SLURM defq reported idle compute nodes\n• Task 1 and Task 2 sources uploaded\n• Setup evidence only; no performance claim", 778, 240, 370, 190, { size: 17, color: C.ink });
  addCaption(slide, "This capture establishes the CAAS environment before the Task 1 job configuration and functional run in Appendices 4-5.", 104, 636, 1072);
  slide.speakerNotes.textFrame.setText("Source: evidence/screenshots/caas_login_and_setup.jpg. Authentic CAAS setup screenshot. It documents head-node access, scheduler visibility, and uploaded source files.");
}

// Slide 13 — Appendix
{
  const slide = baseSlide("CAAS evidence — Task 1 job configuration", 13, true);
  addText(slide, "SLURM script for the two-node Task 1 run", 58, 116, 710, 28, { size: 20, color: C.tealDark, bold: true });
  await addEvidenceScreenshot(
    slide,
    "evidence/screenshots/caas_task1_job_configuration.jpg",
    "CAAS Task 1 SLURM job script requesting two nodes and eight MPI tasks",
    { left: 58, top: 152, width: 650, height: 470 },
  );
  addRect(slide, 748, 166, 452, 346, C.white, 10);
  addText(slide, "What this proves", 778, 194, 370, 26, { size: 20, color: C.tealDark, bold: true });
  addText(slide, "• 2 nodes and 8 MPI tasks requested\n• 4 MPI ranks per node, 1 CPU per rank\n• Open MPI module loaded before compilation\n• srun executes Task 1 at n = 10,000", 778, 240, 370, 190, { size: 17, color: C.ink });
  addCaption(slide, "Configuration evidence for the submitted CAAS Task 1 run shown in Appendix 5.", 104, 636, 1072);
  slide.speakerNotes.textFrame.setText("Source: evidence/screenshots/caas_task1_job_configuration.jpg. Authentic Task 1 SLURM job configuration. It documents the requested two nodes, eight MPI tasks, four tasks per node, compilation command, and srun invocation.");
}

// Slide 14 — Appendix
{
  const slide = baseSlide("CAAS evidence — two-node Task 1 run", 14, true);
  addText(slide, "Two-node CAAS Task 1 functional execution", 58, 116, 710, 28, { size: 20, color: C.tealDark, bold: true });
  await addEvidenceScreenshot(
    slide,
    "evidence/screenshots/caas_task1_two_node_execution_with_line_count.jpg",
    "CAAS Task 1 output showing two allocated nodes, eight MPI ranks, correct prime count, and the 1,229-line output check",
    { left: 58, top: 152, width: 650, height: 470 },
  );
  addRect(slide, 748, 166, 452, 346, C.white, 10);
  addText(slide, "What this proves", 778, 194, 370, 26, { size: 20, color: C.tealDark, bold: true });
  addText(slide, "• Allocated nodes: student-caas-n03 and n04\n• 8 MPI ranks, 4 ranks on each node\n• Task 1 at n = 10,000 returned 1,229 primes\n• wc -l confirms 1,229 output lines", 778, 240, 370, 190, { size: 17, color: C.ink });
  addCaption(slide, "CAAS functional evidence. The existing graphs and balance findings remain based on the documented local measurement files.", 104, 636, 1072);
  slide.speakerNotes.textFrame.setText("Source: evidence/screenshots/caas_task1_two_node_execution_with_line_count.jpg. Authentic two-node CAAS Task 1 functional run at n = 10,000. It verifies allocation of student-caas-n03 and student-caas-n04, eight MPI ranks, the correct 1,229-prime result, sorted output generation, and the 1,229-line output check. It does not support the local benchmark or balance conclusions.");
}

// Slide 15 — Appendix
{
  const slide = baseSlide("CAAS evidence — Task 2 job configuration", 15, true);
  addText(slide, "SLURM script for the two-node hybrid Task 2 run", 58, 116, 710, 28, { size: 20, color: C.tealDark, bold: true });
  await addEvidenceScreenshot(
    slide,
    "evidence/screenshots/caas_task2_job_configuration.jpg",
    "CAAS Task 2 hybrid SLURM job script requesting two nodes, four MPI tasks, and four CPUs per task",
    { left: 58, top: 152, width: 650, height: 470 },
  );
  addRect(slide, 748, 166, 452, 346, C.white, 10);
  addText(slide, "What this proves", 778, 194, 370, 26, { size: 20, color: C.tealDark, bold: true });
  addText(slide, "• 2 nodes and 4 MPI tasks requested\n• 2 MPI ranks per node, 4 CPUs per rank\n• OMP_NUM_THREADS derives from SLURM_CPUS_PER_TASK\n• Configuration only; no CAAS Task 2 output claim", 778, 240, 370, 190, { size: 17, color: C.ink });
  addCaption(slide, "Task 2 configuration evidence: the hybrid launch uses 4 MPI ranks × 4 OpenMP threads when scheduled as shown.", 104, 636, 1072);
  slide.speakerNotes.textFrame.setText("Source: evidence/screenshots/caas_task2_job_configuration.jpg. Authentic Task 2 hybrid SLURM job configuration. It documents the requested two nodes, four MPI tasks, two tasks per node, four CPUs per task, OMP_NUM_THREADS export, and srun invocation. It does not demonstrate a completed CAAS Task 2 execution.");
}

// Slide 16 — Appendix
{
  const slide = baseSlide("Generative AI use declaration", 16, true);
  addText(slide, "Included in this presentation", 58, 116, 620, 28, { size: 20, color: C.tealDark, bold: true });
  addText(slide, "Scope of assistance", 78, 176, 300, 26, { size: 21, color: C.tealDark, bold: true });
  addText(slide, "Generative AI assisted with interpreting the specification and rubric, reviewing the MPI and hybrid implementation, planning experiments, drafting charts and slides, and preparing documentation.", 78, 215, 530, 108, { size: 18, color: C.ink });
  addRect(slide, 640, 164, 2, 166, C.line, 0, C.line);
  addText(slide, "Team responsibility", 678, 176, 300, 26, { size: 21, color: C.tealDark, bold: true });
  addText(slide, "The team reviewed the material against the supplied requirements, compiled and executed the programs locally, validated prime output independently, and accepts responsibility for every submitted claim and result.", 678, 215, 520, 108, { size: 18, color: C.ink });
  addRect(slide, 78, 370, 1122, 2, C.line, 0, C.line);
  addText(slide, "Presentation and Q&A", 78, 404, 360, 26, { size: 21, color: C.tealDark, bold: true });
  addText(slide, "The team will explain the partitioning decision, MPI collectives, OpenMP scheduling, timing method, and performance observations without external tools.", 78, 444, 1120, 42, { size: 18, color: C.ink });
  addCaption(slide, "This presentation records the AI-use declaration. Export the authentic prompt/conversation record from the AI service before Moodle submission.", 104, 544, 1072);
  slide.speakerNotes.textFrame.setText("This appendix declares the use of generative AI within the presentation itself. The assessment specification also requires the team to upload an authentic prompt/conversation record if AI was used. No separate AI declaration PDF is included.");
}

// Slide 17 — Appendix
{
  const slide = baseSlide("CAAS evidence — two-node Task 2 run", 17, true);
  addText(slide, "Two-node CAAS hybrid MPI + OpenMP functional execution", 58, 116, 710, 28, { size: 20, color: C.tealDark, bold: true });
  await addEvidenceScreenshot(
    slide,
    "evidence/screenshots/caas_task2_two_node_execution.png",
    "CAAS Task 2 output showing two allocated nodes, four MPI ranks, four OpenMP threads per rank, correct prime count, and the 1,229-line output check",
    { left: 58, top: 152, width: 650, height: 470 },
  );
  addRect(slide, 748, 166, 452, 346, C.white, 10);
  addText(slide, "What this proves", 778, 194, 370, 26, { size: 20, color: C.tealDark, bold: true });
  addText(slide, "• Allocated nodes: student-caas-n03 and n04\n• 4 MPI ranks × 4 OpenMP threads = 16 workers\n• Task 2 at n = 10,000 returned 1,229 primes\n• wc -l confirms 1,229 output lines", 778, 240, 370, 190, { size: 17, color: C.ink });
  addCaption(slide, "CAAS functional evidence only. The benchmark charts and balance findings remain based on the documented local measurement files.", 104, 636, 1072);
  slide.speakerNotes.textFrame.setText("Source: evidence/screenshots/caas_task2_two_node_execution.png. Authentic two-node CAAS Task 2 functional run at n = 10,000. It verifies allocation of student-caas-n03 and student-caas-n04, four MPI ranks, four OpenMP threads per process, the correct 1,229-prime result, output generation, and the 1,229-line output check. It does not support the local benchmark or balance conclusions.");
}

await (await PresentationFile.exportPptx(deck)).save(candidatePath);
await finalizePresentation({
  workspaceDir,
  candidatePath,
  finalPath,
  pythonExecutable: "C:/Users/savin/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe",
  integrityValidatorPath: path.join(skillDir, "container_tools/inspect_presentation_package_integrity.py"),
  layoutValidatorPath: path.join(skillDir, "container_tools/inspect_presentation_layout_geometry.py"),
  layoutArgs: [
    "--expected-slide-size-emu", "12192000,6858000",
    "--validate-heading-fit",
    "--require-native-table-slide", "3",
    "--require-native-table-slide", "7",
    "--require-native-table-slide", "10",
    "--require-native-table-slide", "11",
  ],
  explicitTotalSlideCount: 17,
  requiredNativeTableOwnerSlides: [3, 7, 10, 11],
  requiredNativeChartOwnerSlides: [4, 5, 6, 8, 9],
  nativeChartTargetApplication: "portable",
  materializeLiteralChartWorkbooks: true,
  fontPolicy: { basis: "design", families: [font] },
  verifyArtifactToolImport: true,
  receiptPath,
});

console.log(finalPath);
