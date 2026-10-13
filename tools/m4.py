#!/usr/bin/env python3
"""Gate M4: one keyboard session, played into the original under DOSBox-X and into the port.

    python3 tools/m4.py --game DIR [--port build/mapedit] SESSION...

DIR is the player's Colonization for DOS directory (MAPEDIT.EXE and the game's files). For
each SESSION, two fresh copies of DIR are made; the original runs in one under DOSBox-X
(dosbox-x on the PATH, typing through its AUTOTYPE command), the port in the other
(headless, through --script). Every file is compared afterwards: the session passes when
the two directories end up byte-identical, and the report says which files the session
changed.

A session is the keys, in DOSBox-X's AUTOTYPE names (`autotype -list`), whitespace
separated, # to the end of a line a comment:

    pace 1.0          # seconds between keys (default 1.0); `wait 6` before the first
    enter             # open the map the file menu offers first
    8 8 6 bspace      # move the cursor, pick up the terrain under it
    3 3 space         # move, paint
    lalt s enter      # Editor > Save Map > Yes

Each key is pressed and released; lalt/ralt/lshift/lctrl alone are pressed and released
with nothing else (Alt alone opens the menu bar). Only keys: AUTOTYPE cannot move a mouse,
and the editor drives everything from the keyboard (the keypad, kp_*, is its mouse).
"""
import argparse, filecmp, os, shutil, subprocess, sys, tempfile, time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

SDL = {'enter': 'Return', 'esc': 'Escape', 'space': 'Space', 'bspace': 'Backspace', 'tab': 'Tab',
       'up': 'Up', 'down': 'Down', 'left': 'Left', 'right': 'Right', 'home': 'Home', 'end': 'End',
       'pageup': 'PageUp', 'pagedown': 'PageDown', 'insert': 'Insert', 'delete': 'Delete',
       'minus': '-', 'equals': '=', 'comma': ',', 'period': '.', 'slash': '/', 'semicolon': ';',
       'quote': "'", 'lbracket': '[', 'rbracket': ']', 'backslash': '\\', 'grave': '`',
       'kp_period': 'Keypad .', 'kp_enter': 'Keypad Enter', 'kp_minus': 'Keypad -',
       'kp_plus': 'Keypad +', 'kp_multiply': 'Keypad *', 'kp_divide': 'Keypad /'}
HOLD = {'lalt': 'alt', 'ralt': 'alt', 'lshift': 'shift', 'rshift': 'shift', 'lctrl': 'ctrl', 'rctrl': 'ctrl'}


def parse(path):
    pace, wait, keys = 1.0, 6.0, []
    for line in open(path):
        toks = line.split('#')[0].split()
        if toks[:1] == ['pace']:
            pace = float(toks[1])
        elif toks[:1] == ['wait']:
            wait = float(toks[1])
        else:
            keys += toks
    return pace, wait, keys


def port_script(keys, pace, wait, out):
    lines = ['wait %d' % (wait * 1000)]
    for k in keys:
        if k in HOLD:
            lines += ['hold ' + HOLD[k], 'wait 150', 'hold']
        elif k.startswith('kp_') and k[3:].isdigit():
            lines.append('key Keypad ' + k[3:])
        elif k in SDL:
            lines.append('key ' + SDL[k])
        elif len(k) == 1 and (k.isalnum()):
            lines.append('key ' + k.upper())
        elif k.startswith('f') and k[1:].isdigit():
            lines.append('key F' + k[1:])
        else:
            sys.exit('%s: no port name for key %r' % (out, k))
        lines.append('wait %d' % (pace * 1000))
    lines += ['wait 1500', 'exit']
    open(out, 'w').write('\n'.join(lines) + '\n')


def copy_game(game, dst):
    shutil.copytree(game, dst)
    for dirpath, _, fs in os.walk(dst):
        os.chmod(dirpath, 0o755)
        for f in fs:
            os.chmod(os.path.join(dirpath, f), 0o644)


