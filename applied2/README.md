# Applied 2: GPU rotation and responsible HPC for AI

All files to upload are collected in [submission/](submission/README.md): the combined PowerPoint/PDF, associated CUDA sources, chart data and the required AI prompt-record PDF. The speaking script is rehearsal material in this parent folder. Run tools/package_submission.py after changing the canonical files to refresh the upload copies.

Use [the combined PowerPoint](Applied2_Combined_Presentation.pptx) and [its PDF copy](Applied2_Combined_Presentation.pdf) as the current presentation package. Both contain the assessment explanations, code excerpts, research comparisons, citations, references and a brief AI declaration. No separate report is needed to understand Tasks 1 and 2.

## Presentation structure

| Slides | Content |
| --- | --- |
| 1 | Cover with both students' verified details |
| 2–6 | Task 1: transfer/rotation diagrams, CUDA launch and features, expected speedup and GDS |
| 7–10 | Task 2: environment, governance/fairness, access and responsible-use framework |
| 11 | Conclusions, limitations and next measurements |
| 12–41 | Full detailed answers, implementations, calculations and research comparisons; not presented |
| 42–45 | Numbered website references |
| 46 | One brief AI model/tool and use declaration |

Present slides 1–11, including the cover and conclusion. The target is 6:20, with a 40-second buffer within seven minutes. [Timing and optional narration](Presentation_Timing_and_Notes.md) are also embedded in the PowerPoint speaker notes. Detailed content has been retained in the appendix, followed by website references and one AI-use declaration. Timing targets support the seven-minute limit; actual delivery duration needs rehearsal. No Q&A slide was added.

## Design and evidence

Task 1 uses inverse gathering, contiguous 8-bit RGB, nearest-neighbour sampling, a fixed canvas and black borders for out-of-bounds rounded source indices. It states serial and multithreaded CPU comparison options, theoretical speedup limits and conditional GDS benefits. The CUDA teaching examples are uncompiled and unbenchmarked; no measured GPU performance is claimed.

Task 2 distinguishes current sector estimates/projections, original research and proposed policy. The chart values remain 485/950 TWh and 17%/50%, with source scope and forecast caveats. Tables, diagrams and charts in the PowerPoint remain native/editable; chart values have embedded workbooks. The PDF is a high-resolution visual copy.

The content follows [teaching-team guidance](TEACHING_TEAM_GUIDANCE.md), the [specification](<assessment/Applied #2 - Assessment Specification.pdf>) and [rubric](<assessment/Applied #2 Rubric.pdf>). The presentation references only public websites; local course-file references and citations have been removed at the student's request.

## Supporting files

The current [conversational presentation script](Applied2_Presentation_Script.md) and [printable PDF](Applied2_Presentation_Script.pdf) follow slides 1–11. They include a suggested Savin/Shamle split, slide timings and brief diagram cues; the 755 spoken words fit the 6:20 planning target at the stated pace.

The task folders contain synchronized standalone decks, optional slide companions and source examples/chart data. The presentation contains one brief AI-use slide, as requested. Historical prompt sources remain in tools/ai_prompt_record.json and tools/deck_content.mjs; prompt-by-prompt slides are excluded from the presentation and its companions.

The earlier six-minute spoken script belongs to the previous deck and is now under archive/. Its slide cues are obsolete. It is not part of the current submission package.

The original review/action plan is retained as a historical audit with a revision-status section. Build sources and content checks are under tools/; private renders and validation receipts are ignored by Git. Moodle requires actual files rather than repository links.
