#!/usr/bin/env python3
"""Import the editor's source from mapedit-decomp at the commit in import/SOURCE.

    python3 tools/import.py                      # write src/ and import/
    python3 tools/import.py --source <commit>    # move to another commit (and record it)
    python3 tools/import.py --check              # change nothing; compare with a fresh import

mapedit-decomp is found at --decomp PATH, $MAPEDIT_DECOMP, or ../mapedit-decomp. Its working
tree is never read: the pinned commit is unpacked with `git archive`.

What an import writes (everything else in the tree is this repository's own):
    src/<editor|madsdev>/<module>.c   each module, its function files expanded, converted
    src/include/<name>.h              the shared declarations, converted
    import/SOURCE                     the commit imported
    import/REFUSED                    the functions not imported, and why (port/ has them)
    import/runtime.txt                the MSC run-time names the source uses (msc_ in the port)

The conversion is mechanical and token-level (comments, strings and character literals are
left alone):
- MSC's memory-model and calling-convention keywords go: far, near, huge, _fastcall,
  pascal, cdecl.
- Integer types keep their 1994 widths: int and short are i16, unsigned is u16, long is
  i32, unsigned long u32, unsigned char u8 (port/msc.h). char stays char, compiled signed.
- Every name the MSC run-time library defines (the publics of MAPEDIT.EXE's run-time
  segment, link/symbols.tsv) is renamed msc_<name>, and FILE is MSC_FILE, so the
  source's own declarations never meet the modern C library's; port/ implements them with
  MSC 6's semantics. #include <x.h> becomes "msc.h".
- MSC's code-generation pragmas (#pragma optimize) become comments; each module packs its
  structures on 2 bytes, MSC 6's default.
- main is editor_main; the port's main calls it.
- The source's own declarations of the C library (its copy of MSC's FILE record, struct
  find_t, prototypes of run-time functions) are dropped: port/msc.h declares them once.
  So are the modules' own definitions of records the port defines once (port/port_records.h:
  DosPort, whose shapes differed between modules but whose size did not).
- A function with inline assembly (_asm) is not imported: its definition becomes its
  prototype, and import/REFUSED lists it. port/ holds the C written for it.
- A far pointer into the BIOS data area written as a constant ((u8 *)0x00400017L, the shift
  flags) points into msc_bda, the port's copy of that area (port/lib/bios.c).
- Last, patches/<file>.patch: the port's deliberate edits, each marked PORT:, as diffs
  against the plain import (tools/import.py --save-patches writes them from src/).
"""
import argparse, os, re, shutil, subprocess, sys, tarfile, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

DROP = {'far', '_far', '__far', 'near', '_near', '__near', 'huge', '_huge', '__huge',
        '_fastcall', '__fastcall', 'pascal', '_pascal', '__pascal', 'cdecl', '_cdecl', '__cdecl',
        '_loadds', '_saveregs', '_export'}
# integer types, longest first: (words, replacement)
TYPES = [
    (('unsigned', 'long', 'int'), 'u32'), (('unsigned', 'short', 'int'), 'u16'),
    (('signed', 'long', 'int'), 'i32'), (('signed', 'short', 'int'), 'i16'),
    (('long', 'unsigned', 'int'), 'u32'), (('short', 'unsigned', 'int'), 'u16'),
    (('unsigned', 'long'), 'u32'), (('unsigned', 'short'), 'u16'), (('unsigned', 'int'), 'u16'),
    (('unsigned', 'char'), 'u8'), (('signed', 'char'), 'i8'),
    (('long', 'unsigned'), 'u32'), (('short', 'unsigned'), 'u16'),
    (('signed', 'long'), 'i32'), (('signed', 'short'), 'i16'), (('signed', 'int'), 'i16'),
    (('long', 'int'), 'i32'), (('short', 'int'), 'i16'),
    (('unsigned',), 'u16'), (('signed',), 'i16'), (('long',), 'i32'), (('short',), 'i16'),
    (('int',), 'i16'),
]
TOKEN = re.compile(r'''
    (?P<comment>/\*.*?\*/|//[^\n]*)
  | (?P<string>"(?:\\.|[^"\\\n])*"|'(?:\\.|[^'\\\n])*')
  | (?P<pp>^[ \t]*\#[^\n]*(?:\\\n[^\n]*)*)
  | (?P<word>[A-Za-z_]\w*)
  | (?P<space>\s+)
  | (?P<other>.)
''', re.S | re.M | re.X)


