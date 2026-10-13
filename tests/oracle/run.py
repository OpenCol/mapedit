#!/usr/bin/env python3
"""Run the oracle tests: each port function beside the original's machine code.

    python3 tests/oracle/run.py --lib build/liboracle_lib.dylib [--exe MAPEDIT.EXE] [-k name]

The original comes from the player's MAPEDIT.EXE ($MAPEDIT_EXE or --exe), run in a
real-mode emulator (emu.py, Python's unicorn package); the port's functions come from the
oracle library CMake builds. Each tests/oracle/test_*.py defines functions named test_*,
which take (machine, lib, rng) and raise AssertionError on the first difference. Exit 77
(skipped) when the EXE or unicorn is missing.
"""
import argparse, ctypes, glob, importlib.util, os, random, sys, traceback

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--lib', required=True)
    ap.add_argument('--exe', default=os.environ.get('MAPEDIT_EXE'))
    ap.add_argument('-k', help='only tests whose name contains this')
    ap.add_argument('--seed', type=int, default=1994)
    a = ap.parse_args()
    if not a.exe or not os.path.exists(a.exe):
        print('skipped: set MAPEDIT_EXE to the player\'s MAPEDIT.EXE')
        return 77
    try:
        import emu
    except ImportError:
        print('skipped: the unicorn Python package is not installed')
        return 77
    lib = ctypes.CDLL(os.path.abspath(a.lib))
    failed = passed = 0
    for path in sorted(glob.glob(os.path.join(HERE, 'test_*.py'))):
        spec = importlib.util.spec_from_file_location(os.path.basename(path)[:-3], path)
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        for name in sorted(n for n in dir(mod) if n.startswith('test_')):
            if a.k and a.k not in name:
                continue
            m = emu.Machine(a.exe)
            try:
                getattr(mod, name)(m, lib, random.Random(a.seed))
                passed += 1
                print('ok    %s' % name)
            except AssertionError as e:
                failed += 1
                print('FAIL  %s: %s' % (name, e))
            except Exception:
                failed += 1
                print('ERROR %s' % name)
                traceback.print_exc()
    print('%d passed, %d failed' % (passed, failed))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
