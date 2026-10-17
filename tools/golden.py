#!/usr/bin/env python3
"""Golden tests: scripted sessions whose screens and saved files must not change.

    python3 tools/golden.py --game DIR [--port build/mapedit] [SESSION...]   # check
    python3 tools/golden.py --game DIR --update [SESSION...]                 # record
    python3 tools/golden.py --game DIR --port build-win/mapedit.exe --wine     # a Windows build

Each session (tests/golden/*.txt, the host's script language: key, type, move, click, hold,
wait, shot) runs headless on the script's own clock, with the editor's options given on a
line "# args: ..." (the port's improvements, port/main.c), if it has one, so it shows the same screens every
time, in a fresh copy of the game directory. What it produces -- every `shot`, and every file
it creates or changes in the game directory -- is hashed (SHA-256) and compared with
tests/golden/expected.json. Only hashes are kept: the screens are renders of the game's art,
which this repository does not carry. A mismatch keeps the session's output under
build/golden/<session>/ (the screenshots as BMP) and fails.

They guard refactoring: the port's behaviour, screens included, stays what it was. They do
not say the behaviour is the original's -- tools/m4.py checks that, against DOSBox-X. Without
the game (no --game, no $MAPEDIT_GAME) the tests are skipped (exit 77, as CTest expects).
"""
import argparse, hashlib, json, os, shutil, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SESSIONS = os.path.join(ROOT, 'tests', 'golden')
EXPECTED = os.path.join(SESSIONS, 'expected.json')


def sha(path):
    return hashlib.sha256(open(path, 'rb').read()).hexdigest()


def copy_game(game, dst):
    shutil.copytree(game, dst, ignore=shutil.ignore_patterns('*.app', 'mapedit*', '*.dll'))
    for dp, _, fs in os.walk(dst):
        os.chmod(dp, 0o755)
        for f in fs:
            os.chmod(os.path.join(dp, f), 0o644)


def run_session(path, game, port, keep, wine=None):
    """{output name: sha256}, the run's directory, the program's exit status"""
    name = os.path.splitext(os.path.basename(path))[0]
    work = tempfile.mkdtemp(prefix='golden-')
    gdir = os.path.join(work, 'game')
    copy_game(game, gdir)
    before = {f: sha(os.path.join(gdir, f)) for f in os.listdir(gdir) if os.path.isfile(os.path.join(gdir, f))}
    shots = os.path.join(work, 'shots')
    os.makedirs(shots)
    script = os.path.join(work, 'session.txt')
    text = open(path).read()
    if not any(l.strip() == 'exit' for l in text.splitlines()):
        text += '\nexit\n'
    open(script, 'w').write(text)
    args = [w for l in text.splitlines() if l.startswith('# args:') for w in l[7:].split()]
    cmd, env = [port, '--headless', '--game', gdir, '--script', script] + args, None
    if wine:                                    # a Windows build: paths on Wine's Z: drive
        cmd = [wine, port, '--headless', '--game', 'Z:' + gdir, '--script', 'Z:' + script] + args
        env = dict(os.environ, WINEDEBUG='-all')
    p = subprocess.run(cmd, cwd=shots, capture_output=True, text=True, timeout=300, env=env)
    out = {}
    for f in sorted(os.listdir(shots)):
        out['shot ' + f] = sha(os.path.join(shots, f))
    for f in sorted(os.listdir(gdir)):
        fp = os.path.join(gdir, f)
        if os.path.isfile(fp) and before.get(f) != sha(fp):
            out['file ' + f.upper()] = sha(fp)
    if keep:
        dst = os.path.join(ROOT, 'build', 'golden', name)
        shutil.rmtree(dst, ignore_errors=True)
        os.makedirs(dst)
        for f in os.listdir(shots):
            shutil.copy(os.path.join(shots, f), dst)
        open(os.path.join(dst, 'log.txt'), 'w').write(p.stdout + p.stderr)
    shutil.rmtree(work)
    return name, out, p.returncode


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--game', default=os.environ.get('MAPEDIT_GAME'))
    ap.add_argument('--port', default=os.path.join(ROOT, 'build', 'mapedit'))
    ap.add_argument('--update', action='store_true', help='record what the sessions produce now')
    ap.add_argument('--wine', action='store_const', const=os.environ.get('WINE', 'wine64'),
                    help='--port is a Windows build: run it under Wine ($WINE, default wine64)')
    ap.add_argument('sessions', nargs='*')
    a = ap.parse_args()
    a.port = os.path.abspath(a.port)            # sessions run in their own directory
    if not a.game or not os.path.isdir(a.game):
        print('skipped: no game directory (--game or $MAPEDIT_GAME)')
        return 77
    sessions = a.sessions or sorted(os.path.join(SESSIONS, f) for f in os.listdir(SESSIONS) if f.endswith('.txt'))
    expected = json.load(open(EXPECTED)) if os.path.exists(EXPECTED) else {}
    failed = 0
    for s in sessions:
        name, got, rc = run_session(s, a.game, a.port, keep=False, wine=a.wine)
        if a.update:
            expected[name] = got
            print('recorded %-16s %d outputs%s' % (name, len(got), ' (exit %d)' % rc if rc else ''))
            continue
        want = expected.get(name)
        bad = [] if want == got else sorted(k for k in set(want or {}) | set(got) if (want or {}).get(k) != got.get(k))
        if want is None:
            bad = ['(no expected outputs: --update records them)']
        if rc:
            bad.append('exit status %d' % rc)
        if bad:
            failed += 1
            run_session(s, a.game, a.port, keep=True, wine=a.wine)
            print('FAIL %-16s %s  (output kept in build/golden/%s)' % (name, ', '.join(bad), name))
        else:
            print('ok   %-16s %d outputs' % (name, len(got)))
    if a.update:
        json.dump(expected, open(EXPECTED, 'w'), indent=1, sort_keys=True)
        open(EXPECTED, 'a').write('\n')
    else:
        print('%d of %d sessions changed' % (failed, len(sessions)))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
