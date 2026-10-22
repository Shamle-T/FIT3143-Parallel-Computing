"""Refresh the current combined deck companions without restoring deleted submission files."""
from pathlib import Path
import json
import re
from finalize_documents import slide_pdf

BASE = Path(__file__).resolve().parents[1]
manifest = json.loads((BASE/'.build/deck_manifest.json').read_text(encoding='utf-8'))
script = json.loads((BASE/'tools/presentation_script.json').read_text(encoding='utf-8'))['slides']
slides = manifest['slides']
main = slides[:manifest['mainCount']]
assert len(main) == len(script) == 11
assert all(a['narration'] == b['speech'] and a['seconds'] == b['seconds'] for a, b in zip(main, script))
seconds = sum(s['seconds'] for s in script)
words = sum(len(s['speech'].split()) for s in script)
slide_pdf(BASE/'Applied2_Combined_Presentation.pdf', range(1, len(slides)+1),
          'Applied 2: GPU rotation and responsible HPC for AI')

def clock(value):
    return f'{value//60}:{value%60:02d}'

rows = ['# Presentation timing and notes', '',
        'Revised 9 October 2026. Present slides 1–11 and stop at the conclusion. Supporting code, calculations and full research comparisons are in the appendix starting at slide 12.', '',
        f'Target: {clock(seconds)}, with {420-seconds} seconds available before seven minutes. The script contains {words} spoken words. At an assumed 130 words/minute, speech alone takes approximately {words/130:.1f} minutes. These are planning estimates, not measured rehearsal results.', '',
        'Savin: slides 1–6 (3:00). Shamle: slides 7–11 (3:40). Speak the core explanation and the brief appendix pointers. Merely displaying an explanation does not establish spoken coverage.', '',
        '| Slide | Topic | Target | Finish by |', '| --- | --- | --- | --- |']
elapsed = 0
for s in script:
    elapsed += s['seconds']
    rows.append(f'| {s["number"]} | {s["title"]} | {s["seconds"]} s | {clock(elapsed)} |')
rows += ['', 'See [spoken coverage audit](HD_SPOKEN_COVERAGE.md) for the rubric-to-slide-and-script mapping.', '']
for s in script:
    rows += [f'## Slide {s["number"]}: {s["title"]}', '', s['speech'], '']
(BASE/'Presentation_Timing_and_Notes.md').write_text('\n'.join(rows), encoding='utf-8')

readme = (BASE/'README.md').read_text(encoding='utf-8')
start = 'All files to upload'
first = readme.index(start)
end = readme.index('\n\n', first)
readme = readme[:first] + ('The current upload materials are in [submission/](submission/): the latest combined PowerPoint, associated CUDA sources and chart data. The combined PDF and speaking script in this parent folder are optional viewing/rehearsal copies. This review preserves the files previously removed from submission/.') + readme[end:]
readme = re.sub(r'\| Slides \| Content \|.*?(?=\n\nPresent slides)',
    '| Slides | Content |\n| --- | --- |\n| 1 | Cover |\n| 2–6 | Task 1: transfers, rotation, CUDA execution/performance and GDS |\n| 7–10 | Task 2: environment, governance/fairness, access and responsible framework |\n| 11 | Conclusion and next measurements |\n| 12–43 | Supporting examples, calculations and research comparisons |\n| 44–47 | Numbered website references |\n| 48 | One brief AI-use declaration |', readme, flags=re.S)
readme = re.sub(r'The target is \d+:\d+, with a \d+-second buffer', f'The target is {clock(seconds)}, with a {420-seconds}-second buffer', readme)
readme = re.sub(r'the \d+ spoken words fit the \d+:\d+ planning target at the stated pace\.',
               f'the {words} spoken words support the {clock(seconds)} planning target at the stated pace. Actual delivery needs rehearsal.', readme)
readme = readme.replace('The task folders contain synchronized standalone decks, optional slide companions and source examples/chart data.',
                       'The task folders contain earlier standalone decks/companions and source examples/chart data. The revised combined presentation in submission/ is the current assessment deck.')
readme += '\n\nThe 9 October spoken-coverage review places core explanations on main slides and explicitly mentions each in the script. See [coverage mapping](HD_SPOKEN_COVERAGE.md). Original detailed code, calculations, charts and research comparisons are retained in the appendix.\n'
(BASE/'README.md').write_text(readme, encoding='utf-8')
print(f'Refreshed {len(slides)}-page combined PDF and timing notes: {words} words, {clock(seconds)} target.')
