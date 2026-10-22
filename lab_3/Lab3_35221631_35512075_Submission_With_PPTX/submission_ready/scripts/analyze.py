"""Rebuild and check the retained local-cluster study from raw measurements.

Savin Vindiv De Alwis 35221631 sdea0018@student.monash.edu
Shamle Thilaksiri 35512075 wthi0003@student.monash.edu
Run: python3 scripts/analyze.py [--check-only]
Requires matplotlib only for chart regeneration. No measurements are generated.
"""
import argparse
import csv
import json
import math
from collections import defaultdict
from pathlib import Path
from statistics import mean, median

ROOT = Path(__file__).resolve().parents[1]
CLASSES = ('query', 'reply', 'report', 'redirect', 'control', 'flags', 'sync', 'ack', 'alert_roundtrip')


def rows(path):
    with path.open(newline='') as handle:
        return list(csv.DictReader(handle))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check-only', action='store_true')
    args = parser.parse_args()
    raw = ROOT / 'results/cluster'
    out = ROOT / 'results/analysis'
    manifest = rows(raw / 'manifest.csv')
    measurements = rows(raw / 'all_timing_measurements.csv')
    groups = defaultdict(list)
    keys = set()
    for item in measurements:
        key = (item['run_id'], int(item['world_rank']), item['class'])
        assert key not in keys, f'Duplicate raw record: {key}'
        keys.add(key)
        assert item['class'] in CLASSES
        for field in ('seconds', 'calls', 'payload_bytes'):
            number = float(item[field])
            assert math.isfinite(number) and number >= 0, (key, field)
        groups[item['run_id']].append(item)
    assert len(manifest) == 540
    assert len({(m['topology'], m['rows'], m['cols'], m['variant'], m['repeat']) for m in manifest}) == 540
    for topology in ('grid', 'line'):
        cases = [m for m in manifest if m['topology'] == topology]
        assert len({int(m['nodes']) for m in cases}) == 30
        assert len(cases) == 270
        for version in range(3):
            assert len([m for m in cases if int(m['variant']) == version]) == 90
    base_logs = (raw / 'all_base_station_logs.log').read_text()
    placement = defaultdict(dict)
    for item in rows(raw / 'host_placement.csv'):
        rank = int(item['world_rank'])
        assert int(item['station_rank']) == rank - 1
        assert rank not in placement[item['run_id']]
        placement[item['run_id']][rank] = item['hostname']
    means = []
    for case in manifest:
        run_id = Path(case['prefix']).name
        n = int(case['nodes'])
        assert n == int(case['rows']) * int(case['cols'])
        data = groups[run_id]
        assert len(data) == (n + 1) * len(CLASSES), run_id
        assert {int(x['world_rank']) for x in data} == set(range(n + 1))
        assert set(placement[run_id]) == set(range(1, n + 1))
        assert len(set(placement[run_id].values())) == 2
        anchor = f'===== {case["prefix"]}.base0.log ====='
        assert anchor in base_logs, run_id
        block = base_logs.split(anchor, 1)[1].split('=====', 1)[0]
        assert f'shutdown_acks={n}' in block, run_id
        events = [json.loads(line) for line in block.splitlines() if line.startswith('{')]
        metric_index = {(int(item['world_rank']),item['class']):item for item in data}
        row_count,col_count = int(case['rows']),int(case['cols'])
        def neighbours(station):
            row,col=divmod(station,col_count)
            return [j for j in (station-col_count if row else -1,
                station+col_count if row+1<row_count else -1,
                station-1 if col else -1,station+1 if col+1<col_count else -1) if j>=0]
        expected_events=[]
        expected_reports=defaultdict(int)
        expected_replies=defaultdict(int)
        for cycle in range(int(case['warmup'])+int(case['rounds'])):
            low=[cycle%3==0 or (station+cycle)%3!=0 for station in range(n)]
            for station in range(n):
                adjacent=neighbours(station)
                if low[station] and all(low[j] for j in adjacent):
                    expected_events.append((cycle,station))
                    if cycle>=int(case['warmup']):
                        expected_reports[station+1]+=1
                if cycle>=int(case['warmup']):
                    expected_replies[station+1]+=sum(low[j] for j in adjacent)
        assert sorted((e['round'],e['source']) for e in events)==expected_events,run_id
        for rank in range(1,n+1):
            counts={'query':len(neighbours(rank-1))*int(case['rounds']),
                    'reply':expected_replies[rank], 'report':expected_reports[rank],
                    'redirect':expected_reports[rank], 'alert_roundtrip':expected_reports[rank],
                    'control':int(case['rounds'])+1,'flags':int(case['rounds']),
                    'sync':int(case['rounds']),'ack':1}
            for category,expected in counts.items():
                assert float(metric_index[(rank,category)]['calls'])==expected,(run_id,rank,category)
            report_seconds=float(metric_index[(rank,'report')]['seconds'])
            redirect_seconds=float(metric_index[(rank,'redirect')]['seconds'])
            assert float(metric_index[(rank,'alert_roundtrip')]['seconds'])+1e-7>=report_seconds+redirect_seconds
        for event in events:
            source = event['source']
            assert event['coordinates'] == [source // int(case['cols']), source % int(case['cols'])]
            assert event['free'] == 0
            assert event['neighbour_bytes'] == event['neighbour_messages'] * 4
            assert event['inferred_candidates'] == sorted(set(event['inferred_candidates']))
            for candidate in event['inferred_candidates']:
                assert 0 <= candidate < n
                cols = int(case['cols'])
                assert abs(source // cols - candidate // cols) + abs(source % cols - candidate % cols) == 2
        for category in CLASSES:
            values = []
            for item in data:
                if int(item['world_rank']) == 0 or item['class'] != category or float(item['calls']) == 0:
                    continue
                denominator = int(case['rounds']) if category in ('query', 'reply') else float(item['calls'])
                values.append(float(item['seconds']) * 1e6 / denominator)
            assert values, (run_id, category)
            means.append({**case, 'class': category, 'mean_us': mean(values),
                          'max_rank_us': max(values), 'active_ranks': len(values)})
    old_means = rows(out / 'run_means.csv')
    assert len(old_means) == len(means) == 4860
    old_index = {(Path(m['prefix']).name, m['class']): m for m in old_means}
    for result in means:
        old = old_index[(Path(result['prefix']).name, result['class'])]
        for field in ('mean_us', 'max_rank_us', 'active_ranks'):
            assert math.isclose(float(old[field]), float(result[field]), rel_tol=1e-9, abs_tol=1e-8), (result['prefix'], result['class'], field)
    summarized = defaultdict(list)
    for item in means:
        key = (item['topology'], int(item['rows']), int(item['cols']), int(item['nodes']), int(item['variant']), item['class'])
        summarized[key].append(item['mean_us'])
    summaries = []
    for key, values in sorted(summarized.items()):
        assert len(values) == 3
        summaries.append(dict(zip(('topology', 'rows', 'cols', 'nodes', 'variant', 'message'), key),
                              median_us=median(values), min_us=min(values), max_us=max(values)))
    old_summaries = rows(out / 'all_message_delays.csv')
    index = {(x['topology'], int(x['nodes']), int(x['variant']), x['message']): x for x in summaries}
    assert len(old_summaries) == len(summaries) == 1620
    for old in old_summaries:
        result = index[(old['topology'], int(old['nodes']), int(old['variant']), old['message'])]
        for field in ('median_us', 'min_us', 'max_us'):
            assert math.isclose(float(old[field]), result[field], rel_tol=1e-9, abs_tol=1e-8)
    comparison = {}
    scores = {0: [], 1: [], 2: []}
    for topology in ('grid', 'line'):
        sizes = sorted({int(m['nodes']) for m in manifest if m['topology'] == topology})
        comparison[topology] = {}
        for category in ('query', 'report', 'redirect', 'alert_roundtrip'):
            comparison[topology][category] = {}
            for version in (1, 2):
                ratios = [index[(topology, n, 0, category)]['median_us'] / index[(topology, n, version, category)]['median_us'] for n in sizes]
                comparison[topology][category][f'v{version}_speedup_median'] = median(ratios)
                if category == 'alert_roundtrip':
                    scores[version].extend(ratios)
        scores[0].extend([1.0] * len(sizes))
    old_comparison = json.loads((out / 'comparison.json').read_text())
    for topology in comparison:
        for category in comparison[topology]:
            for field, value in comparison[topology][category].items():
                assert math.isclose(value, old_comparison[topology][category][field], rel_tol=1e-9)
    old_scores = json.loads((out / 'recommendation.json').read_text())['scores']
    for version, values in scores.items():
        assert math.isclose(median(values), old_scores[str(version)], rel_tol=1e-9)
    print(f'PASS: {len(measurements)} raw records; 540 scaling runs; 30 grid + 30 line sizes; three versions x three repeats; nine classes; all shutdown ACKs and two-host station placements.')
    print('PASS: all 4860 run means, 1620 plotted records, comparison ratios and selection scores independently recomputed from raw CSV.')
    print('PASS: every incident source/cycle and every station message/collective count in all 540 scaling runs matches an independent mixed-workload/topology model.')
    print('Source environment: supplied two-container shared-host TCP study, 18 October 2026. These are not CAAS measurements.')
    if not args.check_only:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        colors = ('#577590', '#df7d3c', '#198d8d')
        for category in CLASSES:
            fig, axes = plt.subplots(1, 2, figsize=(13, 4.6), layout='constrained')
            for ax, topology in zip(axes, ('grid', 'line')):
                for version in range(3):
                    values = sorted((s for s in summaries if s['topology'] == topology and s['variant'] == version and s['message'] == category), key=lambda s:s['nodes'])
                    xs = [s['nodes'] for s in values]
                    ax.plot(xs, [s['median_us'] for s in values], marker='.', linewidth=1.4, color=colors[version], label=f'V{version}')
                    ax.fill_between(xs, [s['min_us'] for s in values], [s['max_us'] for s in values], alpha=0.12, color=colors[version])
                ax.set(title=f'{topology.title()} topology: 30 sizes', xlabel='Charging stations (base excluded)', ylabel='Station endpoint elapsed time (microseconds)')
                ax.set_yscale('log')
                ax.grid(alpha=0.22)
                ax.legend()
            fig.suptitle(f'{category.replace("_", " ").title()} | Local container cluster (shared hardware, oversubscribed)', fontsize=13)
            fig.supxlabel('Median of three launch means; shading = observed min-max. Query/reply per cycle; other classes per operation.', fontsize=9)
            fig.savefig(out / f'{category}.png', dpi=180)
            plt.close(fig)
        print('Regenerated nine graphs from verified data; local origin and timing units labelled.')


if __name__ == '__main__':
    main()
