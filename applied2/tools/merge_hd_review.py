"""Apply Artifact Tool-authored main content while preserving the latest native package."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from copy import deepcopy
import json
import posixpath
from repair_powerpoint_package import repair
from lxml import etree as ET

BASE=Path(__file__).resolve().parents[1]
DIR=BASE/'.build/hd_review'
NS={'p':'http://schemas.openxmlformats.org/presentationml/2006/main',
    'a':'http://schemas.openxmlformats.org/drawingml/2006/main',
    'r':'http://schemas.openxmlformats.org/officeDocument/2006/relationships',
    'c':'http://schemas.openxmlformats.org/drawingml/2006/chart',
    'pr':'http://schemas.openxmlformats.org/package/2006/relationships',
    'ct':'http://schemas.openxmlformats.org/package/2006/content-types'}


def parse(data): return ET.fromstring(data)
def xml(root): return ET.tostring(root,encoding='UTF-8',xml_declaration=True,standalone=True)
def resolve(part,target):
    return target.lstrip('/') if target.startswith('/') else posixpath.normpath(posixpath.join(posixpath.dirname(part),target))
def relpart(part):
    return posixpath.join(posixpath.dirname(part),'_rels',posixpath.basename(part)+'.rels')


with ZipFile(DIR/'latest-source.pptx') as z: original={n:z.read(n) for n in z.namelist()}
with ZipFile(DIR/'authored-main.pptx') as z: authored={n:z.read(n) for n in z.namelist()}
out=dict(original)
presentation=parse(out['ppt/presentation.xml'])
prels=parse(out['ppt/_rels/presentation.xml.rels'])
rels={r.get('Id'):r for r in prels}
order=presentation.find('p:sldIdLst',NS)
source_order=[resolve('ppt/presentation.xml',rels[s.get('{'+NS['r']+'}id')].get('Target')) for s in order]
assert source_order==[f'ppt/slides/slide{i}.xml' for i in range(1,47)]

for n in [4,5,7,8,9,10]:
    name=f'ppt/slides/slide{n}.xml'
    old=parse(original[name]); new=parse(authored[name])
    tree=deepcopy(new.find('p:cSld/p:spTree',NS))
    if n==7:
        # Preserve the original chart definition, source workbook and relationship.
        oldframe=deepcopy(old.findall('p:cSld/p:spTree/p:graphicFrame',NS)[0])
        generated=tree.findall('p:graphicFrame',NS)[0]
        oldframe.find('p:nvGraphicFramePr/p:cNvPr',NS).set('id',generated.find('p:nvGraphicFramePr/p:cNvPr',NS).get('id'))
        oldframe.find('p:xfrm',NS).getparent().replace(oldframe.find('p:xfrm',NS),deepcopy(generated.find('p:xfrm',NS)))
        tree.replace(generated,oldframe)
    old.find('p:cSld',NS).replace(old.find('p:cSld/p:spTree',NS),tree)
    out[name]=xml(old)

# Use the conversational narration as the single delivery source in all main notes.
for n in range(1,12):
    name=f'ppt/notesSlides/notesSlide{n}.xml'
    old=parse(original[name]); new=parse(authored[name])
    def body(root):
        return next(s for s in root.findall('.//p:sp',NS)
                    if s.find('p:nvSpPr/p:nvPr/p:ph',NS) is not None and s.find('p:nvSpPr/p:nvPr/p:ph',NS).get('type')=='body')
    target=body(old)
    target.replace(target.find('p:txBody',NS),deepcopy(body(new).find('p:txBody',NS)))
    out[name]=xml(old)

types=parse(out['[Content_Types].xml'])
rid_counter=max(int(r.get('Id')[3:]) for r in prels if r.get('Id').startswith('rId'))
slide_counter=max(int(s.get('id')) for s in order)
for offset,old_num in enumerate([4,7]):
    new_num=47+offset
    old_slide=f'ppt/slides/slide{old_num}.xml'; new_slide=f'ppt/slides/slide{new_num}.xml'
    out[new_slide]=original[old_slide]
    sr=parse(original[relpart(old_slide)])
    for r in sr:
        if r.get('Type').endswith('/notesSlide'): r.set('Target',f'../notesSlides/notesSlide{new_num}.xml')
    out[relpart(new_slide)]=xml(sr)
    old_note=f'ppt/notesSlides/notesSlide{old_num}.xml'; new_note=f'ppt/notesSlides/notesSlide{new_num}.xml'
    out[new_note]=original[old_note]
    nr=parse(original[relpart(old_note)])
    for r in nr:
        if r.get('Type').endswith('/slide'): r.set('Target',f'../slides/slide{new_num}.xml')
    out[relpart(new_note)]=xml(nr)
    rid_counter+=1;slide_counter+=1
    rel=ET.SubElement(prels,'{'+NS['pr']+'}Relationship',Id=f'rId{rid_counter}',Type=NS['r']+'/slide',Target=f'slides/slide{new_num}.xml')
    entry=ET.Element('{'+NS['p']+'}sldId',id=str(slide_counter))
    entry.set('{'+NS['r']+'}id',rel.get('Id'))
    order.insert(12+offset,entry)
    for part,kind in [(new_slide,'slide'),(new_note,'notesSlide')]:
        ET.SubElement(types,'{'+NS['ct']+'}Override',PartName='/'+part,ContentType=f'application/vnd.openxmlformats-officedocument.presentationml.{kind}+xml')

out['ppt/presentation.xml']=xml(presentation)
out['ppt/_rels/presentation.xml.rels']=xml(prels)
out['[Content_Types].xml']=xml(types)
new_order=source_order[:12]+['ppt/slides/slide47.xml','ppt/slides/slide48.xml']+source_order[12:]
manifest=json.loads((DIR/'updated-manifest.json').read_text(encoding='utf-8'))
for index,name in enumerate(new_order,1):
    root=parse(out[name])
    for t in root.findall('.//a:t',NS):
        if t.text and t.text.strip()==f'{(int(name.split("slide")[-1].split(".")[0]))} / 46': t.text=f'{index} / 48'
    if index in [13,14]:
        texts=root.findall('.//a:t',NS)
        texts[0].text=manifest['slides'][index-1]['label']
        # Snapshot source footer carried its former main position.
        for t in texts:
            if t.text and t.text.strip()==f'{4 if index==13 else 7} / 46':t.text=f'{index} / 48'
    out[name]=xml(root)

# Unedited native evidence, images, masters, chart definitions and workbook bytes stay intact.
protected=[n for n in original if n.startswith(('ppt/charts/','ppt/embeddings/','ppt/media/','ppt/slideMasters/','ppt/slideLayouts/','ppt/theme/'))]
assert all(out[n]==original[n] for n in protected)
with ZipFile(DIR/'merged-candidate.pptx','w',ZIP_DEFLATED) as z:
    for name,data in out.items():z.writestr(name,data)
# PowerPoint requires a separate native chart part for each owner slide.
# Snapshot 14 shares the main electricity chart unless it is explicitly cloned.
repair(DIR/'merged-candidate.pptx',DIR/'ownership-candidate.pptx')
(DIR/'merged-candidate.pptx').write_bytes((DIR/'ownership-candidate.pptx').read_bytes())
(DIR/'native-preservation.json').write_text(json.dumps({'protectedParts':protected,'slideOrder':new_order},indent=2),encoding='utf-8')
print(f'Merged focused edits; preserved {len(protected)} native evidence/design parts byte-for-byte.')
