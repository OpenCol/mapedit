#!/usr/bin/env python3
"""Every function's declarations against its definition, across the imported modules.

    python3 tools/proto_audit.py [--all]

Each 1994 module declared the functions it called itself, and MSC 6 could not see when a
declaration disagreed with the definition. Under _fastcall it often could not matter: the
first near pointer went in BX and the first word or long in AX(:DX) wherever they stood in
the list, so fileio_fread_f(p, size, fp, n) and (p, size, n, fp) compiled to the same bytes.
On a modern compiler the arguments go where the declaration says, so such a disagreement is
a wrong argument at run time.

Reports, per function, every declaration whose parameter types differ from the
definition's: a different count, or a position where one is a pointer and the other an
integer, or two integers of different widths. Pointer-to-pointer differences (char * and
void *, two record views) are left out unless --all, and so are empty definitions (the
editor's stand-ins for the game's functions), which ignore whatever they are passed.
"""
import argparse, collections, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

COMMENT = re.compile(r'/\*.*?\*/|//[^\n]*', re.S)
# a declaration or definition at the top level: type, name, (params), then ; or {
PROTO = re.compile(r'^(?:extern\s+|static\s+)?([A-Za-z_][\w \t\*]*?[\s\*])([A-Za-z_]\w*)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*([;{])',
                   re.M)
KEYWORDS = {'if', 'while', 'for', 'switch', 'return', 'sizeof', 'else'}
WIDTH = {'i8': 1, 'u8': 1, 'char': 1, 'i16': 2, 'u16': 2, 'short': 2, 'i32': 4, 'u32': 4, 'long': 4}


def params(text):
    text = text.strip()
    if text in ('', 'void'):
        return []
    out, depth, cur = [], 0, ''
    for c in text:
        if c == ',' and depth == 0:
            out.append(cur)
            cur = ''
            continue
        depth += c == '('
        depth -= c == ')'
        cur += c
    out.append(cur)
    return [norm(p) for p in out]


def norm(p):
    """A parameter's type without its name: 'const char *s' -> 'char *'."""
    p = re.sub(r'\b(register|const|volatile)\b', ' ', p).strip()
    if p == '...':
        return p
    if '(' in p:                                       # a function pointer
        return 'fnptr'
    p = re.sub(r'\[[^\]]*\]', '*', p)                  # an array parameter is a pointer
    m = re.match(r'^(.*?)([A-Za-z_]\w*)?\s*$', p)
    base = m.group(1) if m.group(2) and m.group(1).strip() else p
    return re.sub(r'\s+', ' ', base.replace('*', ' * ')).strip().replace(' *', '*')


def kind(t):
    if t.endswith('*') or t == 'fnptr':
        return 'ptr'
    w = WIDTH.get(t.split()[-1] if t.split() else t)
    return 'int%d' % w if w else t


def scan():
    defs, decls = {}, collections.defaultdict(list)
    # every module of the program:
    # their definitions are the ones the imported declarations must agree with
    for group in ('src/core', 'src/editor', 'src/ui', 'src/sys'):
        d = os.path.join(ROOT, group)
        for f in sorted(os.listdir(d)):
            if not f.endswith('.c'):
                continue
            path = os.path.join(d, f)
            text = COMMENT.sub(lambda m: '\n' * m.group().count('\n'), open(path).read())
            for m in PROTO.finditer(text):
                name = m.group(2)
                if name in KEYWORDS or m.group(1).strip() in KEYWORDS | {'typedef'}:
                    continue
                line = text.count('\n', 0, m.start()) + 1
                rec = ('%s/%s:%d' % (group, f, line), params(m.group(3)))
                if m.group(4) == '{':
                    if re.match(r'\s*}', text[m.end():]):
                        continue                # an empty body ignores whatever it is passed
                    defs[name] = rec
                else:
                    decls[name].append(rec)
    return defs, decls


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--all', action='store_true', help='pointer-to-pointer differences too')
    a = ap.parse_args()
    defs, decls = scan()
    bad = 0
    for name in sorted(decls):
        if name not in defs:
            continue
        where, want = defs[name]
        for at, got in decls[name]:
            why = []
            if '...' in want or '...' in got:
                continue
            if len(got) != len(want):
                why.append('%d parameter(s), the definition %d' % (len(got), len(want)))
            else:
                for i, (g, w) in enumerate(zip(got, want), 1):
                    kg, kw = kind(g), kind(w)
                    if kg != kw or (a.all and g != w):
                        why.append('#%d %s, the definition %s' % (i, g, w))
            if why:
                bad += 1
                print('%s: %s(%s): %s  [defined %s]' % (at, name, ', '.join(got), '; '.join(why), where))
    print('%d declaration(s) disagree with their definition' % bad)
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
