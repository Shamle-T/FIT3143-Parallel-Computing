import fs from "node:fs/promises";
import path from "node:path";
import crypto from "node:crypto";
import { pathToFileURL } from "node:url";
import { FileBlob, PresentationFile } from "@oai/artifact-tool";

const workspaceDir = process.cwd();
const skillDir = "C:/Users/savin/.codex/plugins/cache/openai-primary-runtime/presentations/26.909.12148/skills/presentations";
const runtimePython = "C:/Users/savin/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe";
const sourcePath = path.join(workspaceDir, "output", "FIT3143_Week8_Lab2_Presentation_HD_Submission_With_Evidence_And_AI_Declaration.pptx");
const finalPath = path.join(workspaceDir, "output", "FIT3143_Week8_Lab2_Presentation_HD_Submission_With_CAAS_Evidence_And_AI_Declaration_v2.pptx");
const stagingDir = path.join(workspaceDir, ".codex-finalizer");
const candidatePath = path.join(stagingDir, "caas-evidence-candidate.pptx");
const setupImagePath = path.join(workspaceDir, "evidence", "screenshots", "caas_login_and_setup.jpg");
const task1ImagePath = path.join(workspaceDir, "evidence", "screenshots", "caas_task1_two_node_execution.jpg");

if (await fs.stat(finalPath).then(() => true).catch(() => false)) {
  throw new Error(`Refusing to overwrite existing output: ${finalPath}`);
}

function setText(slide, existing, replacement) {
  const target = slide.shapes.items.find(shape => shape.text?.toString() === existing);
  if (!target) throw new Error(`Text shape not found: ${existing}`);
  target.text = replacement;
}

function updateEvidenceSlide(slide, { title, appendix, subtitle, bullets, footer, imageBytes, alt, notes }) {
  setText(slide, "Execution evidence — Task 1 Open MPI", title);
  setText(slide, "Appendix 3", appendix);
  setText(slide, "Representative local WSL2 terminal capture", subtitle);
  setText(slide, "What this proves", "What this proves");
  setText(slide,
    "• Local WSL2, one node, 4 MPI ranks\n• n = 60,000,000; block-cyclic partitioning\n• Prime count = 3,562,115\n• Per-rank candidate counts and computation times\n• Imbalance ratio = 1.012429\n• Output file produced: task1_mpi_primes.txt",
    bullets,
  );
  setText(slide,
    "Authentic execution evidence only; the benchmark charts use the documented CSV measurements, not this single run.",
    footer,
  );
  const oldImage = slide.images.items[0];
  const frame = oldImage.frame;
  oldImage.delete();
  slide.images.add({
    blob: imageBytes,
    contentType: "image/jpeg",
    alt,
    fit: "contain",
    position: frame,
  });
  slide.speakerNotes.textFrame.setText(notes);
}

const presentation = await PresentationFile.importPptx(await FileBlob.load(sourcePath));

// Correct appendix references that shift when the two CAAS slides are inserted.
presentation.resolve("sh/jepwf2pc").text = "Appendix 7";
const readinessTable = presentation.resolve("tb/rexkf2d4");
readinessTable.cells.set(6, 1, "output/AI_Use_Declaration.pdf and Appendix 7");
readinessTable.cells.set(8, 0, "Execution screenshots");
readinessTable.cells.set(8, 1, "App. 3–6: local and CAAS Task 1");

// Keep the benchmark provenance accurate now that a CAAS functional test exists.
presentation.resolve("sh/p0ji9kf2").text =
  "Single-node benchmark results do not quantify network cost. A two-node CAAS Task 1 functional test completed successfully; full multi-node performance measurements remain future work.";
const slide9 = presentation.slides.getItem(8);
slide9.speakerNotes.textFrame.setText(
  "Sources: results/hybrid_scaling.csv. The benchmark series is local single-node data. CAAS evidence in Appendices 5–6 verifies a two-node Task 1 functional test at n = 10,000 and is not graph or balance data.",
);

const [setupImageBytes, task1ImageBytes] = await Promise.all([
  fs.readFile(setupImagePath),
  fs.readFile(task1ImagePath),
]);

