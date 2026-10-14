#!/usr/bin/env python3
"""Did a change touch only comments and layout?

    python3 tools/same_code.py [--rev REV] [FILE...]

Compares each file (default: every .c and .h under src/ and port/ that differs from REV,
default HEAD) with its version at REV, both stripped of comments and of whitespace outside
string and character literals. Prints the files whose code changed; exit 1 if any did. For
documentation passes: comments may be rewritten freely, the code may not move a token.
"""
import argparse, os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def code(text):
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith('/*', i):
            j = text.find('*/', i + 2)
            i = n if j < 0 else j + 2
        elif text.startswith('//', i):
            j = text.find('\n', i)
            i = n if j < 0 else j
        elif c in '"\'':
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == '\\' else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif c.isspace():
            i += 1
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--rev', default='HEAD')
    ap.add_argument('files', nargs='*')
    a = ap.parse_args()
    files = a.files
    if not files:
        r = subprocess.run(['git', '-C', ROOT, 'diff', '--name-only', a.rev, '--', 'src', 'port'],
                           capture_output=True, text=True, check=True)
        files = [f for f in r.stdout.split() if f.endswith(('.c', '.h'))]
    changed = []
    for f in files:
        rel = os.path.relpath(os.path.abspath(f), ROOT)
        old = subprocess.run(['git', '-C', ROOT, 'show', '%s:%s' % (a.rev, rel)], capture_output=True, text=True)
        if old.returncode:
            changed.append(rel + ' (new)')
            continue
        if code(old.stdout) != code(open(os.path.join(ROOT, rel)).read()):
            changed.append(rel)
    for f in changed:
        print('code changed:', f)
    print('%d file(s) checked, %d with code changes' % (len(files), len(changed)))
    return 1 if changed else 0


if __name__ == '__main__':
    sys.exit(main())
