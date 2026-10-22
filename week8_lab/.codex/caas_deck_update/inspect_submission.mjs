import path from 'node:path';
import { FileBlob, PresentationFile } from '@oai/artifact-tool';
const pptx=path.join(process.cwd(),'submission_ready','FIT3143_Week8_Lab2_Presentation_HD_Submission_With_CAAS_Evidence_And_AI_Declaration.pptx');
const p=await PresentationFile.importPptx(await FileBlob.load(pptx));
console.log((await p.inspect({kind:'slide,textbox,image,table,notes',maxChars:50000})).ndjson);
console.log((await p.help('delete slide',{maxChars:10000})).text);
