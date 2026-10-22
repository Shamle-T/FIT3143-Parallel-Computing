"""Run inside the supplied two-node SLURM allocation; retain every output.

Savin Vindiv De Alwis 35221631 sdea0018@student.monash.edu
Shamle Thilaksiri 35512075 wthi0003@student.monash.edu
"""
import csv
import json
import os
from pathlib import Path
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'results/caas'


def launch(ranks, executable, arguments=(), nodes=2, timeout=75):
    # Verified on CAAS; explicit step resources avoid inherited task-layout
    # conflicts. Each rank receives its two requested OpenMP port cores.
    command = ['srun', '--mpi=none', '--exact', '--nodes='+str(nodes),
               '--ntasks='+str(ranks), '--ntasks-per-node='+str((ranks+nodes-1)//nodes),
               '--cpus-per-task=2', '--distribution=cyclic', '--cpu-bind=cores',
               executable] + list(arguments)
    start = time.monotonic()
    try:
        result = subprocess.run(command, cwd=str(ROOT), stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, universal_newlines=True, timeout=timeout)
    except subprocess.TimeoutExpired as exc:
        entry = {'command':command, 'timeout_seconds':timeout,
                 'stdout':str(exc.stdout or ''), 'stderr':str(exc.stderr or '')}
        with (OUT / 'commands.jsonl').open('a') as handle:
            handle.write(json.dumps(entry)+'\n')
        raise
    entry = {'command': command, 'returncode': result.returncode,
             'wall_seconds': time.monotonic()-start, 'stdout': result.stdout, 'stderr': result.stderr}
    with (OUT / 'commands.jsonl').open('a') as handle:
        handle.write(json.dumps(entry)+'\n')
    if result.returncode:
        raise RuntimeError(json.dumps(entry))
    return entry


def simulation(name, rows, cols, version, scenario='mixed', rounds=12, warmup=2,
               tick=0, expiry=30):
    prefix = 'results/caas/'+name
    result = launch(rows*cols+1, './wsn_caas',
                    [str(rows), str(cols), '2', '--variant', str(version),
                     '--scenario', scenario, '--rounds', str(rounds), '--warmup', str(warmup),
                     '--tick', str(tick), '--expiry', str(expiry), '--output', prefix])
    text = (ROOT / (prefix+'.base0.log')).read_text()
    assert 'shutdown_acks='+str(rows*cols) in text, name
    assert 'measured_point_to_point_messages=' in text, name
    events = [json.loads(line) for line in text.splitlines() if line.startswith('{')]
    with (OUT/'runs.jsonl').open('a') as handle:
        handle.write(json.dumps(dict(run_id=name,rows=rows,cols=cols,ports=2,
            variant=version,scenario=scenario,rounds=rounds,warmup=warmup,tick=tick,
            expiry=expiry,wall_seconds=result['wall_seconds'],alerts=len(events)))+'\n')
    return result, events


def main():
    assert os.environ.get('SLURM_JOB_ID'), 'Use sbatch scripts/caas_audit.job'
    OUT.mkdir(exist_ok=True)
    test = launch(1, './tests/test_candidates', nodes=1)
    print(test['stdout'], flush=True)
    checks = []
    cases = [(1,1,'full'), (1,1,'empty'), (1,7,'full'), (7,1,'mixed'),
             (3,3,'ties'), (3,3,'full'), (2,5,'empty'), (3,4,'random')]
    for rows, cols, scenario in cases:
        versions = []
        for version in range(3):
            _, events = simulation('check_{}x{}_{}_v{}'.format(rows,cols,scenario,version),
                                   rows,cols,version,scenario,rounds=4,warmup=0)
            versions.append(sorted((e['round'],e['source'],e['free'],e['inferred_candidates']) for e in events))
        assert versions[0] == versions[1] == versions[2], (rows,cols,scenario)
        if scenario == 'full':
            assert len(versions[0]) == 4*rows*cols
            assert all(not e[3] for e in versions[0])
        if scenario == 'empty':
            assert not versions[0]
        if scenario == 'ties':
            assert all(e[1] == 0 and e[3] == [2,4,6] for e in versions[0])
        line = 'PASS {}x{} {}: all three versions; complete shutdown'.format(rows,cols,scenario)
        print(line, flush=True)
        checks.append(line)
    (OUT/'correctness.txt').write_text(test['stdout']+'\n'.join(checks)+'\n')
    fieldnames = ['topology','rows','cols','nodes','variant','repeat','prefix','wall_seconds','rounds','warmup','expiry','scenario']
    sizes = [('grid',2,2),('grid',2,3),('grid',3,3),('grid',3,4),('grid',3,5)]
    sizes += [('line',1,n) for n in (4,6,9,12,15)]
    jobs = [(topology,r,c,v,repeat) for topology,r,c in sizes for repeat in range(3) for v in range(3)]
    jobs.sort(key=lambda x:(x[0],x[1]*x[2],x[4],(x[3]-x[4])%3))
    with (OUT/'manifest.csv').open('w', newline='') as handle:
        writer = csv.DictWriter(handle,fieldnames=fieldnames)
        writer.writeheader()
        for index,(topology,rows,cols,version,repeat) in enumerate(jobs,1):
            name = '{}_{}x{}_v{}_r{}'.format(topology,rows,cols,version,repeat)
            result,_ = simulation(name,rows,cols,version)
            writer.writerow(dict(topology=topology,rows=rows,cols=cols,nodes=rows*cols,
                variant=version,repeat=repeat,prefix='results/caas/'+name,
                wall_seconds=result['wall_seconds'],rounds=12,warmup=2,expiry=30,scenario='mixed'))
            handle.flush()
            print('SCALING {}/{} {} {:.3f}s'.format(index,len(jobs),name,result['wall_seconds']),flush=True)
    for expiry in (0,0.1,1,30):
        simulation('expiry_'+str(expiry),3,3,2,rounds=6,warmup=0,tick=0.05,expiry=expiry)
    simulation('realistic_3x3',3,3,2,scenario='random',rounds=3,warmup=0,tick=10)
    benchmark = launch(1,'./tests/benchmark_candidates',nodes=1)
    (OUT/'candidate_benchmark.csv').write_text(benchmark['stdout'])
    print('PASS: 24 correctness runs, 90 scaling runs, 4 expiry runs, 1 realistic run, candidate tests and benchmark.',flush=True)


if __name__ == '__main__':
    main()
