import fs from 'node:fs/promises';
import path from 'node:path';
import crypto from 'node:crypto';
import {pathToFileURL} from 'node:url';
const root=path.resolve('applied2');
const dir=path.join(root,'.build/hd_review');
const skill=process.env.PRESENTATION_SKILL_DIR;
const {finalizePresentation}=await import(pathToFileURL(path.join(skill,'container_tools/artifact_tool_utils.mjs')).href);
const {PresentationFile,FileBlob}=await import(pathToFileURL(path.join(process.env.RUNTIME_NODE_MODULES,'@oai/artifact-tool/dist/artifact_tool.mjs')).href);
const output=path.join(root,'.artifact-output',`Applied2_HD_Reviewed.${Date.now()}.pptx`);
const receipt=path.join(dir,`validation.${Date.now()}.json`);
await finalizePresentation({workspaceDir:path.dirname(root),candidatePath:path.join(dir,'merged-candidate.pptx'),finalPath:output,
 pythonExecutable:process.env.RUNTIME_PYTHON,integrityValidatorPath:path.join(skill,'container_tools/inspect_presentation_package_integrity.py'),
 layoutValidatorPath:path.join(skill,'container_tools/inspect_presentation_layout_geometry.py'),
 layoutArgs:['--expected-slide-size-emu','12192000,6858000','--validate-heading-fit'],
 requiredNativeChartOwnerSlides:[7,14],materializeLiteralChartWorkbooks:false,
 fontPolicy:{basis:'reference',families:['Arial','Consolas'],referencePath:path.join(dir,'latest-source.pptx'),referenceSha256:crypto.createHash('sha256').update(await fs.readFile(path.join(dir,'latest-source.pptx'))).digest('hex')},
 verifyArtifactToolImport:true,receiptPath:receipt});
const deck=await PresentationFile.importPptx(await FileBlob.load(output));
for(const [i,s] of deck.slides.items.entries()){
 for(const [scale,suffix] of [[1,''],[2,'.print']]){
  const png=await s.export({format:'png',scale});
  await fs.writeFile(path.join(root,'.build/slides',`slide-${String(i+1).padStart(2,'0')}${suffix}.png`),new Uint8Array(await png.arrayBuffer()));
 }
 console.log(`Rendered ${i+1}/${deck.slides.items.length}`);
}
const source=path.join(root,'submission/Applied2_Combined_Presentation.pptx');
const sha=p=>fs.readFile(p).then(b=>crypto.createHash('sha256').update(b).digest('hex'));
if(await sha(source)!==await sha(path.join(dir,'latest-source.pptx')))throw new Error('Submitted source changed during review; validated result retained for merging.');
try{await fs.copyFile(output,source);}catch(e){
 if(e.code==='EBUSY')await fs.copyFile(output,path.join(root,'submission/Applied2_Combined_Presentation_HD_Reviewed.pptx'));
 throw e;
}
await fs.copyFile(output,path.join(root,'Applied2_Combined_Presentation.pptx'));
await fs.copyFile(path.join(dir,'updated-manifest.json'),path.join(root,'.build/deck_manifest.json'));
await fs.copyFile(receipt,path.join(root,'.build/Applied2_Combined_Presentation.validation.latest.json'));
await fs.copyFile(receipt,path.join(dir,'validation.latest.json'));
console.log('Updated latest submitted deck and canonical presentation; preserved native evidence.');
