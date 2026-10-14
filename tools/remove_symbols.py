#!/usr/bin/env python3
"""Remove functions and globals by name: their definitions (with the comments directly above
them) and their declarations in src/include.

    python3 tools/remove_symbols.py NAME... | --from FILE

For dead code: tools/dead_code.py lists what the link leaves unreferenced. A definition that
declares several names at once (`i16 a, b;`) loses only the named ones. Prints what it
removed; a name it cannot find is reported, not guessed at.
"""
import argparse, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = ['src/core', 'src/editor', 'src/ui', 'src/sys']
INC = 'src/include'


def mask(text):
    """comments and literals blanked, same length, so offsets match the text"""
    out = list(text)
    i, n = 0, len(text)
    while i < n:
        if text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                if out[k] != '\n':
                    out[k] = ' '
            i = j
        elif text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
            for k in range(i, j):
                out[k] = ' '
            i = j
        elif text[i] in '"\'':
            q, j = text[i], i + 1
            while j < n and text[j] != q:
                j += 2 if text[j] == '\\' else 1
            for k in range(i + 1, min(j, n)):
                if out[k] != '\n':
                    out[k] = '_'
            i = j + 1
        else:
            i += 1
    return ''.join(out)


def top_statements(text):
    """(kind, start, end) of top-level statements: 'func' definitions and 'decl' statements"""
    m = mask(text)
    out, i, n, start, depth = [], 0, len(m), None, 0
    while i < n:
        c = m[i]
        if start is None:
            if c.isspace():
                i += 1
                continue
            if c == '#':
                j = m.find('\n', i)
                i = n if j < 0 else j
                continue
            start = i
        if c == '{':
            if depth == 0:
                head = m[start:i].strip()
                if head.endswith(')') and not head.startswith('typedef') and '=' not in head.split('(')[0]:
                    d, j = 0, i
                    while j < n:
                        d += m[j] == '{'
                        d -= m[j] == '}'
                        if m[j] == '}' and d == 0:
                            break
                        j += 1
                    out.append(('func', start, j + 1))
                    start, i = None, j + 1
                    continue
            depth += 1
        elif c == '}':
            depth -= 1
        elif c == ';' and depth == 0:
            out.append(('decl', start, i + 1))
            start = None
        i += 1
    return out, m


def declared_names(stmt):
    """the names a top-level statement defines or declares, and whether it is a function"""
    s = re.sub(r'\s+', ' ', stmt).strip()
    if s.startswith('typedef'):
        return [], False
    s = re.sub(r'^(extern|static) ', '', s)
    head = s.split('(')[0] if '(' in s else s
    if '(' in s and '=' not in head:
        m = re.search(r'(\w+)\s*$', head)
        if m and not head.rstrip().endswith('*') or (m and re.search(r'\w\s*$', head)):
            if re.search(r'\)\s*;?$', s):              # a function: NAME( ... )
                return [m.group(1)], True
        fp = re.match(r'^[^(]*\(\s*\*\s*(\w+)\s*\)\s*\(', s)
        if fp:                                          # a function pointer: T (*NAME)(...)
            return [fp.group(1)], False
    body = s.rstrip(';')
    parts, depth, cur = [], 0, ''
    for ch in body:
        depth += ch in '({['
        depth -= ch in ')}]'
        if ch == ',' and depth == 0:
            parts.append(cur)
            cur = ''
        else:
            cur += ch
    parts.append(cur)
    names = []
    for k, p in enumerate(parts):
        p = p.split('=')[0]
        m = re.search(r'(\w+)\s*(\[[^\]]*\]\s*)*$', p.strip())
        if m:
            names.append(m.group(1))
    return names, False


def widen(text, start, end):
    """the statement plus its trailing comment and newline, and the comment block above it"""
    e = end
    while e < len(text) and text[e] in ' \t':
        e += 1
    if text.startswith('/*', e):
        k = text.find('*/', e)
        if k >= 0 and '\n' not in text[e:k]:
            end = k + 2
    elif text.startswith('//', e):
        k = text.find('\n', e)
        end = len(text) if k < 0 else k
    if end < len(text) and text[end] == '\n':
        end += 1
    s = text.rfind('\n', 0, start) + 1
    while s > 0:
        prev = s - 1
        p = text.rfind('\n', 0, prev) + 1
        line = text[p:prev].strip()
        if not line:
            break
        if line.startswith('//'):
            s = p
            continue
        if line.endswith('*/'):
            b = text.rfind('/*', 0, prev)
            lb = text.rfind('\n', 0, b) + 1
            if b >= 0 and not text[lb:b].strip():
                s = lb
                continue
        break
    return s, end


def remove_names_from(text, wanted, is_header):
    """text without the definitions/declarations of `wanted`; the names it removed"""
    sts, m = top_statements(text)
    cuts, done, edits = [], set(), []
    for kind, a, b in sts:
        names, _ = declared_names(m[a:b] if kind == 'decl' else m[a:b].split('{')[0])
        hit = [x for x in names if x in wanted]
        if not hit:
            continue
        if len(names) == len(hit):
            cuts.append(widen(text, a, b))
        else:
            edits.append((a, b, hit))          # several names on one line: drop only these
        done.update(hit)
    # every change from the end backwards, so no offset moves under a later one
    changes = [(a, b, None) for a, b in cuts] + [(a, b, hit) for a, b, hit in edits]
    for a, b, hit in sorted(changes, key=lambda c: c[0], reverse=True):
        if hit is None:
            text = text[:a] + text[b:]
            continue
        stmt = text[a:b]
        for x in hit:
            stmt = re.sub(r',\s*\**\s*%s\s*(\[[^\]]*\]\s*)*(=\s*[^,;]+)?(?=[,;])' % re.escape(x), '', stmt, count=1) \
                if re.search(r',\s*\**\s*%s\b' % re.escape(x), stmt) else \
                re.sub(r'\b%s\s*(\[[^\]]*\]\s*)*(=\s*[^,;]+)?,\s*' % re.escape(x), '', stmt, count=1)
        text = text[:a] + stmt + text[b:]
    return re.sub(r'\n{3,}', '\n\n', text), done


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('names', nargs='*')
    ap.add_argument('--from', dest='src')
    a = ap.parse_args()
    wanted = set(a.names)
    if a.src:
        wanted |= {l.split()[0] for l in open(a.src) if l.strip() and not l.startswith('#')}
    found = set()
    for d in SRC + [INC]:
        for f in sorted(os.listdir(os.path.join(ROOT, d))):
            if not f.endswith(('.c', '.h')):
                continue
            p = os.path.join(ROOT, d, f)
            t = open(p).read()
            new, done = remove_names_from(t, wanted, d == INC)
            if done:
                open(p, 'w').write(new)
                found |= done
                print('%-28s %s' % (os.path.join(d, f), ' '.join(sorted(done))))
    missing = wanted - found
    if missing:
        print('not found:', ' '.join(sorted(missing)))
    return 1 if missing else 0


if __name__ == '__main__':
    sys.exit(main())
