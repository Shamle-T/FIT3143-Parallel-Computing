// Author the changed main slides using the existing editable design system.
import fs from 'node:fs/promises';
import path from 'node:path';
import {mainSlides,appendixSlides,students,refs} from './deck_content.mjs';
process.env.SKIP_DECK_BUILD='1';
const {Presentation,PresentationFile,renderMain,renderAppendix,sourceFooter,C}=await import('./build_presentation.mjs');
const dir=path.resolve('applied2/.build/hd_review');
const deck=Presentation.create({slideSize:{width:1280,height:720}});
const kinds=new Set(['cover','transfer','rotation','cudabrief','frameworkbrief','gds','environment','governance']);
for(const [i,d] of mainSlides.entries()){
 const s=deck.slides.add();s.background.fill=C.bg;
 if(kinds.has(d.kind))renderMain(s,d);else renderAppendix(s,d);
 if(d.kind!=='cover')sourceFooter(s,d.citations,i,48);
 s.speakerNotes.text=`Target time: ${d.seconds} seconds.\n\n${d.narration}\n\n`+d.citations.map(n=>{const r=refs.find(r=>r.n===n);return `[${n}] ${r.text}\n${r.url}`;}).join('\n\n');
 const png=await s.export({format:'png',scale:1});
 await fs.writeFile(path.join(dir,`authored-${i+1}.png`),new Uint8Array(await png.arrayBuffer()));
}
await (await PresentationFile.exportPptx(deck)).save(path.join(dir,'authored-main.pptx'));
const base=JSON.parse(await fs.readFile(path.join(dir,'baseline_manifest.json'),'utf8'));
const old=base.slides;
const slides=[...mainSlides,old[11],
 {...old[3],id:'cuda-evidence',label:'Appendix • CUDA examples and execution detail'},
 {...old[6],id:'sector-evidence',label:'Appendix • Sector charts and scope'},
 ...old.slice(12)];
await fs.writeFile(path.join(dir,'updated-manifest.json'),JSON.stringify({mainCount:11,slides:slides.map((d,i)=>({...d,number:i+1})),students,refs},null,2));
console.log('Authored 11 main slides; retained full supporting material in 48-slide manifest.');
