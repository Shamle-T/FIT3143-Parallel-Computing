"""Create an isolated two-container MPI cluster. No host ports are published."""
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
DOCKER = r'C:\Program Files\Docker\Docker\resources\bin\docker.exe'


def docker(*args, **kwargs):
    return subprocess.run([DOCKER, *args], check=True, **kwargs)


if __name__ == '__main__':
    docker('build', '-t', 'fit3143-lab3:local', '-f', str(ROOT/'scripts/Dockerfile'), str(ROOT/'scripts'))
    names = subprocess.check_output([DOCKER, 'network', 'ls', '--format', '{{.Name}}'], text=True).splitlines()
    if 'fit3143-lab3-net' not in names:
        docker('network', 'create', 'fit3143-lab3-net')
    existing = subprocess.check_output([DOCKER, 'ps', '-a', '--format', '{{.Names}}'], text=True).splitlines()
    for name in ('fit3143-lab3-a', 'fit3143-lab3-b'):
        if name not in existing:
            docker('run', '-d', '--name', name, '--hostname', name, '--network', 'fit3143-lab3-net',
                   '--mount', f'type=bind,source={ROOT},target=/lab', 'fit3143-lab3:local')
        else:
            docker('start', name)
    docker('exec', 'fit3143-lab3-a', 'mpicc', '-std=c11', '-O2', '-g', '-fopenmp',
           '-Wall', '-Wextra', '-Wpedantic', '-Werror', 'src/wsn.c', '-o', 'wsn_cluster', '-lm')
    docker('exec', 'fit3143-lab3-a', 'mpirun', '--allow-run-as-root', '--oversubscribe',
           '--bind-to', 'none', '--mca', 'btl', 'self,tcp', '--host',
           'fit3143-lab3-a:64,fit3143-lab3-b:64', '--map-by', 'node', '-np', '2', 'hostname')
