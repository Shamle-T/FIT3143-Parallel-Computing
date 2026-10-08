from pathlib import Path
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import cm
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer

out = Path(__file__).resolve().parents[1] / "output" / "AI_Use_Declaration.pdf"
out.parent.mkdir(exist_ok=True)
styles = getSampleStyleSheet()
styles.add(ParagraphStyle(name="Body", parent=styles["BodyText"], fontSize=10.5,
                          leading=15, spaceAfter=10))
story = [
    Paragraph("FIT3143 Parallel Computing - Week 8 Lab 2", styles["Title"]),
    Paragraph("Generative AI Use Declaration", styles["Heading1"]),
    Paragraph("Team members: Savin Vindiv De Alwis (35221631, sdea0018@student.monash.edu); "
              "Willwara Arachchilage Shamle Imal Thilaksiri (35512075, "
              "wthi0003@student.monash.edu).", styles["Body"]),
    Paragraph("During the preparation period, generative AI assistance was used to help interpret "
              "the assessment specification and marking rubric; review and improve the Open MPI "
              "and hybrid OpenMP implementation; design correctness and performance experiments; "
              "produce chart and presentation drafts; and prepare this documentation.", styles["Body"]),
    Paragraph("The team reviewed the resulting materials against the supplied specification, "
              "compiled and executed the programs locally with Open MPI, independently validated "
              "the generated prime files, and is responsible for every submitted claim, source file, "
              "result, and explanation. The team will be prepared to explain the design, MPI "
              "collectives, OpenMP scheduling, timing methodology, and performance observations "
              "without external tools during the presentation and Q&amp;A.", styles["Body"]),
    Paragraph("Required accompanying evidence: export and submit the complete prompt/conversation "
              "record as separate PDF file(s), as required by the assessment specification. This "
              "declaration identifies the nature of AI assistance but does not replace those prompt "
              "records.", styles["Body"]),
]
SimpleDocTemplate(str(out), pagesize=A4, rightMargin=2*cm, leftMargin=2*cm,
                  topMargin=2*cm, bottomMargin=2*cm).build(story)
print(out)
