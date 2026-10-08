import fs from 'node:fs/promises';
import path from 'node:path';
import { pathToFileURL, fileURLToPath } from 'node:url';
import { students, refs, mainSlides, appendixSlides } from './deck_content.mjs';

const taskDir = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const workspaceDir = path.dirname(taskDir);
const deps = process.env.RUNTIME_NODE_MODULES;
if (!deps) throw new Error('Set RUNTIME_NODE_MODULES to the bundled runtime packages.');
const { Presentation, PresentationFile } = await import(pathToFileURL(path.join(deps, '@oai/artifact-tool/dist/artifact_tool.mjs')).href);
const { createCanvas, GlobalFonts } = await import(pathToFileURL(path.join(deps, '@napi-rs/canvas/index.js')).href);
const SKILL_DIR = process.env.PRESENTATION_SKILL_DIR;
const { finalizePresentation, applyPresentationChartFont } = await import(pathToFileURL(path.join(SKILL_DIR, 'container_tools/artifact_tool_utils.mjs')).href);
const buildDir = path.join(taskDir, '.build');
const outDir = path.join(taskDir, '.artifact-output');
await fs.mkdir(buildDir, { recursive: true });
await fs.mkdir(outDir, { recursive: true });
const fontNames = GlobalFonts.families.map(x => x.family);
for (const name of ['Arial', 'Consolas']) if (!fontNames.includes(name)) throw new Error(`Required available font missing: ${name}`);

const C = { bg: '#FCFCFA', ink: '#142D45', teal: '#007B83', orange: '#B55E24', grey: '#516170', rule: '#CBD5DC', pale: '#EEF5F5', code: '#EDF1F5', white: '#FFFFFF' };
const ctx = createCanvas(1280, 720).getContext('2d');
const boxes = [];
function wrap(value, w, size, font = 'Arial', bold = false) {
  ctx.font = `${bold ? 'bold ' : ''}${size}px ${font}`;
  return value.split('\n').flatMap(p => {
    if (!p) return [''];
    const lines = []; let current = '';
    for (const word of p.split(/\s+/)) {
      const next = current ? `${current} ${word}` : word;
      if (current && ctx.measureText(next).width > w - 3) { lines.push(current); current = word; }
      else current = next;
    }
    if (current) lines.push(current);
    return lines;
  }).join('\n');
}
function text(s, value, x, y, w, size = 24, options = {}) {
  const font = options.font ?? 'Arial';
  const str = options.code ? value : wrap(value, w, size, font, options.bold);
  const height = options.h ?? (str.split('\n').length * size * 1.24 + 4);
  const shape = s.shapes.add({ geometry: 'textbox', name: options.name ?? value.slice(0, 55), position: { left:x, top:y, width:w, height }, fill:'none', line: {fill:'none', width:0} });
  shape.text = str;
  shape.text.style = { typeface:font, fontSize:size, bold:options.bold ?? false, color:options.color ?? C.ink, alignment:options.align ?? 'left', verticalAlignment:'top', wrap:options.code ? 'none' : 'square', autoFit:'none', insets:0 };
  if (options.link) shape.text.getRange(0, str.length).link = { uri:options.link, isExternal:true };
  boxes.push({ slide:s.index, value, x,y,w,height,size,font });
  return height;
}
function rule(s, x,y,w,color=C.rule,width=1) { s.shapes.add({ geometry:'line', position:{left:x,top:y,width:w,height:0}, fill:'none', line:{fill:color,width,style:'solid'} }); }
function rect(s,x,y,w,h,fill=C.white,line=C.teal) { return s.shapes.add({geometry:'rect',position:{left:x,top:y,width:w,height:h},fill,line:{fill:line,width:1.5,style:'solid'}}); }
function node(s,label,x,y,w,h=100) {
  const sh=rect(s,x,y,w,h,C.white,C.teal);
  sh.text=wrap(label,w-24,23);
  sh.text.style={typeface:'Arial',fontSize:23,color:C.ink,alignment:'center',verticalAlignment:'middle',insets:10,autoFit:'none'};
  return sh;
}
function arrow(s,a,b,options={}) {
  return s.shapes.connect(a,b,{kind:options.kind??'straight',fromSide:options.from??'right',toSide:options.to??'left',line:{fill:options.color??C.teal,width:2,style:options.dashed?'dashed':'solid'},tail:{type:'arrow',width:'med',length:'med'}});
}
function code(s, value, x,y,w,size=21) {
  const h=value.split('\n').length * size * 1.2 + 24;
  rect(s,x-12,y-9,w+24,h,C.code,'none');
  text(s,value,x,y,w,size,{font:'Consolas',code:true,h:h-14});
  return h;
}
function paragraphs(s, values,x,y,w,size=23, gap=18) {
  let yy=y;
  for(const p of values) yy+=text(s,p,x,yy,w,size)+gap;
  return yy;
}
function sourceFooter(s, ids, index, total) {
  rule(s,64,664,1152);
  const label=ids.length ? 'Sources: '+ids.map(n=>`[${n}] ${refs.find(r=>r.n===n).short}`).join('; ') : 'FIT3143 Applied 2';
  text(s,label,64,676,1060,13.2,{color:C.grey,h:28});
  text(s,`${index+1} / ${total}`,1150,676,66,13.2,{color:C.grey,align:'right',h:22});
}
function heading(s,d) {
  text(s,d.label??'',64,30,1152,16,{color:C.teal,bold:true});
  text(s,d.title,64,68,1152,36,{bold:true,h:75});
}
function takeaway(s,value) { rule(s,64,597,1152,C.teal,2); text(s,value,64,611,1152,22,{bold:true,h:48}); }
function twoColumns(s,d) {
  let size=23;
  const height=(col,n)=> col.body.reduce((sum,v)=>sum+(wrap(v,538,n).split('\n').length*n*1.24+4)+18,0)+47;
  while(size>20 && Math.max(height(d.left,size),height(d.right,size))>478) size--;
  for(const [col,x] of [[d.left,64],[d.right,674]]) {
    text(s,col.heading,x,162,538,27,{bold:true,color:C.teal});
    const end=paragraphs(s,col.body,x,207,538,size,18);
    if(end>660) throw new Error(`Column overflow on ${d.id}: ${end}`);
  }
  rule(s,639,162,1,C.rule);
}
function nativeTable(s,d) {
  const count=d.rows.length+1;
  const tableHeight=d.tableHeight??406;
  const t=s.tables.add({rows:count,columns:3,left:64,top:164,width:1152,height:tableHeight,columnWidths:[235,435,482],values:[d.columns,...d.rows]});
  t.borders.assign({fill:C.rule,width:0.8,style:'solid'});
  t.styleOptions={headerRow:true,bandedRows:false};
  for(let r=0;r<count;r++) {
    t.rows[r].height=r===0?56:(tableHeight-56)/d.rows.length;
    for(let c=0;c<3;c++) {
      const cell=t.getCell(r,c); cell.fill=r===0?C.ink:(r%2?C.white:'#F0F4F4');
      cell.text.style={typeface:'Arial',fontSize:r===0?22:20,color:r===0?C.white:C.ink,bold:r===0,insets:10,verticalAlignment:'middle',alignment:'left',autoFit:'none'};
    }
  }
  text(s,d.note,64,601,1152,20,{color:C.grey,h:58});
}

