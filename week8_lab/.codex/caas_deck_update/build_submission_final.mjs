import fs from "node:fs/promises";
import path from "node:path";
import crypto from "node:crypto";
import { pathToFileURL } from "node:url";
import { FileBlob, PresentationFile } from "@oai/artifact-tool";

const workspaceDir = process.cwd();
const skillDir = "C:/Users/savin/.codex/plugins/cache/openai-primary-runtime/presentations/26.909.12148/skills/presentations";
const runtimePython = "C:/Users/savin/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe";
const sourcePath = path.join(workspaceDir, "submission_ready", "FIT3143_Week8_Lab2_Presentation_HD_Submission_With_CAAS_Evidence_And_AI_Declaration.pptx");
const finalPath = path.join(workspaceDir, "output", "final", "FIT3143_Week8_Lab2_Presentation_Submission.pptx");
const submissionPath = path.join(workspaceDir, "submission_ready", "FIT3143_Week8_Lab2_Presentation_Submission.pptx");
const stagingDir = path.join(workspaceDir, ".codex-finalizer");
const candidatePath = path.join(stagingDir, "week8-submission-final-candidate.pptx");
const screenshotSource = "C:/Users/savin/AppData/Local/Temp/codex-clipboard-d2b5816b-493b-4757-bd10-01f5cdd72242.png";
const screenshotEvidencePath = path.join(workspaceDir, "evidence", "screenshots", "caas_task1_two_node_execution_submitted.png");
const screenshotSubmissionPath = path.join(workspaceDir, "submission_ready", "supporting", "evidence", "screenshots", "caas_task1_two_node_execution_submitted.png");

for (const output of [finalPath, submissionPath]) {
  if (await fs.stat(output).then(() => true).catch(() => false)) {
    throw new Error(`Refusing to overwrite existing output: ${output}`);
  }
}

await fs.mkdir(path.dirname(screenshotEvidencePath), { recursive: true });
await fs.mkdir(path.dirname(screenshotSubmissionPath), { recursive: true });
await Promise.all([
  fs.copyFile(screenshotSource, screenshotEvidencePath),
  fs.copyFile(screenshotSource, screenshotSubmissionPath),
]);

const presentation = await PresentationFile.importPptx(await FileBlob.load(sourcePath));

function setText(anchorId, value) {
  presentation.resolve(anchorId).text = value;
}

// Remove the two redundant local-terminal evidence slides. The benchmark data
// remains in the presentation, while the appendix now focuses on CAAS evidence.
presentation.resolve("sl/hgzapcr6").delete();
presentation.resolve("sl/8f2psfyx").delete();

// Correct the remaining appendix references after the local screenshots are removed.
setText("sh/jepwf2pc", "Appendix 3");
setText("sh/x4ni9ofy", "This capture establishes the CAAS environment before the submitted two-node Task 1 job shown in Appendix 4.");
setText("sh/76p4jqls", "Appendix 4");
setText("sh/ytkf2h8j", "Appendix 5");
setText("sh/p0ji9kf2", "Single-node benchmark results do not quantify network cost. A two-node CAAS Task 1 functional test completed successfully; full multi-node performance measurements remain future work.");

// Replace the earlier copy with the screenshot supplied in this conversation.
const caasRunSlide = presentation.resolve("sl/vaxsvy10");
const oldImage = presentation.resolve("im/wbalg3a1");
oldImage.delete();
caasRunSlide.images.add({
  blob: await fs.readFile(screenshotEvidencePath),
  contentType: "image/png",
  alt: "Submitted CAAS Task 1 output showing two allocated nodes, eight MPI ranks, correct prime count, rank balance, and output file",
  fit: "contain",
  position: { left: 58, top: 152, width: 650, height: 470 },
});
caasRunSlide.speakerNotes.textFrame.setText(
  "Source: evidence/screenshots/caas_task1_two_node_execution_submitted.png. Authentic two-node CAAS Task 1 functional run at n = 10,000. It verifies allocation of student-caas-n03 and student-caas-n04, eight MPI ranks, the correct 1,229-prime result, and sorted output generation. It does not support the local benchmark or balance conclusions.",
);

// Keep the disclosure in the submitted presentation; no separate declaration PDF is requested.
setText("sh/zutgvm94", "Included in this presentation");
presentation.resolve("sl/gny5sjyp").speakerNotes.textFrame.setText(
  "This appendix declares the use of generative AI within the presentation itself. The assessment specification also requires the team to upload an authentic prompt/conversation record if AI was used. No separate AI declaration PDF is included.",
);

// Ensure the readiness slide identifies the selected submission files accurately.
const readiness = presentation.resolve("tb/rexkf2d4");
readiness.cells.set(3, 1, "FIT3143_Week8_Lab2_Presentation_Submission.pptx");
readiness.cells.set(4, 0, "Supporting scripts");
readiness.cells.set(4, 1, "supporting/scripts/ (retained for inspection)");
readiness.cells.set(4, 2, "Optional upload");
readiness.cells.set(5, 0, "Supporting results");
readiness.cells.set(5, 1, "supporting/results/ and evidence/ (retained for inspection)");
readiness.cells.set(5, 2, "Optional upload");
readiness.cells.set(6, 0, "AI use declaration");
readiness.cells.set(6, 1, "Appendix 5 in this presentation");
readiness.cells.set(6, 2, "Ready");
readiness.cells.set(7, 0, "CAAS screenshots");
readiness.cells.set(7, 1, "Appendices 3-4: setup and two-node Task 1");
readiness.cells.set(7, 2, "Ready");
presentation.resolve("sl/jetc3ut0").speakerNotes.textFrame.setText(
  "Required Moodle files are task1.c, task2.c, and the presentation. Supporting scripts, results, CSVs, and screenshots are retained for inspection but are not listed as compulsory uploads in the assessment specification. The prompt/conversation record must still be exported authentically if AI was used.",
);
presentation.resolve("sl/udsvah03").speakerNotes.textFrame.setText(
  "Sources: results/hybrid_scaling.csv. The benchmark series is local single-node data. CAAS evidence in Appendices 3-4 verifies setup and a two-node Task 1 functional test at n = 10,000; it is not graph or balance data.",
);

await fs.mkdir(stagingDir, { recursive: true });
await fs.mkdir(path.dirname(finalPath), { recursive: true });
await (await PresentationFile.exportPptx(presentation)).save(candidatePath);

const { finalizePresentation } = await import(pathToFileURL(
  path.join(skillDir, "container_tools", "artifact_tool_utils.mjs"),
).href);
const sourceHash = crypto.createHash("sha256").update(await fs.readFile(sourcePath)).digest("hex");
await finalizePresentation({
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
  explicitTotalSlideCount: 14,
  requiredNativeTableOwnerSlides: [3, 7, 10, 11],
  materializeLiteralChartWorkbooks: true,
  fontPolicy: {
    basis: "reference",
    families: ["Arial"],
    referencePath: sourcePath,
    referenceSha256: sourceHash,
  },
  verifyArtifactToolImport: true,
  receiptPath: path.join(stagingDir, "week8-submission-final.validation.json"),
});
await fs.copyFile(finalPath, submissionPath);
console.log(JSON.stringify({ finalPath, submissionPath, screenshotSubmissionPath }, null, 2));
