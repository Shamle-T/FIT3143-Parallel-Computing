import { FileBlob, PresentationFile } from "@oai/artifact-tool";

const presentation = await PresentationFile.importPptx(await FileBlob.load(process.argv[2]));
for (const id of [
  "sh/0f2lgnmp", "sh/ra943il8", "sh/q903ad4n", "sh/b6d4f2lc", "sh/a543mx4r", "sh/xw3q94ne",
  "sh/bq9orito", "sh/o3i5w3ad", "sh/p4r6p8by", "sh/gzy5s3a1",
  "sh/p0ji9kf2", "sh/72t03qp0"
]) {
  const item = presentation.resolve(id);
  console.log(id, Object.keys(item.text), item.text.toString(), item.text.plainText, item.text.getText?.());
}
for (const slideIndex of [11, 12]) {
  const slide = presentation.slides.getItem(slideIndex);
  console.log("slide", slideIndex + 1, "images", slide.images.items.length, slide.images.items.map(i => ({ frame: i.frame, alt: i.alt, fit: i.fit, keys: Object.keys(i) })));
}
const table = presentation.resolve("tb/rexkf2d4");
console.log("table", Object.keys(table), table.rows.length, table.cols?.length, table.columns?.length);
for (let r = 0; r < table.rows.length; r++) {
  const values = [];
  for (let c = 0; c < (table.cols?.length ?? table.columns?.length ?? 0); c++) values.push(table.getCell(r, c).text.toString());
  console.log(r, values);
}