const kernel = await fs.readFile(path.join(taskDir,'task1/Task1_CUDA_Kernel_Examples.cu'),'utf8');
const perf = await fs.readFile(path.join(taskDir,'task1/Task1_CUDA_Performance_Examples.cu'),'utf8');
function functionText(src,name) { const start=src.indexOf(name); if(start<0)throw new Error(name); const end=src.indexOf('\n}',start);return src.slice(start,end+2).replaceAll('\r',''); }
const br=functionText(kernel,'__global__ void brightness_rgb');
const rot=functionText(kernel,'__global__ void rotate_rgb').split('\n');
const rotationSplit=rot.findIndex(x=>x.includes('const size_t destination'));
const snippets={ brightness:br, rotation1:rot.slice(0,rotationSplit).join('\n'), rotation2:rot.slice(rotationSplit).join('\n'), async:functionText(perf,'cudaError_t enqueue_rotation') };

function renderMain(s,d) {
  if(d.kind==='cover') {
    s.background.fill=C.ink;
    text(s,d.subtitle,64,65,1140,20,{color:'#8CD7D5',bold:true});
    text(s,d.title,64,164,1140,54,{color:C.white,bold:true,h:155});
    text(s,d.takeaway,64,337,1140,26,{color:'#CCDCE4'});
    rule(s,64,430,1152,'#577082');
    students.forEach((p,i)=>{
      const y=465+i*82;
      text(s,`${p.name}  ·  ${p.id}`,64,y,1140,26,{color:C.white,bold:true});
      text(s,p.email,64,y+36,1140,22,{color:'#C8D9E3'});
    });
    text(s,'8 October 2026  •  Focused answers first; full implementation and evidence in the appendix',64,671,1140,15,{color:'#C8D9E3'});
    return;
  }
  heading(s,d);
  if(d.kind==='transfer') {
    text(s,'CPU: load/decode, allocate, launch, save',64,160,550,25,{bold:true});
    text(s,'GPU: read input, rotate, write output',664,160,552,25,{bold:true});
    const labels=['Storage\nencoded image','Host RAM\nDDR example\ninput pixels','GPU memory\nGDDR / HBM\ninput buffer','GPU memory\nGDDR / HBM\noutput buffer','Host RAM\noutput pixels'];
    const nodes=labels.map((v,i)=>node(s,v,64+i*244,240,176,142));
    nodes.slice(0,-1).forEach((a,i)=>arrow(s,a,nodes[i+1]));
    ['CPU\ndecode','H2D\nPCIe','CUDA\nkernel','D2H\nPCIe'].forEach((v,i)=>text(s,v,224+i*244,192,104,18,{align:'center',color:C.teal,h:47}));
    paragraphs(s,[
      'H2D: host → device. D2H: device → host. Separate buffers; DMA (direct memory access) engines transfer bytes over PCIe.',
      'GPU memory bandwidth differs from PCIe bandwidth. Pinned RAM supports DMA; pageable RAM may stage. Complete output before saving or reuse.',
    ],64,432,1152,24,16);
  } else if(d.kind==='rotation') {
    text(s,'Inverse mapping in y-down image coordinates',64,159,690,27,{bold:true,color:C.teal});
    code(s,'sx = c*(x-cx) - s*(y-cy) + cx;\nsy = s*(x-cx) + c*(y-cy) + cy;\nix = floor(sx + 0.5);\niy = floor(sy + 0.5);',76,220,690,23);
    text(s,'c = cos θ; s = sin θ; θ in radians; center = ((W−1)/2, (H−1)/2)',64,375,810,21);
    const out=node(s,'Output pixel\n(x,y)',930,197,245,93);
    const inp=node(s,'Nearest input\n(ix,iy)',930,357,245,93);
    arrow(s,out,inp,{from:'bottom',to:'top'});
    text(s,'lookup',1072,307,112,21,{color:C.teal});
    paragraphs(s,['Forward [c s; −s c]; inverse [c −s; s c]. Positive θ is visually counterclockwise.', 'Fixed RGB canvas can crop; black borders. Gathering avoids scatter holes; nearest-neighbour can alias.'],64,440,810,22,16);
  } else if(d.kind==='cudabrief') {
    text(s,'Host launch + device pixel indexing',64,155,686,26,{bold:true,color:C.teal});
    code(s,'dim3 block(16,16), grid(64,64);\nrotate_rgb<<<grid,block>>>(\n    d_in,d_out,1024,1024,c,s);\nint x = blockIdx.x*blockDim.x + threadIdx.x;\nint y = blockIdx.y*blockDim.y + threadIdx.y;\nif (x >= W || y >= H) return;',76,206,690,20);
    const a=node(s,'Grid: 64 × 64 blocks',842,165,374,63);
    const b=node(s,'Block: 16 × 16 threads\n256 threads = 8 warps',842,278,374,82);
    const c=node(s,'SMs schedule resident warps\n32 threads/warp • SIMT',842,410,374,82);
    arrow(s,a,b,{from:'bottom',to:'top'});arrow(s,b,c,{from:'bottom',to:'top'});
    text(s,'Second kernel feature: clamp a brightness channel',64,393,686,23,{bold:true,color:C.teal});
    code(s,'int v = int(in[p]) + delta;\nout[p] = v<0 ? 0 : (v>255 ? 255 : v);',76,440,690,20);
    text(s,'After bounds checks: p = 3*(size_t(y)*W+x)+ch; ch ∈ [0,2]; delta ∈ [−255,255]. Separate RGB buffers. Uncompiled excerpts.',64,523,710,19);
    text(s,'Residency hides latency; registers and divergence limit execution.',842,523,374,21);
  } else if(d.kind==='frameworkbrief') {
    const a=node(s,'Map\nUse, stakeholders, alternatives',64,178,312,102);
    const b=node(s,'Measure\nQuality, energy, security, harms',484,178,312,102);
    const c=node(s,'Manage\nApprove / defer / redesign',904,178,312,102);
    arrow(s,a,b);arrow(s,b,c);
    text(s,'Govern throughout: accountable owner, rights, data rules and equitable access',64,321,1152,26,{bold:true,color:C.teal});
    paragraphs(s,[
      'Green AI [10]: efficiency per useful outcome. OECD [15] + NIST [16]: rights, safeguards and review.',
      'Carbon-aware scheduling [14]: shift flexible jobs within deadlines, water/site limits and equitable waiting commitments.',
      'Record exceptions and outcomes; urgent work and forecast uncertainty can override deferral. Prevent starvation.',
    ],64,379,1152,23,14);
  } else if(d.kind==='launch') {
    code(s,'dim3 block(16,16);\ndim3 grid(64,64);\nrotate_rgb<<<grid,block>>>(\n  d_in,d_out,1024,1024,c,s);',76,168,600,22);
    code(s,'x = blockIdx.x*blockDim.x + threadIdx.x;\ny = blockIdx.y*blockDim.y + threadIdx.y;\nif (x >= W || y >= H) return;',76,347,600,21);
    text(s,'1024 × 1024 output',760,164,456,28,{bold:true,color:C.teal});
    paragraphs(s,['64 × 64 = 4,096 blocks', '16 × 16 = 256 threads/block', '1,048,576 logical threads', '32 threads/warp → 8 warps/block'],760,219,456,25,16);
    text(s,'One highlighted warp within a 16-wide block',760,423,456,20,{color:C.grey});
    for(let i=0;i<32;i++)rect(s,760+(i%16)*25,463+Math.floor(i/16)*25,20,20,C.teal,C.teal);
    text(s,'Non-multiple dimensions: ceil-divide the grid and keep the bounds guard.',64,540,1152,23);
  } else if(d.kind==='features') {
    text(s,'Example: independent brightness channels',64,164,655,27,{bold:true,color:C.teal});
    code(s,'size_t p = 3*(size_t(y)*W + x);\nfor (int ch=0; ch<3; ++ch) {\n  int v = int(in[p+ch]) + delta;\n  out[p+ch] = v<0 ? 0 : (v>255 ? 255 : v);\n}',76,221,650,21);
    text(s,'After coordinate/bounds checks; valid RGB buffers; delta ∈ [−255,255].',64,376,664,19,{color:C.grey,h:51});
    const pending=node(s,'Pending blocks',860,172,306,70);
    const sm=node(s,'Streaming multiprocessor\nresident warps → execution units',820,315,385,116);
    arrow(s,pending,sm,{from:'bottom',to:'top'});
    text(s,'SIMT = single instruction, multiple threads. A warp issues common instructions; divergent paths can reduce efficiency.',810,451,406,24);
    text(s,'Resident warps hide latency; registers limit residency. Coalescing groups nearby accesses into fewer transactions; rotation can disrupt read locality.',64,458,684,24);
  } else if(d.kind==='speed') {
    text(s,'Baseline: single CPU thread, same rotation and sampling',64,159,1152,27,{bold:true,color:C.teal});
    text(s,'Also compare a multithreaded CPU; identical output and timing boundaries.',64,202,1152,24);
    text(s,'Speedup = TCPU / TGPU,total',64,263,1152,34,{bold:true});
    text(s,'TGPU,total = allocation/setup + H2D + kernel + D2H',64,319,1152,27);
    text(s,'Many GPU execution units and high device bandwidth favor parallel throughput. Large or resident images amortize setup; gathered reads may limit bandwidth.',64,390,548,24);
    text(s,'Amdahl: S = 1 / (s + (1−s)/a). s is the unchanged serial share; a accelerates the remainder. s = 0.20 gives a 5× ideal ceiling before added transfers. Theoretical, not measured.',674,390,542,24);
  } else if(d.kind==='streamcore') {
    text(s,'Ordered work in one stream',64,158,650,27,{bold:true,color:C.teal});
    code(s,'cudaMemcpyAsync(d_in, h_in, bytes,\n  cudaMemcpyHostToDevice, stream);\nrotate_rgb<<<grid, block, 0, stream>>>(\n  d_in, d_out, W, H, c, s);\ncudaMemcpyAsync(h_out, d_out, bytes,\n  cudaMemcpyDeviceToHost, stream);\n// After enqueueing all batch images:\ncudaStreamSynchronize(stream);',76,216,650,21);
    text(s,'Batch: enqueue all streams before waiting. Check every API, launch and completion result; submission is not completion.',64,437,650,22);
    text(s,'Independent images / streams',790,158,426,25,{bold:true,color:C.teal});
    for(const [i,label] of ['A','B'].entries()) {
      const x=800+i*35,y=220+i*126;
      text(s,label,755,y+21,34,21,{bold:true});
      const a=node(s,'H2D',x,y,102,68),b=node(s,'Kernel',x+137,y,108,68),c=node(s,'D2H',x+280,y,102,68);
      arrow(s,a,b);arrow(s,b,c);
    }
    text(s,'Possible overlap depends on supporting hardware and available resources.',790,451,426,21,{color:C.grey});
    text(s,'Separate pinned/device buffers and non-default streams; synchronize before reuse. More buffers cost memory. Keep images on the GPU across operations to avoid repeated transfers.',64,521,1152,23);
    takeaway(s,'Overlap can shorten the batch critical path; it changes scheduling, not total work.');
    return;
  } else if(d.kind==='gds') {
    text(s,'Conventional storage path',64,160,1152,26,{bold:true,color:C.teal});
    const a=node(s,'Storage',64,207,215,78),b=node(s,'Host staging RAM',464,207,310,78),c=node(s,'GPU memory',974,207,242,78);
    arrow(s,a,b);arrow(s,b,c);text(s,'storage read',288,216,160,18,{align:'center'});text(s,'H2D / PCIe',790,216,171,18,{align:'center'});
    text(s,'GDS direct path (supported configuration)',64,307,800,26,{bold:true,color:C.teal});
    const aa=node(s,'Storage-side DMA\nNVMe / NIC',64,350,260,86), cc=node(s,'GPU memory',974,350,242,86);arrow(s,aa,cc);
    text(s,'PCIe data path bypasses host staging',389,356,522,21,{align:'center'});
    text(s,'CPU issues cuFile I/O. Fallback may stage through RAM; GDS does not decode images or accelerate kernel arithmetic.',64,452,1152,22);
    text(s,'Use conventional copies for CPU-decoded / already-in-RAM pixels. Consider GDS for storage-bound GPU-ready batches only with supported GPU/filesystem/driver/topology and lower completed batch time.',64,517,1152,22);
  } else if(d.kind==='hpc') {
    const labels=['GPU 0\nsame model\ndata partition A','GPU 1\nsame model\ndata partition B','GPU 2\nsame model\ndata partition C'];
    const n=labels.map((v,i)=>node(s,v,64+i*420,182,312,118));
    node(s,'Gradient all-reduce: average + share → consistent update on every GPU',64,390,1152,80);
    [220,640,1060].forEach(x=>s.shapes.add({geometry:'connector',kind:'straight',position:{left:x,top:300,width:0,height:90},line:{fill:C.teal,width:2},tail:{type:'arrow',width:'med',length:'med'}}));
    text(s,'Interconnect:\nlatency and bandwidth',275,319,314,22,{color:C.teal,h:62});
    text(s,'Gradient synchronization\nbefore each update',696,319,320,22,{color:C.teal,h:62});
    text(s,'HPC combines processors, memory, networks and storage. Tensor/pipeline parallelism can also split a model.',64,499,548,23);
    text(s,'More GPUs enable larger work; communication, serial work and imbalance limit speedup. Evaluate useful output against resource cost.',674,499,542,23);
  } else if(d.kind==='environment') {
    text(s,'Global data-centre electricity (TWh/year)',64,151,552,23,{bold:true});
    text(s,'Electricity growth during 2025',664,151,552,23,{bold:true});
    const common={hasLegend:false,barOptions:{direction:'column',grouping:'clustered',gapWidth:120},dataLabels:{showValue:true,position:'outEnd',textStyle:{fontSize:23,typeface:'Arial',fill:C.ink}},chartFill:'none',chartLine:{fill:'none'},plotAreaFill:'none',plotAreaLine:{fill:'none'},xAxis:{textStyle:{fontSize:19,typeface:'Arial'},line:{fill:C.rule}},yAxis:{textStyle:{fontSize:18,typeface:'Arial'},majorGridlines:{fill:C.rule,width:1}}};
    const chart1=s.charts.add('bar',{...common,position:{left:64,top:197,width:552,height:299},categories:['2025 estimate','2030 projection'],series:[{name:'Electricity (TWh)',values:[485,950],fill:C.teal}],yAxis:{...common.yAxis,minimumScale:0,maximumScale:1000,numberFormatCode:'0'}});
    const chart2=s.charts.add('bar',{...common,position:{left:664,top:197,width:552,height:299},categories:['All data centres','AI-focused subset'],series:[{name:'2025 growth',values:[0.17,0.50],fill:C.orange}],yAxis:{...common.yAxis,minimumScale:0,maximumScale:0.6,numberFormatCode:'0%'},dataLabels:{...common.dataLabels,numberFormatCode:'0%'}});
    applyPresentationChartFont(chart1,{fontFamily:'Arial'}); applyPresentationChartFont(chart2,{fontFamily:'Arial'});
    text(s,'IEA 2026: ≈96% increase to 2030. These cover the whole sector, not AI training alone; growth categories overlap and cannot be added.',64,511,1152,21);
    text(s,'Research comparison: Green AI [10] emphasizes efficiency; water research [9] adds site/time constraints. Track energy, carbon and local cooling impacts.',64,565,1152,21);
  } else if(d.kind==='governance') {
    text(s,'Security: protect distributed intermediates',64,158,550,26,{bold:true,color:C.teal});
    text(s,'Fairness: evaluate affected groups',664,158,552,26,{bold:true,color:C.teal});
    const l1=node(s,'Workers exchange gradients\nand save checkpoints',64,211,548,92);
    const l2=node(s,'Least privilege, isolation,\nprotected transport/storage',64,360,548,92);arrow(s,l1,l2,{from:'bottom',to:'top'});
    const r1=node(s,'Data coverage and model outputs\nmay differ across groups',664,211,552,92);
    const r2=node(s,'Disaggregated, harm-specific tests\n+ mitigation and reevaluation',664,360,552,92);arrow(s,r1,r2,{from:'bottom',to:'top'});
    text(s,'Zhu et al. [11]: exposed gradients can reconstruct inputs in studied settings. Not every production collective leaks.',64,479,548,22);
    text(s,'Gallegos et al. [12]: metrics, datasets and mitigations differ. Apply these to cluster-trained models; scale alone does not guarantee fairness.',664,474,552,22);
    text(s,'Provenance/permissions, replica access and dual-use review need owners. Encryption alone cannot establish fair outputs.',64,568,1152,21);
  } else if(d.kind==='access') {
    const t=s.tables.add({rows:3,columns:3,left:64,top:165,width:1152,height:250,columnWidths:[300,430,422],values:[['Evidence','What it shows','What it cannot establish'],['Ahmed & Wahed [13]','Historical research participation and compute concentration','Current worldwide access or a universal causal effect'],['Strubell et al. [8]','Selected NLP workload costs and financial barriers','The cost of every current AI workload']]});
    t.borders.assign({fill:C.rule,width:1});t.styleOptions={headerRow:true};
    for(let r=0;r<3;r++){t.rows[r].height=r?97:56;for(let c=0;c<3;c++){const cell=t.getCell(r,c);cell.fill=r===0?C.ink:C.white;cell.text.style={typeface:'Arial',fontSize:23,bold:r===0,color:r===0?C.white:C.ink,insets:12,verticalAlignment:'middle'};}}
    text(s,'Current response: NAIRR supported >600 teams and >6,000 students',64,449,1152,28,{bold:true,color:C.teal});
    text(s,'NSF, March 2026; US programme since 2024. Reach does not prove global equality.',64,491,1152,22);
    text(s,'Proposal: transparent GPU-hour allocation, smaller-institution support and appeals. Trade-off: reserved access can reduce immediate utilization or delay urgent jobs.',64,538,1152,24);
  } else if(d.kind==='framework') {
    const labels=['Define outcome\n+ accountable owner','Measure quality,\nenergy and harms','Approve / defer /\nreduce scale / redesign'];
    const n=labels.map((v,i)=>node(s,v,64+i*420,184,312,108));n.slice(0,-1).forEach((a,i)=>arrow(s,a,n[i+1]));
    text(s,'Green AI: efficiency evidence [10]  •  Carbon-aware scheduling: flexible work [14]',64,333,1152,24,{bold:true,color:C.teal});
    text(s,'OECD: values and rights [15]  •  NIST: Govern, Map, Measure, Manage [16]',64,379,1152,24);
    paragraphs(s,['Use an energy budget, security/group evaluation and transparent access rules. Record owner, rationale, exceptions and outcomes.', 'Measurement informs action: shift flexible jobs to cleaner periods within deadlines/capacity. Forecast uncertainty, water stress and urgent access can change the decision.'],64,429,1152,24,13);
  }
  if(d.kind!=='environment') takeaway(s,d.takeaway);
}

