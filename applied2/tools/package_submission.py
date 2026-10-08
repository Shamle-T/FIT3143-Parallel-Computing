"""Gather the actual submission files and export the preserved AI prompt record."""
from pathlib import Path
from html import escape
import hashlib
import json
import shutil
import subprocess

from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer
from pypdf import PdfReader

BASE = Path(__file__).resolve().parents[1]
DEST = BASE/'submission'
DEST.mkdir(exist_ok=True)
FILES = [
    'Applied2_Combined_Presentation.pptx',
    'Applied2_Combined_Presentation.pdf',
    'task1/Task1_CUDA_Kernel_Examples.cu',
    'task1/Task1_CUDA_Performance_Examples.cu',
    'task2/Task2_Chart_Data_and_Sources.json',
]
for relative in FILES:
    source = BASE/relative
    target = DEST/source.name
    shutil.copy2(source, target)
    assert hashlib.sha256(source.read_bytes()).digest() == hashlib.sha256(target.read_bytes()).digest()

earlier = json.loads((BASE/'tools/ai_prompt_record.json').read_text(encoding='utf-8'))['prompts']
continuation = json.loads(subprocess.check_output([
    'C:/Users/savin/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/bin/node.exe',
    '--input-type=module', '-e',
    'const {continuationPrompts}=await import("./applied2/tools/deck_content.mjs");process.stdout.write(JSON.stringify(continuationPrompts));',
], cwd=BASE.parent).decode('utf-8'))
recent = json.loads((BASE/'tools/recent_prompt_record.json').read_text(encoding='utf-8'))['prompts']
prompts = earlier+continuation+recent
students = json.loads((BASE/'.build/deck_manifest.json').read_text(encoding='utf-8'))['students']

for name, filename in [('Arial','arial.ttf'),('Arial-Bold','arialbd.ttf')]:
    pdfmetrics.registerFont(TTFont(name, str(Path('C:/Windows/Fonts')/filename)))
pdfmetrics.registerFontFamily('Arial',normal='Arial',bold='Arial-Bold')
ink = colors.HexColor('#142D45')
title = ParagraphStyle('Title',fontName='Arial-Bold',fontSize=21,leading=27,textColor=ink,spaceAfter=12)
body = ParagraphStyle('Body',fontName='Arial',fontSize=10.5,leading=15,textColor=ink,spaceAfter=10,splitLongWords=True)
heading = ParagraphStyle('Heading',fontName='Arial-Bold',fontSize=12,leading=17,textColor=colors.HexColor('#007B83'),spaceBefore=13,spaceAfter=6,keepWithNext=True)
small = ParagraphStyle('Small',fontName='Arial',fontSize=9,leading=13,textColor=ink,spaceAfter=6)


def p(value, style=body):
    return Paragraph(escape(value).replace('\n','<br/>'),style)


def footer(canvas, doc):
    canvas.setFont('Arial',9)
    canvas.setFillColor(colors.HexColor('#516170'))
    canvas.drawString(48, A4[1]-28, 'FIT3143 • Applied 2 • AI declaration and prompt record')
    canvas.drawString(48, 27, 'Preserved project records and available preparation-session messages')
    canvas.drawRightString(A4[0]-48,27,str(doc.page))


story = [p('AI declaration and prompt record',title)]
for student in students:
    story.append(p(f'{student["name"]} ({student["id"]}) — {student["email"]}',small))
story.extend([
    Spacer(1,9),
    p('Model and use',heading),
    p('The current presentation documents OpenAI GPT-6, accessed through Codex. AI assisted research and source checks, drafting and revision of explanations and CUDA teaching examples, diagrams/charts, presentation formatting, the speaking script and submission packaging.'),
    p('Record scope',heading),
    p('This PDF reproduces the seven earlier prompt records saved in the project, seven continuation records, and seven later preparation-session messages. Original spelling is preserved. Earlier model versions were not recorded in the saved prompt files. The records include instructions subsequently superseded by later user requests; the final presentation reflects the later instructions.'),
    p('The presentation itself contains one brief AI-use slide. This separate prompt PDF accompanies the submission under assessment instruction 9.'),
])
for index, prompt in enumerate(prompts,1):
    story.append(p(f'Record {index} — {prompt["date"]} — {prompt["title"]}',heading))
    story.append(p(prompt['text']))
pdf_path = DEST/'AI_Declaration_and_Prompt_Record.pdf'
SimpleDocTemplate(str(pdf_path),pagesize=A4,leftMargin=48,rightMargin=48,topMargin=50,bottomMargin=57,
                  title='Applied 2: AI declaration and prompt record',
                  author='Savin Vindiv De Alwis; Shamle Thilaksiri').build(story,onFirstPage=footer,onLaterPages=footer)
pdf_text = '\n'.join(page.extract_text() for page in PdfReader(pdf_path).pages)
normalize = lambda value: ' '.join(value.split())
for index, prompt in enumerate(prompts,1):
    assert normalize(prompt['text']) in normalize(pdf_text), index

(DEST/'README.md').write_text('''# Applied 2 submission files

Savin Vindiv De Alwis (35221631) — sdea0018@student.monash.edu
Shamle Thilaksiri (35512075) — wthi0003@student.monash.edu

Upload the actual files from this folder to Moodle. Both students should submit the same set.

| File | Purpose |
| --- | --- |
| Applied2_Combined_Presentation.pptx | Editable presentation: complete Task 1/2 answers, diagrams, code, calculations, research, references and brief AI declaration. |
| Applied2_Combined_Presentation.pdf | Matching 46-page visual copy, including the full appendix. |
| Task1_CUDA_Kernel_Examples.cu | Associated rotation and brightness teaching kernels. |
| Task1_CUDA_Performance_Examples.cu | Associated launch-shape and asynchronous-stream teaching examples. |
| Task2_Chart_Data_and_Sources.json | Chart values and numbered website sources. |
| AI_Declaration_and_Prompt_Record.pdf | Separate preserved prompt records required by assessment instruction 9, plus a brief AI-use statement. |

Present slides 1–11 only, ending at the conclusion. The appendix starts at slide 12. The conversational script and timing guide are available in the parent applied2 folder for rehearsal.

The CUDA files are illustrative excerpts, not a standalone application; compilation, GPU execution and measured speedup remain unverified. No measured result is asserted. The presentation contains the full content evidence, so no separate solution report is required.
''',encoding='utf-8')
print(f'Packaged {len(FILES)+2} files in {DEST}; exported {len(prompts)} preserved prompt records.')