def run_dos(gdir, keys, pace, wait):
    total = wait + pace * len(keys) + 4
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
    cmd = ['dosbox-x', '-defaultconf', '-nopromptfolder', '-set', 'cpu cycles=max',
           '-set', 'dosbox quit warning=false', '-c', 'mount c "%s"' % gdir, '-c', 'c:',
           '-c', 'autotype -w %g -p %g %s' % (wait, pace, ' '.join(keys)), '-c', 'MAPEDIT']
    p = subprocess.Popen(cmd, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        p.wait(timeout=total)
    except subprocess.TimeoutExpired:
        p.kill()
        p.wait()


def run_port(gdir, port, script, timeout):
    p = subprocess.run([port, '--headless', '--game', gdir, '--script', script],
                       capture_output=True, text=True, timeout=timeout)
    return p.returncode, p.stdout


def bytes_differ(x, y):
    a, b = open(x, 'rb').read(), open(y, 'rb').read()
    return sum(p != q for p, q in zip(a, b)) + abs(len(a) - len(b))


def snapshot(d):
    return {os.path.relpath(os.path.join(dp, f), d).upper(): os.path.join(dp, f)
            for dp, _, fs in os.walk(d) for f in fs}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--game', default=os.environ.get('MAPEDIT_GAME'))
    ap.add_argument('--port', default=os.path.join(ROOT, 'build', 'mapedit'))
    ap.add_argument('--keep', action='store_true', help='keep the two directories')
    ap.add_argument('sessions', nargs='+')
    a = ap.parse_args()
    if not a.game or not os.path.exists(os.path.join(a.game, 'MAPEDIT.EXE')):
        sys.exit('pass --game: the Colonization directory with MAPEDIT.EXE')
    if not shutil.which('dosbox-x'):
        sys.exit('dosbox-x is not on the PATH')
    failed = 0
    orig = snapshot(a.game)
    for s in a.sessions:
        pace, wait, keys = parse(s)
        tmp = tempfile.mkdtemp(prefix='m4-')
        dos, prt = os.path.join(tmp, 'dos'), os.path.join(tmp, 'port')
        copy_game(a.game, dos)
        copy_game(a.game, prt)
        script = os.path.join(tmp, 'session.txt')
        port_script(keys, pace, wait, script)
        t0 = time.time()
        # DOSBox-X drops a typed key now and then (one typed while the program starts, or
        # draws the map): a run in which the original wrote nothing is retried, so a lost
        # key is never reported as the port's difference
        for attempt in range(3):
            run_dos(dos, keys, pace, wait)
            A = snapshot(dos)
            if any(n not in orig or not filecmp.cmp(A[n], orig[n], shallow=False) for n in A):
                break
            print('  %s: the original wrote nothing (attempt %d); again' % (os.path.basename(s), attempt + 1))
            shutil.rmtree(dos)
            copy_game(a.game, dos)
        rc, out = run_port(prt, a.port, script, wait + pace * len(keys) + 30)
        B = snapshot(prt)
        differ = sorted(n for n in set(A) | set(B)
                        if n not in A or n not in B or not filecmp.cmp(A[n], B[n], shallow=False))
        changed = sorted('%s (%s)' % (n, 'new' if n not in orig else '%d bytes' % bytes_differ(B[n], orig[n]))
                         for n in B if n not in orig or not filecmp.cmp(B[n], orig[n], shallow=False))
        ok = not differ and rc == 0
        failed += not ok
        print('%s %s: %d keys, %.0fs; the session changed %s%s%s'
              % ('PASS' if ok else 'FAIL', os.path.basename(s), len(keys), time.time() - t0,
                 ', '.join(changed) or 'nothing',
                 '; differ: ' + ', '.join(differ) if differ else '',
                 '; port exit %d %s' % (rc, out.strip()[:200]) if rc else ''))
        if a.keep or not ok:
            print('  kept', tmp)
        else:
            shutil.rmtree(tmp)
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