function renderAppendix(s,d) {
  heading(s,d);
  if(d.kind==='columns') twoColumns(s,d);
  else if(d.kind==='table')nativeTable(s,d);
  else if(d.kind==='codecolumns') {
    code(s,d.code,76,166,650,19);
    text(s,d.explanation[0],64,449,650,21);
    paragraphs(s,d.explanation.slice(1),758,166,458,22,18);
  } else if(d.kind==='sourcecode') {
    const value=d.code??snippets[d.source];
    const size=value.split('\n').length>24?17.5:19;
    code(s,value,76,166,785,size);
    paragraphs(s,d.explanation,908,166,308,22,20);
  } else if(d.kind==='worked') {
    const input=Array.from({length:5},(_,y)=>Array.from({length:5},(_,x)=>String(y*5+x)));
    const output=Array.from({length:5},(_,y)=>Array.from({length:5},(_,x)=>String(x*5+4-y)));
    for(const [values,x,label] of [[input,64,'Input'],[output,794,'Output: 90° counterclockwise']]) {
      text(s,label,x,159,424,27,{bold:true,color:C.teal});
      const t=s.tables.add({rows:5,columns:5,left:x,top:212,width:422,height:300,values});
      t.borders.assign({fill:C.rule,width:1});
      for(let r=0;r<5;r++)for(let c=0;c<5;c++){const cell=t.getCell(r,c);cell.fill=(label==='Input'&&r===0&&c===3)||(label.startsWith('Output')&&r===1&&c===0)?'#C2E7E5':C.white;cell.text.style={typeface:'Arial',fontSize:27,color:C.ink,alignment:'center',verticalAlignment:'middle',insets:8};}
    }
    text(s,'θ = 90°\nc = 0, s = 1\ncenter = (2,2)',524,240,216,26,{align:'center'});
    text(s,'Output (0,1) → source (3,0) → value 3',64,550,1152,27,{bold:true});
    text(s,d.takeaway,64,605,1152,22,{color:C.grey});
  } else if(d.kind==='overlap') {
    text(s,'Same stream: H2D → kernel → D2H, in order',64,158,1152,27,{bold:true,color:C.teal});
    ['Stream A / image A','Stream B / image B'].forEach((label,i)=>{
      text(s,label,64,229+i*147,230,22,{bold:true});
      const x=330+i*160,y=212+i*147;
      const a=node(s,'H2D',x,y,177,78),b=node(s,'Kernel',x+226,y,177,78),c=node(s,'D2H',x+452,y,177,78);arrow(s,a,b);arrow(s,b,c);
    });
    text(s,'Schematic overlap opportunity; not measured durations or guaranteed simultaneous execution.',64,480,1152,22,{color:C.grey});
    text(s,'Independent streams + separate pinned/device buffers + supporting copy/compute resources. Synchronize before reuse. Extra buffers consume memory; device residency avoids unnecessary round trips.',64,528,1152,24);
    takeaway(s,d.takeaway);
  } else if(d.kind==='jobenergy') {
    const td={tableHeight:270,columns:['Assumed configuration','Completed runtime','Whole-job energy'],rows:[['Lower parallel scale; 1 kW','2.0 hours','1 × 2.0 = 2.0 kWh'],['Higher parallel scale; 2 kW','1.2 hours','2 × 1.2 = 2.4 kWh']],note:'Illustrative constant power assumptions, not measurements. Same useful output is assumed; faster completion uses 20% more energy here.'};
    nativeTable(s,td);
    text(s,'E = ∫ P(t) dt     •     Carbon ≈ Σ Einterval × carbon intensity',64,478,1152,28,{bold:true,color:C.teal});
  } else if(d.kind==='schedule') {
    const a=node(s,'Flexible job\nready',64,193,270,100),b=node(s,'Lower-carbon\nforecast window',474,193,308,100),c=node(s,'Complete before\ndeadline',934,193,282,100);arrow(s,a,b);arrow(s,b,c);
    paragraphs(s,['Research [14] demonstrates shifting temporally flexible workloads using forecasts while preserving daily capacity. It does not show every urgent GPU job can wait.', 'Proposed rule: defer only if deadline, energy budget, water/site limits and equitable waiting-time commitments remain satisfied; record exceptions.', 'No feasible window: reduce scale, redesign or approve a justified urgent exception. Prevent indefinite deferral. Data residency and transfer costs constrain moving work between sites.'],64,356,1152,25,19);
    takeaway(s,d.takeaway);
  }
}

