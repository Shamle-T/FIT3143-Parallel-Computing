"""Integration checks against independent expected outcomes, not random luck."""
import json
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
LAUNCH = ['mpirun', '--oversubscribe', '--bind-to', 'none']


def run(rows, cols, scenario, variant, ports=2, extra=()):
    prefix = f'results/check_{rows}_{cols}_{scenario}_{variant}'
    subprocess.run(LAUNCH + ['-np', str(rows*cols+1), './wsn', str(rows), str(cols), str(ports),
                   '--rounds', '4', '--tick', '0', '--scenario', scenario, '--variant', str(variant),
                   '--output', prefix, *extra], check=True, timeout=90, cwd=ROOT)
    text = (ROOT/f'{prefix}.base0.log').read_text()
    assert f'shutdown_acks={rows*cols}' in text
    events = [json.loads(line) for line in text.splitlines() if line.startswith('{')]
    return sorted((e['round'], e['source'], e['free'], e['inferred_candidates']) for e in events)


if __name__ == '__main__':
    subprocess.run(['mpicc', '-std=c11', '-O2', '-fopenmp', '-Wall', '-Wextra', '-Werror',
                    'tests/test_candidates.c', '-o', 'tests/test_candidates', '-lm'], cwd=ROOT, check=True)
    subprocess.run(['tests/test_candidates'], cwd=ROOT, check=True)
    cases = [(1, 1, 'full'), (1, 1, 'empty'), (1, 7, 'full'), (7, 1, 'mixed'),
             (3, 3, 'ties'), (3, 3, 'full'), (2, 5, 'empty'), (3, 4, 'random')]
    for rows, cols, scenario in cases:
        results = [run(rows, cols, scenario, v) for v in range(3)]
        assert results[0] == results[1] == results[2], (rows, cols, scenario)
        if scenario == 'empty':
            assert not results[0]
        if scenario == 'full':
            assert len(results[0]) == 4*rows*cols
            assert all(not e[3] for e in results[0])
        if scenario == 'ties':
            assert all(e[1] == 0 and e[3] == [2, 4, 6] for e in results[0])
        print('PASS', rows, cols, scenario, 'all three variants', flush=True)
    run(2, 2, 'random', 2, ports=6, extra=('--threshold', '3', '--expiry', '0'))
    for args in [('0', '2', '2'), ('2', '2', '0'), ('1', '1', '2'), ('x', '1', '2')]:
        p = subprocess.run(LAUNCH+['-np', '3', './wsn', *args], cwd=ROOT,
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
        assert p.returncode != 0, args
    for args in [('--tick', 'nan'), ('--tick', '-1'), ('--tick', '1e100'), ('--expiry', 'inf')]:
        p = subprocess.run(LAUNCH+['-np', '2', './wsn', '1', '1', '2', *args], cwd=ROOT,
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
        assert p.returncode != 0, args
    print('PASS nonfinite, negative and overflowing time arguments', flush=True)
    print('PASS invalid inputs/process counts; 25 successful integration runs', flush=True)
