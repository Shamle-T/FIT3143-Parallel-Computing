"""Refresh derived companions from the finalized, self-contained slide deck."""
from pathlib import Path
import json
import re
from zipfile import ZipFile
from xml.etree import ElementTree as ET

from reportlab.pdfgen import canvas
from reportlab.lib.utils import ImageReader

BASE = Path(__file__).resolve().parents[1]
BUILD = BASE / '.build'
MANIFEST = json.loads((BUILD / 'deck_manifest.json').read_text(encoding='utf-8'))
NS = {'a': 'http://schemas.openxmlformats.org/drawingml/2006/main'}


def visible_text_by_slide():
    with ZipFile(BASE / 'Applied2_Combined_Presentation.pptx') as z:
        names = sorted((n for n in z.namelist() if re.fullmatch(r'ppt/slides/slide\d+\.xml', n)),
                       key=lambda n: int(re.search(r'slide(\d+)', n)[1]))
        return ['\n'.join(''.join(t.text or '' for t in p.findall('.//a:t', NS))
                          for p in ET.fromstring(z.read(n)).findall('.//a:p', NS))
                for n in names]


def slide_pdf(destination, numbers, title):
    pdf = canvas.Canvas(str(destination), pagesize=(960, 540), pageCompression=1)
    pdf.setTitle(title)
    pdf.setAuthor('Savin Vindiv De Alwis; Shamle Thilaksiri')
    pdf.setSubject('FIT3143 Applied 2: presentation, references and AI use declaration')
    for number in numbers:
        image = BUILD / 'slides' / f'slide-{number:02d}.print.png'
        if not image.is_file():
            raise FileNotFoundError(image)
        pdf.drawImage(ImageReader(str(image)), 0, 0, width=960, height=540)
        pdf.showPage()
    pdf.save()


