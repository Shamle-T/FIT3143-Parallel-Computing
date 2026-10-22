"""Check preserved user edits/evidence, current spoken notes and the PDF visual copy."""
from pathlib import Path
from zipfile import ZipFile
from lxml import etree as E
from pypdf import PdfReader
from PIL import Image
import json
import hashlib
import re

BASE = Path(__file__).resolve().parents[1]
PRIVATE = BASE/'.build/hd_review'
NS = {'a':'http://schemas.openxmlformats.org/drawingml/2006/main',
      'p':'http://schemas.openxmlformats.org/presentationml/2006/main'}
script = json.loads((BASE/'tools/presentation_script.json').read_text(encoding='utf-8'))['slides']
manifest = json.loads((BASE/'.build/deck_manifest.json').read_text(encoding='utf-8'))
preservation = json.loads((PRIVATE/'native-preservation.json').read_text())
final_path = BASE/'submission/Applied2_Combined_Presentation.pptx'
receipt = json.loads((PRIVATE/'validation.latest.json').read_text())
digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
assert digest(final_path) == digest(BASE/'Applied2_Combined_Presentation.pptx') == receipt['finalSha256']
assert receipt['packageIntegrity']['finding_count'] == receipt['presentationLayout']['finding_count'] == 0

def paragraphs(data):
    return [''.join(p.itertext()) for p in E.fromstring(data).findall('.//a:p', NS)]

def body(data):
    # Footer pagination and the task label may change in an appendix snapshot.
    result = []
    root = E.fromstring(data)
    for p in root.findall('.//a:p', NS):
        value = ''.join(p.xpath('.//a:t/text()', namespaces=NS))
        if re.fullmatch(r'\d+\s*/\s*\d+', value) or value.startswith(('Task 1(b)', 'Task 2(a)', 'Appendix •')):
            continue
        result.append(value)
    return result

with ZipFile(PRIVATE/'latest-source.pptx') as old, ZipFile(final_path) as final:
    for name in preservation['protectedParts']:
        assert old.read(name) == final.read(name), name
    changes = {4,5,7,8,9,10}
    for n in range(1,47):
        if n in changes:
            continue
        name = f'ppt/slides/slide{n}.xml'
        assert body(old.read(name)) == body(final.read(name)), name
    for original, snapshot in [(4,47),(7,48)]:
        assert body(old.read(f'ppt/slides/slide{original}.xml')) == body(final.read(f'ppt/slides/slide{snapshot}.xml')), original
    for s in script:
        note = '\n'.join(E.fromstring(final.read(f'ppt/notesSlides/notesSlide{s["number"]}.xml')).xpath('.//a:t/text()', namespaces=NS))
        assert s['speech'] in note, s['number']
        assert manifest['slides'][s['number']-1]['narration'] == s['speech']
    assert len(preservation['slideOrder']) == 48
    assert sum(s['seconds'] for s in script) == 400
    assert sum(len(s['speech'].split()) for s in script) == 793

pdf = PdfReader(BASE/'Applied2_Combined_Presentation.pdf')
assert len(pdf.pages) == 48
for n,page in enumerate(pdf.pages,1):
    images = list(page.images)
    assert len(images) == 1, n
    actual = images[0].image.convert('RGB')
    with Image.open(BASE/'.build/slides'/f'slide-{n:02d}.print.png') as expected:
        assert actual.size == expected.size and actual.tobytes() == expected.convert('RGB').tobytes(), n
script_pdf = PdfReader(BASE/'Applied2_Presentation_Script.pdf')
text = ' '.join(' '.join(p.extract_text().split()) for p in script_pdf.pages)
for s in script:
    assert ' '.join(s['speech'].split()) in text, s['number']
assert len(script_pdf.pages) == 4
print('PASS: latest submission/canonical hashes, 22 protected native parts, unchanged evidence and two snapshots, eleven spoken notes, 793 words/400 seconds, all 48 PDF image pixels and full four-page script text.')