def decomp_path(argv_path):
    return os.path.abspath(argv_path or os.environ.get('MAPEDIT_DECOMP')
                           or os.path.join(ROOT, '..', 'mapedit-decomp'))


def unpack(decomp, commit, tmp):
    if subprocess.run(['git', '-C', decomp, 'cat-file', '-e', commit + '^{commit}'],
                      capture_output=True).returncode:
        sys.exit('mapedit-decomp at %s does not have commit %s' % (decomp, commit))
    arc = os.path.join(tmp, 'src.tar')
    with open(arc, 'wb') as f:
        subprocess.run(['git', '-C', decomp, 'archive', commit, '--', 'src', 'include', 'link'],
                       stdout=f, check=True)
    tree = os.path.join(tmp, 'tree')
    with tarfile.open(arc) as t:
        t.extractall(tree)
    return tree


def tsv(tree, name):
    lines = open(os.path.join(tree, 'link', name)).read().rstrip('\n').split('\n')
    head = lines[0].split('\t')
    return [dict(zip(head, l.split('\t'))) for l in lines[1:]]


def runtime_names(tree):
    """C names of the MSC run-time's code publics: everything from crt0's frame up to DGROUP,
    as link/symbols.tsv lists them (`_strcpy` is strcpy, `__fstrcpy` is _fstrcpy). Compiler
    helpers (__aFlmul) never appear in source, so including them costs nothing."""
    import json
    t = json.load(open(os.path.join(tree, 'link', 'target.json')))
    crt = 0x1388                     # crt0's frame in MAPEDIT.EXE (MLIBCE's _TEXT)
    dgroup = int(t['mz']['dgroup'], 16)
    names = set()
    for s in tsv(tree, 'symbols.tsv'):
        seg = int(s['address'][:4], 16)
        if s['name'].startswith('_') and crt <= seg < dgroup:
            names.add(s['name'][1:])
    names.discard('main')
    return names


def scan_defs(text):
    """[(name, start, open brace, close brace)] for every top-level function definition."""
    out, depth, i, n = [], 0, 0, len(text)
    last_sig, term, popen = '', 0, None
    pdepth = 0
    while i < n:
        m = TOKEN.match(text, i)
        kind, tok = m.lastgroup, m.group()
        j = m.end()
        if kind in ('comment', 'space'):
            i = j
            continue
        if kind == 'pp':
            if depth == 0:
                term = j
            i = j
            continue
        if depth == 0 and kind == 'other':
            if tok == '(':
                if pdepth == 0:
                    popen = i
                pdepth += 1
            elif tok == ')':
                pdepth -= 1
            elif tok == ';':
                term = j
        if tok == '{':
            if depth == 0 and last_sig == ')':
                name = re.search(r'([A-Za-z_]\w*)\s*$', text[:popen])
                out.append([name.group(1) if name else None, term, i, None])
            depth += 1
        elif tok == '}':
            depth -= 1
            if depth == 0:
                if out and out[-1][3] is None:
                    out[-1][3] = i
                term = j
        last_sig = tok if kind != 'string' else '"'
        i = j
    return [tuple(d) for d in out]


def has_asm(body):
    return any(m.lastgroup == 'word' and m.group() in ('_asm', '__asm')
               for m in TOKEN.finditer(body))


# Declarations of the C library the source writes for itself: port/msc.h owns them.
OWN_DECLS = [
    re.compile(r'^[ \t]*typedef struct (?:_iobuf )?\{[^{}]*\} MSC_FILE;[ \t]*\n', re.M),
    re.compile(r'^[ \t]*struct find_t \{[^{}]*\};[ \t]*\n', re.M),
    re.compile(r'^[ \t]*typedef[ \t]+i32[ \t]+fpos_t;[^\n]*\n', re.M),
    # records with one definition in the port (port/port_records.h)
    re.compile(r'^[ \t]*typedef struct \{[^{}]*\} (?:DosPort);[^\n]*\n', re.M),
    # a prototype: a type first (never `return f(x);`), a msc_ declarator, a parameter list
    re.compile(r'^[ \t]*(?:extern[ \t]+)?(?:const[ \t]+)?(?:i8|u8|i16|u16|i32|u32|char|void|MSC_FILE|'
               r'struct[ \t]+\w+)[ \t\*]+(?:const[ \t\*]+)?msc_\w+[ \t]*\([^;{}()]*\)[ \t]*;[ \t]*'
               r'(?:/\*[^\n]*\*/)?[ \t]*\n', re.M),
]


def drop_own_decls(text):
    for rx in OWN_DECLS:
        text = rx.sub('', text)
    return text


