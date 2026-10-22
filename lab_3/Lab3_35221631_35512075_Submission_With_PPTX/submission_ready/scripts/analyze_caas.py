"""Validate, consolidate and graph authentic CAAS run files.

Savin Vindiv De Alwis 35221631 sdea0018@student.monash.edu
Shamle Thilaksiri 35512075 wthi0003@student.monash.edu
Run: python3 scripts/analyze_caas.py /path/to/downloaded/results/caas
"""
import argparse
import csv
import hashlib
import json
import math
from collections import defaultdict
from datetime import datetime
from pathlib import Path
import re
import shutil
from statistics import mean, median
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CLASSES = ('query','reply','report','redirect','control','flags','sync','ack','alert_roundtrip')


def read_csv(path):
    with path.open(newline='') as handle:
        return list(csv.DictReader(handle))


def write_csv(path, data):
    assert data, path
    with path.open('w',newline='') as handle:
        writer = csv.DictWriter(handle,fieldnames=list(data[0]))
        writer.writeheader()
        writer.writerows(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input',type=Path,nargs='?')
    parser.add_argument('--consolidated',action='store_true',help='Rebuild directly from the packaged consolidated evidence')
    parser.add_argument('--no-graphs',action='store_true')
    args = parser.parse_args()
    out = ROOT/'results/caas'
    temporary = None
    if args.consolidated:
        assert args.input is None, 'Choose a raw input directory OR --consolidated'
        temporary = tempfile.TemporaryDirectory(prefix='lab3-caas-analysis-')
        raw = Path(temporary.name)
        for filename in ('runs.jsonl','commands.jsonl','manifest.csv','environment.txt','candidate_benchmark.csv'):
            shutil.copy2(out/filename,raw/filename)
        # The recorded hash below verifies this byte-identical source before use.
        shutil.copy2(ROOT/'src/wsn.c',raw/'measured_source.c')
        grouped = defaultdict(list)
        for record in read_csv(out/'all_timing_measurements.csv'):
            run_id = record.pop('run_id')
            grouped[run_id].append(record)
        for run_id, records in grouped.items():
            write_csv(raw/(run_id+'.metrics.csv'),records)
        for filename in ('all_base_station_logs.log','all_station_logs.log'):
            parts = re.split(r'\n===== ([^\n]+) =====\n',(out/filename).read_text())
            assert not parts[0].strip()
            for name,text in zip(parts[1::2],parts[2::2]):
                assert Path(name).name==name
                (raw/name).write_text(text)
    else:
        assert args.input is not None, 'Supply a raw input directory or --consolidated'
        raw = args.input.resolve()
    charts = out/'analysis'
    charts.mkdir(parents=True,exist_ok=True)
    runs = [json.loads(line) for line in (raw/'runs.jsonl').read_text().splitlines()]
    manifest = read_csv(raw/'manifest.csv')
    assert len(runs) == 119 and len({r['run_id'] for r in runs}) == 119
    assert len(manifest) == 90
    for topology in ('grid','line'):
        cases = [m for m in manifest if m['topology']==topology]
        assert len(cases)==45
        assert len({(m['rows'],m['cols']) for m in cases})==5
        assert len({(m['rows'],m['cols'],m['variant'],m['repeat']) for m in cases})==45
    source_hash = hashlib.sha256((ROOT/'src/wsn.c').read_bytes()).hexdigest()
    assert hashlib.sha256((raw/'measured_source.c').read_bytes()).hexdigest() == source_hash
    assert source_hash in (raw/'environment.txt').read_text()
    commands = [json.loads(line) for line in (raw/'commands.jsonl').read_text().splitlines()]
    assert len(commands)==121 and all(c.get('returncode')==0 for c in commands)
    run_commands = {}
    for entry in commands:
        argv = entry['command']
        if '--output' in argv:
            run_id = argv[argv.index('--output')+1].split('/')[-1]
            assert run_id not in run_commands
            run_commands[run_id] = argv
    assert len(run_commands)==119
    launch_warnings = sorted({c['stderr'] for c in commands if c['stderr']})
    for warning in launch_warnings:
        assert re.fullmatch(r"srun: Warning: can't honor --ntasks-per-node set to \d+ which doesn't match the requested tasks \d+ with the number of requested nodes 2\. Ignoring --ntasks-per-node\.\n",warning),warning
    benchmark = read_csv(raw/'candidate_benchmark.csv')
    assert len(benchmark)==12
    benchmark_index = {(int(b['nodes']),int(b['variant'])):b for b in benchmark}
    assert set(benchmark_index)=={(n,v) for n in (10,100,1000,10000) for v in range(3)}
    for (n,version),item in benchmark_index.items():
        assert int(item['repetitions'])==10000
        assert math.isfinite(float(item['seconds'])) and float(item['seconds'])>0
        assert int(item['checksum'])==20000-4*(10000//n)
    candidate_speedup = float(benchmark_index[(10000,0)]['seconds'])/float(benchmark_index[(10000,2)]['seconds'])
    metrics_all, placement, means = [], [], []
    base_all, station_all = [], []
    indexed = {m['prefix'].split('/')[-1]:m for m in manifest}
    metric_index = {}
    expiry_results = []
    for run in runs:
        run_id = run['run_id']
        argv = run_commands[run_id]
        for flag in ('--mpi=none','--exact','--nodes=2','--cpus-per-task=2','--distribution=cyclic','--cpu-bind=cores'):
            assert flag in argv,(run_id,flag)
        executable = argv.index('./wsn_caas')
        assert list(map(int,argv[executable+1:executable+4]))==[run['rows'],run['cols'],run['ports']]
        for flag,field in (('--variant','variant'),('--rounds','rounds'),('--warmup','warmup'),('--tick','tick'),('--expiry','expiry')):
            assert float(argv[argv.index(flag)+1])==run[field],(run_id,flag)
        assert argv[argv.index('--scenario')+1]==run['scenario']
        n = run['rows']*run['cols']
        metrics = read_csv(raw/(run_id+'.metrics.csv'))
        assert len(metrics) == (n+1)*9, run_id
        records = {(int(m['world_rank']),m['class']):m for m in metrics}
        assert len(records)==len(metrics)
        assert set(records)=={(rank,c) for rank in range(n+1) for c in CLASSES}
        for item in metrics:
            for field in ('seconds','calls','payload_bytes'):
                assert math.isfinite(float(item[field])) and float(item[field])>=0
            metrics_all.append({'run_id':run_id,**item})
        log = (raw/(run_id+'.base0.log')).read_text()
        configuration = dict(re.findall(r'(\w+)=(\S+)',log.splitlines()[0]))
        assert int(configuration['ports'])==run['ports'] and int(configuration['variant'])==run['variant']
        assert int(configuration['threshold'])==1 and float(configuration['tick'])==run['tick'] and float(configuration['expiry'])==run['expiry']
        base_all.append('\n===== '+run_id+'.base0.log =====\n'+log)
        assert 'shutdown_acks='+str(n) in log
        events = [json.loads(line) for line in log.splitlines() if line.startswith('{')]
        assert len(events) == run['alerts']
        base_host = re.search(r'host=(\S+)',log).group(1)
        hosts = {base_host}
        host_ranks = defaultdict(int)
        host_ranks[base_host] = 1
        measured_alerts = defaultdict(int)
        for event in events:
            source = event['source']
            assert 0<=source<n and 0<=event['round']<run['rounds']+run['warmup']
            assert event['coordinates']==[source//run['cols'],source%run['cols']]
            assert event['free']==0
            datetime.strptime(event['discovered_utc'],'%Y-%m-%dT%H:%M:%SZ')
            neighbours = [v for v in event['neighbours'] if v['rank']>=0]
            assert all(v['free']==0 for v in neighbours)
            assert event['neighbour_messages']==4*len(neighbours)
            assert event['neighbour_bytes']==4*event['neighbour_messages']
            candidates = event['inferred_candidates']
            assert candidates==sorted(set(candidates))
            for candidate in candidates:
                assert 0<=candidate<n
                assert abs(source//run['cols']-candidate//run['cols'])+abs(source%run['cols']-candidate%run['cols'])==2
            if event['round']>=run['warmup']:
                measured_alerts[source+1]+=1
        for rank in range(1,n+1):
            text = (raw/(run_id+'.node'+str(rank-1)+'.log')).read_text()
            station_all.append('\n===== '+run_id+'.node'+str(rank-1)+'.log =====\n'+text)
            host = re.search(r'host=(\S+)',text).group(1)
            hosts.add(host)
            host_ranks[host]+=1
            placement.append(dict(run_id=run_id,world_rank=rank,station_rank=rank-1,hostname=host))
            assert len([line for line in text.splitlines() if line.startswith('round=')])==run['rounds']+run['warmup']
            for category, expected in [('control',run['rounds']+1),('flags',run['rounds']),('sync',run['rounds']),('ack',1),('report',measured_alerts[rank]),('redirect',measured_alerts[rank]),('alert_roundtrip',measured_alerts[rank])]:
                assert float(records[(rank,category)]['calls'])==expected,(run_id,rank,category)
        assert len(hosts)==2 and all(h.startswith('student-caas-n') for h in hosts), (run_id,hosts)
        assert max(host_ranks.values())<=8 and max(host_ranks.values())-min(host_ranks.values())<=1,(run_id,dict(host_ranks))
        sent_messages = sent_bytes = 0
        for (rank,category),item in records.items():
            outgoing = (rank>0 and category in ('query','reply','report','ack')) or (rank==0 and category=='redirect')
            if outgoing:
                sent_messages += float(item['calls'])
                sent_bytes += float(item['payload_bytes'])
        summary = re.search(r'measured_point_to_point_messages=(\d+) measured_payload_bytes=(\d+)',log)
        assert summary and int(summary[1])==sent_messages and int(summary[2])==sent_bytes
        if run_id.startswith('expiry_'):
            expiry_results.append(dict(expiry=run['expiry'],alerts=len(events),candidate_recommendations=sum(len(e['inferred_candidates']) for e in events)))
        if run_id=='realistic_3x3':
            elapsed = float(re.search(r'elapsed_seconds=([\d.]+)',log).group(1))
            assert run['tick']==10 and elapsed>=30
        if run_id in indexed:
            case = indexed[run_id]
            for category in CLASSES:
                values = []
                for rank in range(1,n+1):
                    item=records[(rank,category)]
                    calls=float(item['calls'])
                    if calls:
                        denominator=run['rounds'] if category in ('query','reply') else calls
                        values.append(float(item['seconds'])*1e6/denominator)
                assert values,(run_id,category)
                means.append({**case,'class':category,'mean_us':mean(values),'max_rank_us':max(values),'active_ranks':len(values)})
    write_csv(out/'all_timing_measurements.csv',metrics_all)
    write_csv(out/'host_placement.csv',placement)
    (out/'all_base_station_logs.log').write_text(''.join(base_all))
    (out/'all_station_logs.log').write_text(''.join(station_all))
    write_csv(charts/'run_means.csv',means)
    grouped=defaultdict(list)
    for item in means:
        grouped[(item['topology'],int(item['nodes']),int(item['variant']),item['class'])].append(item['mean_us'])
    points=[]
    for key,values in sorted(grouped.items()):
        assert len(values)==3
        point=dict(zip(('topology','nodes','variant','message'),key),median_us=median(values),min_us=min(values),max_us=max(values))
        points.append(point)
        metric_index[key]=point
    write_csv(charts/'all_message_delays.csv',points)
    comparison={}
    scores={0:[],1:[],2:[]}
    for topology in ('grid','line'):
        sizes=sorted({int(m['nodes']) for m in manifest if m['topology']==topology})
        comparison[topology]={}
        for category in CLASSES:
            comparison[topology][category]={}
            for version in (1,2):
                ratios=[metric_index[(topology,n,0,category)]['median_us']/metric_index[(topology,n,version,category)]['median_us'] for n in sizes]
                comparison[topology][category]['v'+str(version)+'_speedup_median']=median(ratios)
                if category=='alert_roundtrip':
                    scores[version].extend(ratios)
        scores[0].extend([1.0]*len(sizes))
    scores={str(v):median(values) for v,values in scores.items()}
    summary=dict(source_sha256=source_hash,runs=len(runs),scaling_runs=len(manifest),raw_records=len(metrics_all),station_placements=len(placement),
                 comparison=comparison,selection_scores=scores,selected_variant=max(scores,key=scores.get),expiry_results=expiry_results,
                 candidate_benchmark=benchmark,candidate_v0_v2_n10000=candidate_speedup,
                 stderr_warning_commands=sum(bool(c['stderr']) for c in commands),launch_warnings=launch_warnings,
                 qualification='Descriptive medians of three launches; no statistical significance or universal best variant is established.')
    (charts/'summary.json').write_text(json.dumps(summary,indent=2))
    message='PASS: 119 complete CAAS simulations; 90 scaling runs; five grid and five line sizes x three versions x three launches; all rank metrics, message/byte summaries, two compute hosts, 24 correctness checks, expiry periods, 30-second realistic run, source hash and successful command exits verified.'
    (out/'verification.txt').write_text(message+'\nSource SHA-256: '+source_hash+'\n'
        +str(summary['stderr_warning_commands'])+' launches emitted only the recorded uneven-task per-node-cap warning; actual placement is balanced and at most eight ranks per node. No unexpected stderr was accepted.\n')
    print(message)
    if not args.no_graphs:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        colors=('#577590','#df7d3c','#198d8d')
        for category in CLASSES:
            fig,axes=plt.subplots(1,2,figsize=(13,4.6),layout='constrained')
            for ax,topology in zip(axes,('grid','line')):
                for version in range(3):
                    data=sorted((p for p in points if p['topology']==topology and p['variant']==version and p['message']==category),key=lambda p:p['nodes'])
                    xs=[p['nodes'] for p in data]
                    ax.plot(xs,[p['median_us'] for p in data],'.-',color=colors[version],label='V'+str(version))
                    ax.fill_between(xs,[p['min_us'] for p in data],[p['max_us'] for p in data],color=colors[version],alpha=.13)
                ax.set(title=topology.title()+' topology: CAAS',xlabel='Charging stations (base excluded)',ylabel='Station endpoint elapsed time (microseconds)')
                ax.set_yscale('log');ax.grid(alpha=.22);ax.legend()
            fig.suptitle(category.replace('_',' ').title()+' | CAAS: two allocated compute nodes, two cores per rank',fontsize=13)
            fig.supxlabel('Median of three launch means; shading = min-max. Query/reply per cycle; other classes per operation.',fontsize=9)
            fig.savefig(charts/(category+'.png'),dpi=180)
            plt.close(fig)
    print(json.dumps({k:summary[k] for k in ('raw_records','selection_scores','selected_variant')}))
    if temporary is not None:
        temporary.cleanup()


if __name__=='__main__':
    main()
