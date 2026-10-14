#!/usr/bin/env python3
"""What the program never references: functions and data the linker can drop.

    python3 tools/dead_code.py [--build DIR] [--names]

Builds mapedit unoptimised and without inlining (so a function inlined into its only caller is
not mistaken for dead) with the linker's dead stripping and a link map, then lists every
symbol the editor's objects define that the link did not keep -- unreachable from main
through calls, function pointers or data. --names prints bare names, one a line, for
tools/remove_symbols.py --from. macOS only (ld64's -dead_strip and -map); a function-local
static is listed under its function and is not dead.
"""
import argparse, collections, glob, os, re, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--build', default=os.path.join(tempfile.gettempdir(), 'mapedit-dead-code'))
    ap.add_argument('--names', action='store_true')
    a = ap.parse_args()
    if sys.platform != 'darwin':
        sys.exit('macOS only: it reads ld64 link maps')
    mapfile = os.path.join(a.build, 'mapedit.map')
    subprocess.run(['cmake', '-S', ROOT, '-B', a.build, '-DCMAKE_BUILD_TYPE=Debug',
                    '-DCMAKE_C_FLAGS=-O0 -fno-inline',
                    '-DCMAKE_EXE_LINKER_FLAGS=-Wl,-dead_strip -Wl,-map,' + mapfile],
                   check=True, stdout=subprocess.DEVNULL)
    subprocess.run(['cmake', '--build', a.build, '-j8', '--target', 'mapedit'], check=True, stdout=subprocess.DEVNULL)
    live = open(mapfile, errors='replace').read().split('# Dead Stripped Symbols:')[0]
    kept = set(re.findall(r'\]\s+_(\w+)\s*$', live, re.M))
    defined = {}
    for o in glob.glob(os.path.join(a.build, 'CMakeFiles', 'editor.dir', 'src', '*', '*.c.o')):
        out = subprocess.run(['nm', '--defined-only', o], capture_output=True, text=True).stdout
        for line in out.split('\n'):
            p = line.split()
            if len(p) == 3 and p[1] in 'TtDdSsBb' and p[2].startswith('_') and '.' not in p[2]:
                defined[p[2][1:]] = (os.path.basename(o)[:-4], p[1] in 'Tt')
    dead = sorted((n, f, fn) for n, (f, fn) in defined.items() if n not in kept)
    if a.names:
        for n, _, _ in dead:
            print(n)
        return 0
    by = collections.defaultdict(list)
    for n, f, fn in dead:
        by[f].append(n + ('()' if fn else ''))
    for f in sorted(by):
        print('%-14s %s' % (f, ' '.join(by[f])))
    print('%d of %d symbols unreferenced' % (len(dead), len(defined)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