# A far pointer into the BIOS data area, written as a constant (0x00400017L: 0040:0017, the
# shift flags): the port keeps that area in msc_bda (port/lib/bios.c).
BDA = re.compile(r'\(\s*([A-Za-z_][\w \t]*\*)\s*\)\s*0x0040([0-9a-fA-F]{4})L')


def bios_area(text):
    return BDA.sub(lambda m: '(%s)(msc_bda + 0x%s)' % (m.group(1).strip(), m.group(2).lower()), text)


def convert(text, runtime):
    """The token-level conversion (see the module docstring)."""
    toks = [(m.lastgroup, m.group()) for m in TOKEN.finditer(text)]
    out, i = [], 0
    words = [k for k in range(len(toks)) if toks[k][0] == 'word']
    pos = {k: n for n, k in enumerate(words)}

    def next_words(k, count):
        """The next `count` word tokens from k, skipping only whitespace and comments."""
        got, idx = [], k
        while len(got) < count and idx < len(toks):
            kind, tok = toks[idx]
            if kind == 'word':
                got.append((idx, tok))
            elif kind not in ('space', 'comment'):
                break
            idx += 1
        return got
    while i < len(toks):
        kind, tok = toks[i]
        if kind == 'pp':
            if re.match(r'\s*#\s*include\s*<', tok):
                tok = '#include "msc.h"'
            elif re.match(r'\s*#\s*pragma\s+(optimize|check_stack|intrinsic|function|loop_opt)\b', tok):
                tok = '// MSC: %s' % tok.strip()         # MSC 6 code generation, no meaning here
            out.append(tok)
            i += 1
            continue
        if kind != 'word':
            out.append(tok)
            i += 1
            continue
        if tok in DROP:
            # drop the keyword and one following space
            i += 1
            if i < len(toks) and toks[i][0] == 'space' and '\n' not in toks[i][1]:
                i += 1
            continue
        done = False
        for seq, rep in TYPES:
            if tok != seq[0]:
                continue
            nw = next_words(i, len(seq))
            if [w for _, w in nw] == list(seq):
                out.append(rep)
                i = nw[-1][0] + 1
                done = True
                break
        if done:
            continue
        if tok == 'FILE':
            out.append('MSC_FILE')
        elif tok == 'size_t':
            out.append('u16')
        elif tok == 'main':
            out.append('editor_main')
        elif tok in runtime:
            out.append('msc_' + tok)
        else:
            out.append(tok)
        i += 1
    return ''.join(out)


INC = re.compile(r'^#include "([^"]+\.c)"\s*$', re.M)


def import_module(tree, group, mod, runtime, refused):
    d = os.path.join(tree, 'src', group, mod)
    parts = []
    for line in open(os.path.join(d, 'module.c')).read().split('\n'):
        m = INC.match(line)
        if not m:
            parts.append(line + '\n')
            continue
        fn_file = m.group(1)
        body = open(os.path.join(d, fn_file)).read()
        defs = scan_defs(body)
        if defs and has_asm(body[defs[-1][2]:defs[-1][3] + 1]):
            name, term, lb, rb = defs[-1]
            sig = re.compile(r'(?:\s+|/\*.*?\*/|//[^\n]*)*', re.S).match(body, term).end()
            proto = re.sub(r'\s+', ' ', body[sig:lb]).strip() + ';'
            refused.append((group, mod, name, 'inline assembly'))
            parts.append(body[:sig] + '/* PORT: %s is in port/ (inline assembly) */\n%s\n'
                         % (name, proto) + body[rb + 1:])
        else:
            parts.append(body if body.endswith('\n') else body + '\n')
    text = ''.join(parts)
    return ('/* Imported from mapedit-decomp src/%s/%s by tools/import.py. Do not edit; mark a\n'
            ' * deliberate change with PORT: (docs/plan.md). */\n#include "msc.h"\n'
            '#pragma pack(2)   /* MSC 6\'s default structure packing (/Zp2) */\n' % (group, mod)
            + bios_area(drop_own_decls(convert(text, runtime))))


def produce(tree):
    runtime = runtime_names(tree)
    refused, files = [], {}
    for r in tsv(tree, 'modules.tsv'):
        g, mod = r['group'], r['module']
        if not os.path.exists(os.path.join(tree, 'src', g, mod, 'module.c')):
            continue
        files[os.path.join('src', g, mod + '.c')] = import_module(tree, g, mod, runtime, refused)
    inc = os.path.join(tree, 'include')
    for h in sorted(os.listdir(inc)) if os.path.isdir(inc) else ():
        if h.endswith('.h'):
            files[os.path.join('src', 'include', h)] = (
                '/* Imported from mapedit-decomp include/%s by tools/import.py. */\n' % h
                + convert(open(os.path.join(inc, h)).read(), runtime))
    used = set()
    for t in files.values():
        used |= set(re.findall(r'\bmsc_(\w+)', t))
    files['import/REFUSED'] = ''.join('%s/%s\t%s\t%s\n' % r for r in refused)
    apply_patches(files)
    files['import/runtime.txt'] = ''.join(n + '\n' for n in sorted(used))
    return files


