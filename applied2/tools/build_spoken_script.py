"""Create the conversational script and a printable, slide-aligned copy."""
from pathlib import Path
import json
from html import escape

from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, PageBreak, KeepTogether

BASE = Path(__file__).resolve().parents[1]
DATA = json.loads((BASE/'tools/presentation_script.json').read_text(encoding='utf-8'))
MANIFEST = json.loads((BASE/'.build/deck_manifest.json').read_text(encoding='utf-8'))
SLIDES = DATA['slides']
assert [s['number'] for s in SLIDES] == list(range(1, MANIFEST['mainCount']+1))
assert [s['seconds'] for s in SLIDES] == [s['seconds'] for s in MANIFEST['slides'][:MANIFEST['mainCount']]]
WORDS = sum(len(s['speech'].split()) for s in SLIDES)
SECONDS = sum(s['seconds'] for s in SLIDES)


def clock(seconds):
    return f'{seconds//60}:{seconds%60:02d}'


intro = (f'{WORDS} spoken words. Target: {clock(SECONDS)}, with {420-SECONDS} seconds available before seven minutes. '
         'At an assumed 130 words per minute, speech takes about '
         f'{WORDS/130:.1f} minutes, leaving time for pauses and slide changes. This is a planning estimate, not a measured rehearsal.')
split = 'Suggested split: Savin presents slides 1–6; Shamle presents slides 7–11. Stop at the conclusion; the appendix is not part of the script.'
rows = ['# Applied 2: conversational presentation script', '', intro, '', split, '',
        'Italic cues are directions, not spoken lines. Use the wording naturally rather than reading the code or equations aloud.', '',
        '| Slide | Speaker | Target | Finish by |', '| --- | --- | --- | --- |']
elapsed = 0
for s in SLIDES:
    elapsed += s['seconds']
    s['finish'] = clock(elapsed)
    rows.append(f'| {s["number"]} | {s["speaker"]} | {s["seconds"]} s | {s["finish"]} |')
rows.extend(['', '## Student details', ''])
for student in MANIFEST['students']:
    rows.append(f'{student["name"]} ({student["id"]}) — {student["email"]}')
    rows.append('')
for s in SLIDES:
    rows.extend([f'## Slide {s["number"]}: {s["title"]}', '',
                 f'**{s["speaker"]} · {s["seconds"]} seconds · finish by {s["finish"]}**', '',
                 f'*[{s["cue"]}]*', '', s['speech'], ''])
(BASE/'Applied2_Presentation_Script.md').write_text('\n'.join(rows), encoding='utf-8')

for name, filename in [('Arial', 'arial.ttf'), ('Arial-Bold', 'arialbd.ttf'), ('Arial-Italic', 'ariali.ttf')]:
    pdfmetrics.registerFont(TTFont(name, str(Path('C:/Windows/Fonts')/filename)))
pdfmetrics.registerFontFamily('Arial', normal='Arial', bold='Arial-Bold', italic='Arial-Italic', boldItalic='Arial-Bold')
ink = colors.HexColor('#142D45')
teal = colors.HexColor('#007B83')
styles = {
    'title': ParagraphStyle('Title', fontName='Arial-Bold', fontSize=22, leading=28, textColor=ink, spaceAfter=12),
    'intro': ParagraphStyle('Intro', fontName='Arial', fontSize=10, leading=14, textColor=ink, spaceAfter=8),
    'student': ParagraphStyle('Student', fontName='Arial', fontSize=9, leading=12, textColor=ink, spaceAfter=3),
    'heading': ParagraphStyle('Heading', fontName='Arial-Bold', fontSize=13, leading=17, textColor=teal, spaceBefore=15, spaceAfter=4),
    'meta': ParagraphStyle('Meta', fontName='Arial', fontSize=9, leading=12, textColor=colors.HexColor('#516170'), spaceAfter=5),
    'cue': ParagraphStyle('Cue', fontName='Arial-Italic', fontSize=9.5, leading=13, textColor=colors.HexColor('#516170'), spaceAfter=7),
    'speech': ParagraphStyle('Speech', fontName='Arial', fontSize=12, leading=18, textColor=ink, spaceAfter=8),
}


def para(text, style):
    return Paragraph(escape(text), styles[style])


def footer(canvas, doc):
    canvas.setFont('Arial', 9)
    canvas.setFillColor(colors.HexColor('#516170'))
    canvas.drawString(48, A4[1]-28, 'FIT3143 • Applied 2 • Conversational speaking script')
    canvas.setStrokeColor(colors.HexColor('#CBD5DC'))
    canvas.line(48, 42, A4[0]-48, 42)
    canvas.drawString(48, 27, 'Slides 1–11 only • Suggested timing, not measured rehearsal')
    canvas.drawRightString(A4[0]-48, 27, str(doc.page))


story = [para(DATA['title'], 'title'), para(intro, 'intro'), para(split, 'intro')]
for student in MANIFEST['students']:
    story.append(para(f'{student["name"]} ({student["id"]}) — {student["email"]}', 'student'))
story.extend([Spacer(1, 7), para('Italic cues are directions; speak only the main paragraphs.', 'intro')])
for s in SLIDES:
    if s['number'] in [4, 7, 9]:
        story.append(PageBreak())
        section = 'Task 1: CUDA and GPUDirect Storage' if s['number']==4 else 'Task 2: responsible HPC scaling'
        story.append(para(section, 'title'))
    story.append(KeepTogether([
        para(f'Slide {s["number"]} — {s["title"]}', 'heading'),
        para(f'{s["speaker"]} • {s["seconds"]} seconds • Finish by {s["finish"]}', 'meta'),
        para(s['cue'], 'cue'),
        para(s['speech'], 'speech'),
    ]))
doc = SimpleDocTemplate(str(BASE/'Applied2_Presentation_Script.pdf'), pagesize=A4,
                        leftMargin=48, rightMargin=48, topMargin=50, bottomMargin=57,
                        title=DATA['title'], author='Savin Vindiv De Alwis; Shamle Thilaksiri',
                        subject='Conversational script for the eleven-slide Applied 2 presentation')
doc.build(story, onFirstPage=footer, onLaterPages=footer)
print(f'Created Markdown and PDF: {WORDS} spoken words, {clock(SECONDS)} timing target.')