def main():
    slides = MANIFEST['slides']
    texts = visible_text_by_slide()
    assert len(slides) == len(texts)
    slide_pdf(BASE / 'Applied2_Combined_Presentation.pdf', range(1, len(slides)+1),
              'Applied 2: GPU rotation and responsible HPC for AI')
    for task, label in [('t1', 'Task1'), ('t2', 'Task2')]:
        selected = [i+1 for i, d in enumerate(slides) if d['task'] in [task, 'both']]
        folder = BASE / ('task1' if task == 't1' else 'task2')
        pdf_name = folder / f'{label}_Study_Guide_and_AI_Record.pdf'
        slide_pdf(pdf_name, selected, f'{label}: slide companion and AI record')
        rows = [f'# {label}: current presentation companion', '',
                'Generated from the current combined presentation on 8 October 2026.',
                'All assessment explanations, code excerpts, research comparisons, references and a brief AI declaration are visible in the presentation; this companion is optional.', '',
                'The current timed presentation path is in ../Presentation_Timing_and_Notes.md and in the PowerPoint speaker notes. Q&A remains outside this revision.', '']
        for i in selected:
            d = slides[i-1]
            rows.extend([f'## Combined slide {i}: {d["title"].replace(chr(10), " ")}', '', texts[i-1], ''])
            if d['id'] == 'environment':
                rows.extend(['Chart values: 2025 estimate 485 TWh/year; 2030 projection 950 TWh/year. 2025 growth: all data centres 17%; AI-focused subset 50%. Source [6].', ''])
        markdown = '\n'.join(rows)
        # Trim display-only Markdown whitespace; canonical prompt records stay intact.
        markdown = '\n'.join(line.rstrip() for line in markdown.split('\n'))
        (folder / f'{label}_Speaker_Notes_and_Study_Guide.md').write_text(markdown, encoding='utf-8')

    chart_path = BASE / 'task2/Task2_Chart_Data_and_Sources.json'
    chart = json.loads(chart_path.read_text(encoding='utf-8'))
    assert chart['chartData']['electricity']['values'] == [485, 950]
    assert chart['chartData']['growth']['values'] == [0.17, 0.5]
    for data in chart['chartData'].values():
        data['source'] = '6'
    chart['references'] = [{'id': str(r['n']), 'citation': r['text'], 'url': r['url']}
                           for r in MANIFEST['refs'] if 5 <= r['n'] <= 16]
    chart['citationScheme'] = 'Numbering matches the current combined presentation.'
    chart_path.write_text(json.dumps(chart, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    readme = '''# Applied 2: GPU rotation and responsible HPC for AI

All files to upload are collected in [submission/](submission/README.md): the combined PowerPoint/PDF, associated CUDA sources, chart data and the required AI prompt-record PDF. The speaking script is rehearsal material in this parent folder. Run tools/package_submission.py after changing the canonical files to refresh the upload copies.

Use [the combined PowerPoint](Applied2_Combined_Presentation.pptx) and [its PDF copy](Applied2_Combined_Presentation.pdf) as the current presentation package. Both contain the assessment explanations, code excerpts, research comparisons, citations, references and a brief AI declaration. No separate report is needed to understand Tasks 1 and 2.

## Presentation structure

| Slides | Content |
| --- | --- |
| 1 | Cover with both students' verified details |
| 2–7 | Task 1: transfers, CUDA rotation/model, speedup and GPUDirect Storage |
| 8–12 | Task 2: parallel HPC training and all four ethics topics |
| 13–34 | Clearly labeled supporting detail, code, comparisons and limitations |
| 35–39 | Numbered website references |
| 40 | Brief AI model/tool and use declaration |

The main presentation contains 12 slides including the cover. The appendix supports assessment of detailed content and is included in both formats. Timing targets support the seven-minute limit; actual delivery duration needs rehearsal. No Q&A slide was added.

## Design and evidence

Task 1 uses inverse gathering, contiguous 8-bit RGB, nearest-neighbour sampling, a fixed canvas and black borders for out-of-bounds rounded source indices. It states serial and multithreaded CPU comparison options, theoretical speedup limits and conditional GDS benefits. The CUDA teaching examples are uncompiled and unbenchmarked; no measured GPU performance is claimed.

Task 2 distinguishes current sector estimates/projections, original research and proposed policy. The chart values remain 485/950 TWh and 17%/50%, with source scope and forecast caveats. Tables, diagrams and charts in the PowerPoint remain native/editable; chart values have embedded workbooks. The PDF is a high-resolution visual copy.

The content follows [teaching-team guidance](TEACHING_TEAM_GUIDANCE.md), the [specification](<assessment/Applied #2 - Assessment Specification.pdf>) and [rubric](<assessment/Applied #2 Rubric.pdf>). The presentation references only public websites; local course-file references and citations have been removed at the student's request.

## Supporting files

The current [conversational presentation script](Applied2_Presentation_Script.md) and [printable PDF](Applied2_Presentation_Script.pdf) follow slides 1–11. They include a suggested Savin/Shamle split, slide timings and brief diagram cues; the 755 spoken words fit the 6:20 planning target at the stated pace.

The task folders contain synchronized standalone decks, optional slide companions and source examples/chart data. The presentation contains one brief AI-use slide, as requested. Historical prompt sources remain in tools/ai_prompt_record.json and tools/deck_content.mjs; prompt-by-prompt slides are excluded from the presentation and its companions.

The earlier six-minute spoken script belongs to the previous deck and is now under archive/. Its slide cues are obsolete. It is not part of the current submission package.

The original review/action plan is retained as a historical audit with a revision-status section. Build sources and content checks are under tools/; private renders and validation receipts are ignored by Git. Moodle requires actual files rather than repository links.
'''
    def span(numbers):
        return str(numbers[0]) if numbers[0] == numbers[-1] else f'{numbers[0]}–{numbers[-1]}'
    main_count = MANIFEST['mainCount']
    main_slides = slides[:main_count]
    total_seconds = sum(d['seconds'] for d in main_slides)
    total_words = sum(len(d['narration'].split()) for d in main_slides)
    timing = ['# Presentation timing and notes', '',
              f'Present slides 1–{main_count}, ending at the conclusion. The appendix is for assessment/supporting detail and is not included in the spoken path.', '',
              f'Target: {total_seconds//60}:{total_seconds%60:02d}, leaving {420-total_seconds} seconds within the seven-minute limit. These are planning targets, not a measured rehearsal.', '',
              f'The optional narration contains {total_words} words. At an assumed 130 words/minute, speech alone takes approximately {total_words/130:.1f} minutes. Allow the remaining time for pointing at diagrams, slide changes and pauses.', '',
              '| Slide | Topic | Target | Finish by |', '| --- | --- | --- | --- |']
    elapsed = 0
    for d in main_slides:
        elapsed += d['seconds']
        timing.append(f'| {d["number"]} | {d["title"].replace(chr(10), " ")} | {d["seconds"]} s | {elapsed//60}:{elapsed%60:02d} |')
    timing.extend(['', 'Follow the concepts and diagrams; do not read every equation, code line or citation aloud. Full code, calculations and source comparisons remain in the appendix.', ''])
    for d in main_slides:
        timing.extend([f'## Slide {d["number"]}: {d["title"].replace(chr(10), " ")}', '', d['narration'], ''])
    (BASE/'Presentation_Timing_and_Notes.md').write_text('\n'.join(timing), encoding='utf-8')
    sections = [
        ([1], "Cover with both students' verified details"),
        ([d['number'] for d in slides[:main_count] if d['task'] == 't1'], 'Task 1: transfer/rotation diagrams, CUDA launch and features, expected speedup and GDS'),
        ([d['number'] for d in slides[:main_count] if d['task'] == 't2'], 'Task 2: environment, governance/fairness, access and responsible-use framework'),
        ([d['number'] for d in slides if d['id'] == 'conclusions'], 'Conclusions, limitations and next measurements'),
        ([d['number'] for d in slides[main_count:] if not d['id'].startswith('references-') and d['id'] != 'ai-declaration'], 'Full detailed answers, implementations, calculations and research comparisons; not presented'),
        ([d['number'] for d in slides if d['id'].startswith('references-')], 'Numbered website references'),
        ([d['number'] for d in slides if d['id'] == 'ai-declaration'], 'One brief AI model/tool and use declaration'),
    ]
    table = '| Slides | Content |\n| --- | --- |\n' + '\n'.join(f'| {span(nums)} | {label} |' for nums, label in sections)
    readme = re.sub(r'\| Slides \| Content \|.*?(?=\n\nThe main presentation)', table, readme, flags=re.S)
    readme = readme.replace('The main presentation contains 12 slides including the cover. The appendix supports assessment of detailed content and is included in both formats.',
                            f'Present slides 1–{main_count}, including the cover and conclusion. The target is {total_seconds//60}:{total_seconds%60:02d}, with a {420-total_seconds}-second buffer within seven minutes. [Timing and optional narration](Presentation_Timing_and_Notes.md) are also embedded in the PowerPoint speaker notes. Detailed content has been retained in the appendix, followed by website references and one AI-use declaration.')
    (BASE/'README.md').write_text(readme, encoding='utf-8')
    print(f'Exported the {len(slides)}-page presentation PDF and both synchronized slide companions.')


if __name__ == '__main__':
    main()