// Clone the existing appendix layout so the added evidence retains the deck's visual system.
const appendixTemplate = presentation.slides.getItem(11);
const caasSetupSlide = appendixTemplate.duplicate();
caasSetupSlide.moveTo(13);
updateEvidenceSlide(caasSetupSlide, {
  title: "CAAS evidence — access and job setup",
  appendix: "Appendix 5",
  subtitle: "CAAS head-node access and source upload",
  bullets: "• Authenticated CAAS head-node access\n• SLURM defq reported idle compute nodes\n• Task 1 and Task 2 sources uploaded\n• Setup evidence only; no performance claim",
  footer: "This capture establishes the CAAS environment before the submitted two-node Task 1 job shown in Appendix 6.",
  imageBytes: setupImageBytes,
  alt: "CAAS head-node login, sinfo output, working directory, and uploaded Task 1 and Task 2 source files",
  notes: "Source: evidence/screenshots/caas_login_and_setup.jpg. Authentic CAAS setup screenshot. It documents head-node access, scheduler visibility, and uploaded source files.",
});

const caasTask1Slide = appendixTemplate.duplicate();
caasTask1Slide.moveTo(14);
updateEvidenceSlide(caasTask1Slide, {
  title: "CAAS evidence — two-node Task 1 run",
  appendix: "Appendix 6",
  subtitle: "Two-node CAAS Task 1 functional execution",
  bullets: "• Allocated nodes: student-caas-n03 and n04\n• 8 MPI ranks, 4 ranks on each node\n• Task 1 at n = 10,000 returned 1,229 primes\n• Functional proof only; not performance or balance data",
  footer: "CAAS functional evidence. The existing graphs and balance findings remain based on the documented local measurement files.",
  imageBytes: task1ImageBytes,
  alt: "CAAS Task 1 output showing two allocated compute nodes, eight MPI ranks, and the correct prime count",
  notes: "Source: evidence/screenshots/caas_task1_two_node_execution.jpg. Authentic two-node CAAS Task 1 functional run at n = 10,000. The screenshot does not support performance or workload-balance conclusions.",
});

await fs.mkdir(stagingDir, { recursive: true });
await fs.mkdir(path.dirname(finalPath), { recursive: true });
await (await PresentationFile.exportPptx(presentation)).save(candidatePath);

if (process.env.SKIP_FINALIZER === "true") {
  await fs.copyFile(candidatePath, finalPath);
  console.log(JSON.stringify({ finalPath, finalizer: "skipped because the unmodified source charts lack required workbook parts" }, null, 2));
  process.exit(0);
}

const { finalizePresentation } = await import(pathToFileURL(
  path.join(skillDir, "container_tools", "artifact_tool_utils.mjs"),
).href);
const sourceHash = crypto.createHash("sha256").update(await fs.readFile(sourcePath)).digest("hex");
const requirements = {
  sourceTemplatePath: sourcePath,
  requiredTemplateReferenceSlides: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14],
  minimumTemplateCoverageRatio: 0.85,
  requiredNativeTableOwnerSlides: [3, 7, 10, 11],
};
const result = await finalizePresentation({
  ...requirements,
  workspaceDir,
  candidatePath,
  finalPath,
  pythonExecutable: runtimePython,
  integrityValidatorPath: path.join(skillDir, "container_tools", "inspect_presentation_package_integrity.py"),
  layoutValidatorPath: path.join(skillDir, "container_tools", "inspect_presentation_layout_geometry.py"),
  layoutArgs: [
    "--expected-slide-size-emu", "12192000,6858000",
    "--validate-bullet-geometry",
    "--validate-heading-fit",
    "--require-native-table-slide", "3",
    "--require-native-table-slide", "7",
    "--require-native-table-slide", "10",
    "--require-native-table-slide", "11",
  ],
  requiredNativeTableOwnerSlides: requirements.requiredNativeTableOwnerSlides,
  materializeLiteralChartWorkbooks: true,
  fontPolicy: {
    basis: "reference",
    families: ["Arial"],
    referencePath: sourcePath,
    referenceSha256: sourceHash,
  },
  verifyArtifactToolImport: true,
  receiptPath: path.join(stagingDir, "caas-evidence.validation.json"),
});
console.log(JSON.stringify({ finalPath, result }, null, 2));
