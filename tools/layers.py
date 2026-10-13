#!/usr/bin/env python3
"""Calls and data that go the wrong way between the layers.

    python3 tools/layers.py [-v]

The layers, from the bottom: sys (src/sys and port/lib: files, memory, packing, input, the
clock, errors), core (src/core: the map and its files, no screen), ui (src/ui: drawing,
widgets, fonts, palettes, screens), editor (src/editor: the commands and the main loop).
Each may use the layers below it. A function or global used from a lower layer than its
own is a violation: listed with who uses it. Exit 1 when there are any.
"""
import collections, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LAYERS = [('sys', ['src/sys', 'port/lib']), ('core', ['src/core']), ('ui', ['src/ui']), ('editor', ['src/editor'])]
RANK = {name: i for i, (name, _) in enumerate(LAYERS)}
COMMENT = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"', re.S)
DEF_FN = re.compile(r'^(?!static)[A-Za-z_][\w \t\*]*?[\s\*]([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{', re.M)
DEF_VAR = re.compile(r'^(?!static|extern|typedef)[A-Za-z_][\w \t\*]*?[\s\*]\(?\*?([A-Za-z_]\w*)\)?\s*(?:\[[^\]]*\]\s*)*(?:=|;|\()', re.M)


def files():
    for layer, dirs in LAYERS:
        for d in dirs:
            for f in sorted(os.listdir(os.path.join(ROOT, d))):
                if f.endswith('.c'):
                    p = os.path.join(d, f)
                    yield layer, p, COMMENT.sub(' ', open(os.path.join(ROOT, p)).read())


def top_level(text):
    """the text outside function bodies, and each body"""
    out, bodies, depth, start, i = [], [], 0, 0, 0
    for m in re.finditer(r'[{}]', text):
        if m.group() == '{':
            if depth == 0:
                out.append(text[start:m.start()])
                start = m.start()
            depth += 1
        else:
            depth -= 1
            if depth == 0:
                bodies.append(text[start:m.end()])
                start = m.end()
    out.append(text[start:])
    return ''.join(out), bodies


def main():
    verbose = '-v' in sys.argv
    owner = {}
    texts = []
    for layer, p, t in files():
        texts.append((layer, p, t))
        for m in DEF_FN.finditer(t):
            owner.setdefault(m.group(1), (layer, p))
        outside, _ = top_level(t)
        for m in DEF_VAR.finditer(outside):
            owner.setdefault(m.group(1), (layer, p))
    bad = collections.defaultdict(set)
    for layer, p, t in texts:
        _, bodies = top_level(t)
        used = set(re.findall(r'\b([A-Za-z_]\w*)\b', ' '.join(bodies)))
        for name in used:
            if name in owner and RANK[owner[name][0]] > RANK[layer]:
                bad[(owner[name][0], name)].add('%s (%s)' % (os.path.basename(p), layer))
    by_pair = collections.Counter()
    for (lay, name), users in sorted(bad.items()):
        for u in users:
            by_pair[(u.split('(')[1][:-1], lay)] += 1
        if verbose:
            print('%-7s %-28s used by %s' % (lay, name, ', '.join(sorted(users))))
    for (lo, hi), n in sorted(by_pair.items()):
        print('%s uses %s: %d' % (lo, hi, n))
    print('%d name(s) used from below their layer' % len(bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
