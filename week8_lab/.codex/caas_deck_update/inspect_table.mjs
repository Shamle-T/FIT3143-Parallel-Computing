import path from 'node:path';
import { FileBlob, PresentationFile } from '@oai/artifact-tool';
const p=await PresentationFile.importPptx(await FileBlob.load(path.join(process.cwd(),'submission_ready','FIT3143_Week8_Lab2_Presentation_HD_Submission_With_CAAS_Evidence_And_AI_Declaration.pptx')));
const t=p.resolve('tb/rexkf2d4');
for(let r=0;r<8;r++){let a=[];for(let c=0;c<3;c++)a.push(t.getCell(r,c).text.toString());console.log(r,JSON.stringify(a));}
