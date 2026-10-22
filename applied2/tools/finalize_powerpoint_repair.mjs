import fs from 'node:fs/promises';
import path from 'node:path';
import crypto from 'node:crypto';
import {pathToFileURL} from 'node:url';
const root=path.resolve('applied2');
const dir=path.join(root,'.build/hd_review');
const skill=process.env.PRESENTATION_SKILL_DIR;
const {finalizePresentation}=await import(pathToFileURL(path.join(skill,'container_tools/artifact_tool_utils.mjs')).href);
const output=path.join(root,'.artifact-output',`Applied2_PowerPoint_Compatible.${Date.now()}.pptx`);
const receipt=path.join(dir,`compatibility.${Date.now()}.json`);
const source=path.join(dir,'latest-source.pptx');
await finalizePresentation({workspaceDir:path.dirname(root),candidatePath:path.join(dir,'ownership-candidate.pptx'),finalPath:output,
 pythonExecutable:process.env.RUNTIME_PYTHON,
 integrityValidatorPath:path.join(skill,'container_tools/inspect_presentation_package_integrity.py'),
 layoutValidatorPath:path.join(skill,'container_tools/inspect_presentation_layout_geometry.py'),
 layoutArgs:['--expected-slide-size-emu','12192000,6858000','--validate-heading-fit'],
 requiredNativeChartOwnerSlides:[7,14],materializeLiteralChartWorkbooks:false,
 fontPolicy:{basis:'reference',families:['Arial','Consolas'],referencePath:source,
 referenceSha256:crypto.createHash('sha256').update(await fs.readFile(source)).digest('hex')},
 verifyArtifactToolImport:true,receiptPath:receipt});
await fs.copyFile(receipt,path.join(dir,'compatibility.latest.json'));
await fs.writeFile(path.join(dir,'compatible-output.json'),JSON.stringify({output,receipt},null,2));
console.log('Finalized compatible candidate for native PowerPoint verification.');
