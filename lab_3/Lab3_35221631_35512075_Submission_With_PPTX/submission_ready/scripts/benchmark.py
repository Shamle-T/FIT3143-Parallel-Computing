"""Run inside cluster coordinator; retains all per-rank logs and placement."""
import argparse
import csv
import json
import os
import pathlib
import platform
import subprocess
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]


def launch(cluster):
    args = ['mpirun', '--allow-run-as-root', '--oversubscribe', '--bind-to', 'none']
    if cluster:
        args += ['--mca', 'btl', 'self,tcp', '--host',
                 'fit3143-lab3-a:64,fit3143-lab3-b:64', '--map-by', 'node']
    return args


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--cluster', action='store_true')
    parser.add_argument('--repeats', type=int, default=3)
    options = parser.parse_args()
    os.chdir(ROOT)
    out = ROOT/'results'/'cluster'
    out.mkdir(exist_ok=True)
    grids = {}
    for r in range(2, 12):
        for c in range(r, 18):
            grids.setdefault(r*c, (r, c))
    sizes = [('grid', *grids[n]) for n in sorted(grids)[:30]]
    sizes += [('line', 1, n) for n in range(2, 32)]
    environment = {'platform': platform.platform(), 'cpu_count': os.cpu_count(),
                   'cluster': options.cluster, 'transport': 'TCP between containers; shared host',
                   'oversubscribed': True, 'repeats': options.repeats,
                   'command': launch(options.cluster), 'grids': sizes,
                   'mpi': subprocess.check_output(['mpirun', '--version'], text=True)}
    (out/'environment.json').write_text(json.dumps(environment, indent=2))
    with (out/'manifest.csv').open('w', newline='') as handle:
        writer = csv.DictWriter(handle, fieldnames=['topology','rows','cols','nodes','variant','repeat',
                               'prefix','wall_seconds','rounds','warmup','expiry','scenario'])
        writer.writeheader()
        jobs = [(t,r,c,v,rep) for t,r,c in sizes for v in range(3) for rep in range(options.repeats)]
        # Deterministically rotate version order to reduce order/thermal bias.
        jobs.sort(key=lambda j:(j[0],j[1]*j[2],j[4],(j[3]-j[4])%3))
        for index,(topology,r,c,v,rep) in enumerate(jobs,1):
            prefix = f'results/cluster/{topology}_{r}x{c}_v{v}_r{rep}'
            cmd = launch(options.cluster)+['-np',str(r*c+1),'./wsn_cluster',str(r),str(c),'2',
                  '--rounds','6','--warmup','2','--tick','0','--scenario','mixed',
                  '--variant',str(v),'--expiry','30','--output',prefix]
            before=time.monotonic()
            subprocess.run(cmd, check=True, timeout=180, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            wall=time.monotonic()-before
            writer.writerow(dict(topology=topology,rows=r,cols=c,nodes=r*c,variant=v,repeat=rep,
                                 prefix=prefix,wall_seconds=wall,rounds=6,warmup=2,expiry=30,scenario='mixed'))
            handle.flush()
            print(f'{index}/{len(jobs)} {topology} {r}x{c} version={v} repeat={rep} wall={wall:.2f}s', flush=True)
    # Real-time sensitivity runs, separate from accelerated scaling measurements.
    for expiry in (0, 0.1, 1, 30):
        prefix=f'results/cluster/expiry_{expiry}'
        subprocess.run(launch(options.cluster)+['-np','10','./wsn_cluster','3','3','2','--rounds','6',
                       '--tick','0.05','--scenario','mixed','--variant','2','--expiry',str(expiry),
                       '--output',prefix],check=True,timeout=90)
    print('COMPLETE scaling and expiry sensitivity',flush=True)