function aiSlides() {
  return [{id:'ai-declaration',task:'both',kind:'columns',label:'Appendix • AI declaration',title:'AI use declaration',citations:[],
    left:{heading:'AI model and tool',body:[
      'OpenAI GPT-6, accessed through Codex.',
    ]},
    right:{heading:'How AI was used',body:[
      'Research and source checks; drafting and revision of explanations and CUDA teaching examples; creation of diagrams and charts; presentation formatting and assembly.',
    ]}}];
}

function referencesSlides(selected){
  const needed=new Set(selected.flatMap(x=>x.citations??[]));
  const rr=refs.filter(r=>needed.has(r.n));
  const pages=Array.from({length:Math.ceil(rr.length/4)},(_,i)=>rr.slice(i*4,i*4+4));
  if(pages.length>1&&pages.at(-1).length===1)pages.at(-2).push(...pages.pop());
  return pages.map((page,i)=>({id:`references-${i}`,task:'both',kind:'refs',title:'References',label:`Appendix • References ${i+1}/${pages.length}`,citations:[],refs:page}));
}

async function buildDeck(which) {
  const selectedMain=mainSlides.filter(d=>which==='combined'||d.task===which||d.task==='both').map(d=>d.id==='cover'?{...d,title:which==='t1'?'GPU image rotation':which==='t2'?'Responsible HPC for AI':d.title,subtitle:which==='combined'?d.subtitle:`FIT3143 • Applied 2 • ${which==='t1'?'Task 1':'Task 2'}`} :d);
  const selectedApp=appendixSlides.filter(d=>which==='combined'||d.task===which||d.task==='both');
  const slides=[...selectedMain,...selectedApp,...referencesSlides([...selectedMain,...selectedApp]),...aiSlides()];
  const presentation=Presentation.create({slideSize:{width:1280,height:720}});
  const tableOwners=[],chartOwners=[];
  for(const [i,d] of slides.entries()){
    const s=presentation.slides.add();s.background.fill=C.bg;
    if(d.kind==='refs'){
      heading(s,d);let y=164;
      const compact=d.refs.length>4;
      for(const r of d.refs){const url=which==='combined'||r.url.startsWith('https:')?r.url:'../'+r.url;y+=text(s,`[${r.n}] ${r.text}`,64,y,1152,21)+(compact?3:4);y+=text(s,url,64,y,1152,18,{color:C.teal,link:r.url.startsWith('https:')?r.url:undefined})+(compact?14:20);}
      if(y>662)throw new Error('Reference overflow');
    }else if(d.kind==='prompt'){
      heading(s,d);let font=22;
      while(font>18&&wrap(d.prompt,1152,font).split('\n').length*font*1.24>485)font--;
      const h=text(s,d.prompt,64,157,1152,font,{h:493});
      if(wrap(d.prompt,1152,font).split('\n').length*font*1.24>493)throw new Error(`Prompt overflow ${d.id}`);
    }else if(['cover','transfer','rotation','cudabrief','frameworkbrief','launch','features','speed','streamcore','gds','hpc','environment','governance','access','framework'].includes(d.kind))renderMain(s,d);
    else renderAppendix(s,d);
    if(d.kind!=='cover')sourceFooter(s,d.citations,i,slides.length);
    const sourceNotes=(d.citations??[]).map(n=>{const r=refs.find(x=>x.n===n);const url=which==='combined'||r.url.startsWith('https:')?r.url:'../'+r.url;return `[${n}] ${r.text}\n${url}`;}).join('\n\n');
    const deliveryNotes=d.seconds?`Target time: ${d.seconds} seconds.\n\n${d.narration}\n\n`:'';
    s.speakerNotes.text=deliveryNotes+sourceNotes;
    if(s.tables.items.length)tableOwners.push(i+1);
    if(s.charts.items.length)chartOwners.push(i+1);
  }
  const stem={combined:'Applied2_Combined_Presentation',t1:'Task1_GPU_Image_Rotation_Final',t2:'Task2_HPC_AI_Ethics'}[which];
  const candidate=path.join(buildDir,`${stem}.draft.pptx`);
  const final=path.join(outDir,`${stem}.${Date.now()}.pptx`);
  await (await PresentationFile.exportPptx(presentation)).save(candidate);
  const receipt=path.join(buildDir,`${stem}.${Date.now()}.validation.json`);
  await finalizePresentation({workspaceDir,candidatePath:candidate,finalPath:final,pythonExecutable:process.env.RUNTIME_PYTHON,integrityValidatorPath:path.join(SKILL_DIR,'container_tools/inspect_presentation_package_integrity.py'),layoutValidatorPath:path.join(SKILL_DIR,'container_tools/inspect_presentation_layout_geometry.py'),layoutArgs:['--expected-slide-size-emu','12192000,6858000','--validate-heading-fit',...tableOwners.flatMap(n=>['--require-native-table-slide',String(n)])],requiredNativeTableOwnerSlides:tableOwners,requiredNativeChartOwnerSlides:chartOwners,materializeLiteralChartWorkbooks:true,fontPolicy:{basis:'design',families:['Arial','Consolas']},verifyArtifactToolImport:true,receiptPath:receipt});
  const dest=which==='combined'?path.join(taskDir,`${stem}.pptx`):path.join(taskDir,which==='t1'?'task1':'task2',`${stem}.pptx`);
  await fs.copyFile(final,dest);
  await fs.copyFile(receipt,path.join(buildDir,`${stem}.validation.latest.json`));
  if(which==='combined'){
    const previews=path.join(buildDir,'slides');await fs.mkdir(previews,{recursive:true});
    for(const [i,s]of presentation.slides.items.entries()){
      const blob=await s.export({format:'png',scale:1});await fs.writeFile(path.join(previews,`slide-${String(i+1).padStart(2,'0')}.png`),new Uint8Array(await blob.arrayBuffer()));
      const highres=await s.export({format:'png',scale:2});await fs.writeFile(path.join(previews,`slide-${String(i+1).padStart(2,'0')}.print.png`),new Uint8Array(await highres.arrayBuffer()));
      console.log(`Rendered ${i+1}/${slides.length}: ${dTitle(slides[i])}`);
    }
    await fs.writeFile(path.join(buildDir,'deck_manifest.json'),JSON.stringify({mainCount:selectedMain.length,slides:slides.map((d,i)=>({...d,number:i+1})),students,refs},null,2));
  }
  console.log(`FINAL ${dest} (${slides.length} slides)`);
}
function dTitle(d){return d.title.replaceAll('\n',' / ');}
for(const which of (process.argv.slice(2).length?process.argv.slice(2):['combined','t1','t2']))await buildDeck(which);
await fs.writeFile(path.join(buildDir,'authored_geometry.json'),JSON.stringify(boxes,null,2));
