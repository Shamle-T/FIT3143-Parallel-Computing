import fs from "node:fs/promises";
import { FileBlob, PresentationFile } from "@oai/artifact-tool";

const sourcePath = process.argv[2];
const outDir = process.argv[3];
if (!sourcePath || !outDir) throw new Error("Usage: inspect.mjs <source.pptx> <outDir>");

await fs.mkdir(outDir, { recursive: true });
const presentation = await PresentationFile.importPptx(await FileBlob.load(sourcePath));
const snapshot = await presentation.inspect({
  kind: "deck,slide,textbox,shape,image,table,chart,notes,layout",
  include: "id,slide,name,title,text,textPreview,textChars,textLines,bbox,bboxUnit,rows,cols,preview,alt,prompt,placeholders",
  maxChars: 30000,
});
await fs.writeFile(`${outDir}/inspect.ndjson`, snapshot.ndjson, "utf8");
const montage = await presentation.export({ format: "webp", montage: true, scale: 1 });
await fs.writeFile(`${outDir}/source-montage.webp`, new Uint8Array(await montage.arrayBuffer()));
for (let i = 0; i < presentation.slides.items.length; i++) {
  const slide = presentation.slides.getItem(i);
  const png = await slide.export({ format: "png", scale: 1 });
  await fs.writeFile(`${outDir}/source-slide-${i + 1}.png`, new Uint8Array(await png.arrayBuffer()));
}
console.log(snapshot.ndjson);
