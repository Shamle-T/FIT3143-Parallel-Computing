"""Give the duplicated evidence slide independent native chart ownership."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from lxml import etree as E
import posixpath
import sys

def repair(source, destination):
    with ZipFile(source) as z:
        parts = {n:z.read(n) for n in z.namelist()}
    ns = {'r':'http://schemas.openxmlformats.org/package/2006/relationships',
          'ct':'http://schemas.openxmlformats.org/package/2006/content-types'}
    relname = 'ppt/slides/_rels/slide48.xml.rels'
    relationships = E.fromstring(parts[relname])
    shared = next(r for r in relationships if r.get('Target') == '../charts/chart1.xml')
    shared.set('Target','../charts/chart3.xml')
    serialize = lambda root:E.tostring(root,encoding='UTF-8',xml_declaration=True,standalone=True)
    parts[relname] = serialize(relationships)
    parts['ppt/charts/chart3.xml'] = parts['ppt/charts/chart1.xml']
    chartrels = E.fromstring(parts['ppt/charts/_rels/chart1.xml.rels'])
    for r in chartrels:
        if r.get('Type').endswith('/package'):
            original = posixpath.normpath(posixpath.join('ppt/charts',r.get('Target')))
            copied = 'ppt/embeddings/Appendix_Electricity_Chart.xlsx'
            parts[copied] = parts[original]
            r.set('Target','../embeddings/Appendix_Electricity_Chart.xlsx')
    parts['ppt/charts/_rels/chart3.xml.rels'] = serialize(chartrels)
    content_types = E.fromstring(parts['[Content_Types].xml'])
    E.SubElement(content_types,'{'+ns['ct']+'}Override',
        PartName='/ppt/charts/chart3.xml',
        ContentType='application/vnd.openxmlformats-officedocument.drawingml.chart+xml')
    parts['[Content_Types].xml'] = serialize(content_types)
    with ZipFile(destination,'w',ZIP_DEFLATED) as z:
        for n,data in parts.items():
            z.writestr(n,data)
    print('Copied the shared electricity chart/workbook for independent appendix ownership; retained every slide and original part.')

if __name__ == '__main__':
    repair(Path(sys.argv[1]),Path(sys.argv[2]))