PATCHES = os.path.join(ROOT, 'patches')


def apply_patches(files):
    """patches/<path>.patch: the port's deliberate edits to an imported file, as a unified
    diff against the plain import (made by --save-patches). Applied in place; one that no
    longer applies stops the import."""
    for dirpath, _, names in os.walk(PATCHES):
        for n in sorted(names):
            if not n.endswith('.patch'):
                continue
            rel = os.path.relpath(os.path.join(dirpath, n), PATCHES)[:-len('.patch')]
            if rel not in files:
                sys.exit('patches/%s.patch: the import has no %s' % (rel, rel))
            tmp = tempfile.mkdtemp()
            try:
                p = os.path.join(tmp, 'f')
                open(p, 'w').write(files[rel])
                r = subprocess.run(['patch', '-s', '-N', p, os.path.join(dirpath, n)],
                                   capture_output=True, text=True)
                if r.returncode:
                    sys.exit('patches/%s.patch no longer applies:\n%s' % (rel, r.stdout + r.stderr))
                files[rel] = open(p).read()
            finally:
                shutil.rmtree(tmp)


def save_patches(fresh):
    """Record every src/ file that differs from the plain import as a patch."""
    n = 0
    for rel, text in sorted(fresh.items()):
        if not rel.startswith('src/'):
            continue
        p = os.path.join(ROOT, rel)
        if not os.path.exists(p):
            continue
        have = open(p).read()
        out = os.path.join(PATCHES, rel + '.patch')
        if have == text:
            if os.path.exists(out):
                os.remove(out)
            continue
        if 'PORT:' not in have:
            sys.exit('%s differs from the import but has no PORT: comment' % rel)
        tmp = tempfile.mkdtemp()
        try:
            a = os.path.join(tmp, 'a')
            open(a, 'w').write(text)
            d = subprocess.run(['diff', '-u', '--label', rel, '--label', rel, a, p],
                               capture_output=True, text=True).stdout
        finally:
            shutil.rmtree(tmp)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        open(out, 'w').write(d)
        n += 1
    print('%d patch(es) in patches/' % n)


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument('--decomp')
    ap.add_argument('--source')
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--save-patches', action='store_true',
                    help='record the PORT: edits in src/ as patches/ (against a plain import)')
    a = ap.parse_args(argv)
    decomp = decomp_path(a.decomp)
    src_file = os.path.join(ROOT, 'import', 'SOURCE')
    commit = a.source or (open(src_file).read().strip() if os.path.exists(src_file) else None)
    if not commit:
        commit = subprocess.run(['git', '-C', decomp, 'rev-parse', 'HEAD'], capture_output=True,
                                text=True, check=True).stdout.strip()
    tmp = tempfile.mkdtemp()
    try:
        if a.save_patches:
            global apply_patches
            apply_patches = lambda files: None      # compare with the plain import
        files = produce(unpack(decomp, commit, tmp))
    finally:
        shutil.rmtree(tmp)
    if a.save_patches:
        save_patches(files)
        return 0
    files['import/SOURCE'] = commit + '\n'
    if a.check:
        bad = 0
        for rel, text in sorted(files.items()):
            p = os.path.join(ROOT, rel)
            have = open(p).read() if os.path.exists(p) else None
            if have != text and 'PORT:' not in (have or '').replace('/* PORT: ', ''):
                print('differs' if have is not None else 'missing', rel)
                bad += 1
        print('%d file(s) differ from a fresh import of %s' % (bad, commit[:12]))
        return 1 if bad else 0
    for d in ('src/editor', 'src/madsdev', 'src/include'):
        shutil.rmtree(os.path.join(ROOT, d), ignore_errors=True)
    for rel, text in files.items():
        p = os.path.join(ROOT, rel)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        open(p, 'w').write(text)
    n = sum(1 for k in files if k.startswith('src/') and k.endswith('.c'))
    print('imported %d modules from %s; %d function(s) refused (import/REFUSED)'
          % (n, commit[:12], files['import/REFUSED'].count('\n')))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
