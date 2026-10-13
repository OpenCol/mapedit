#!/usr/bin/env python3
"""Where the imported code counts on a record's 1994 size.

In 1994 a far pointer was 4 bytes and a near one 2, so a record with pointers in it had a
size the code could write as a number: `mem_get(size + 0x96)` for a popup, `count * 12 +
0x42` for a sprite series. In the port pointers are 8 bytes, the records grow, and every
such number is wrong. This finds them.

It computes each record's 1994 layout from mapedit-decomp's own declarations (where far and
near are still written; MSC 6 medium model: int 2 bytes, long 4, near pointer 2, far 4,
/Zp2 packing), keeps the records that contain a pointer (only those change size), and lists
every integer literal in src/ equal to one of their sizes where a size is used as one (an
allocation, byte-pointer arithmetic, a multiplication), in a file that uses the record. Each hit is a candidate for a
PORT patch (sizeof / offsetof); a literal that means something else is marked in
tools/layout_audit.ok with why.

    python3 tools/layout_audit.py [--decomp PATH]
"""
import argparse, collections, glob, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TYPEDEF = re.compile(r'typedef\s+struct(?:\s+\w+)?\s*\{(?P<body>(?:[^{}]|\{[^{}]*\})*)\}\s*(?P<name>\w+)\s*;', re.S)
SCALAR = {'char': 1, 'unsigned char': 1, 'signed char': 1, 'int': 2, 'unsigned': 2,
          'unsigned int': 2, 'short': 2, 'unsigned short': 2, 'long': 4, 'unsigned long': 4,
          'FILE': None}


def strip_comments(t):
    """Comments blanked, their line breaks kept (so line numbers stay right)."""
    return re.sub(r'/\*.*?\*/|//[^\n]*', lambda m: re.sub(r'[^\n]', ' ', m.group()), t, flags=re.S)


def parse_records(text, known):
    """{name: [(field, offset, size, is_pointer)], ...} for typedefs whose members we can size."""
    out = {}
    for m in TYPEDEF.finditer(strip_comments(text)):
        name, body = m.group('name'), m.group('body')
        fields, off, ok, has_ptr = [], 0, True, False
        for decl in [d.strip() for d in body.split(';') if d.strip()]:
            if '{' in decl or '(' in decl:
                ok = False
                break
            mm = re.match(r'(.*?)(\**)\s*(\w+)\s*((?:\[[^\]]*\])*)$', decl.replace('\n', ' '))
            if not mm:
                ok = False
                break
            base, stars, fname, dims = mm.group(1).strip(), mm.group(2), mm.group(3), mm.group(4)
            far = 'far' in base.split() or 'far' in decl
            base = ' '.join(w for w in base.split() if w not in ('far', 'near', 'huge', 'const', 'volatile'))
            stars = stars or ('*' if decl.split(fname)[0].strip().endswith('*') else '')
            if '*' in decl.split(fname)[0]:
                size, align, ptr = (4 if far else 2), 2, True
            elif base in SCALAR and SCALAR[base]:
                size, align, ptr = SCALAR[base], min(SCALAR[base], 2), False
            elif base in known:
                size, align, ptr = known[base][0], 2, known[base][1]
            else:
                ok = False
                break
            n = 1
            for d in re.findall(r'\[([^\]]*)\]', dims):
                try:
                    n *= int(d, 0) if d else 1
                except ValueError:
                    ok = False
            if align == 2 and off % 2:
                off += 1
            fields.append((fname, off, size * n, ptr))
            has_ptr |= ptr
            off += size * n
        if ok and fields:
            size = off + (off % 2 if max(f[2] for f in fields) > 1 else 0)
            out[name] = (fields, size, has_ptr)
            known[name] = (size, has_ptr)
    return out


SIZE_USE = re.compile(r'\b(heap_get|heap_create|heap_declare|mem_get|mem_get_name|mem_adjust|msc__fmemcpy|'
                      r'msc__fmemmove|msc__fmemset|loader_read)\s*\(|\(\s*(?:char|u8)\s*\*\s*\)|\*\s*(?:0x)?\d|'
                      r'(?:0x)?\d+\s*\*')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--decomp', default=os.path.join(ROOT, '..', 'mapedit-decomp'))
    a = ap.parse_args()
    known, records = {}, {}
    for f in sorted(glob.glob(os.path.join(a.decomp, 'src', '*', '*', '*.c'))):
        for name, rec in parse_records(open(f).read(), known).items():
            records.setdefault(name, rec)
    ptr_records = {n: r for n, r in records.items() if r[2]}
    ok = set()
    okf = os.path.join(ROOT, 'tools', 'layout_audit.ok')
    if os.path.exists(okf):
        ok = {l.split('#')[0].strip() for l in open(okf) if l.split('#')[0].strip()}
    hits = collections.defaultdict(list)
    for f in sorted(glob.glob(os.path.join(ROOT, 'src', '*', '*.c'))):
        text = strip_comments(open(f).read())
        # split into functions roughly: a hit counts when the record's name is used nearby
        for name, (fields, size, _) in ptr_records.items():
            if not re.search(r'\b%s\b' % name, text):
                continue
            values = {size: 'sizeof(%s) in 1994' % name}
            for k, line in enumerate(text.split('\n'), 1):
                # where a size is used as one: an allocation, pointer arithmetic on a byte
                # pointer, a multiplication
                if not SIZE_USE.search(line):
                    continue
                for lit in re.findall(r'\b(0x[0-9a-fA-F]+|\d+)L?\b', line):
                    v = int(lit, 0)
                    if v in values and v > 4:
                        key = '%s:%d:%s' % (os.path.relpath(f, ROOT), k, lit)
                        if key not in ok:
                            hits[name].append('%s  %s = %s   | %s' % (key, lit, values[v], line.strip()[:90]))
    print('%d records with pointers (1994 sizes):' % len(ptr_records))
    for n, (fields, size, _) in sorted(ptr_records.items()):
        print('  %-14s %3d bytes' % (n, size))
    total = 0
    for n in sorted(hits):
        print('\n%s:' % n)
        for h in hits[n]:
            print('  ' + h)
            total += 1
    print('\n%d candidate literal(s)' % total)


if __name__ == '__main__':
    main()
