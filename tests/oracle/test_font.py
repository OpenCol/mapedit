"""font_write (src/ui/font.c) against the original's machine code."""
import ctypes, struct

from emu import W, P, D
from test_buffer import DosPort, Buffers

N = 300


def make_font(rng):
    h = rng.randrange(1, 12)
    widths = [rng.choice([0, rng.randrange(1, 12)]) for _ in range(128)]
    glyphs = bytearray()
    offs = []
    base = 0x82 + 256
    for w in widths:
        offs.append(base + len(glyphs))
        glyphs += bytes(rng.randrange(256) for _ in range(h * ((max(w, 1) - 1) // 4 + 1)))
    font = bytearray(base)
    font[0] = h
    font[2:2 + 128] = bytes(widths)
    for i, o in enumerate(offs):
        struct.pack_into('<H', font, 0x82 + 2 * i, o)
    return bytes(font + glyphs)


def test_font_write(m, lib, rng):
    f = lib.font_write
    f.argtypes = [ctypes.POINTER(DosPort), ctypes.c_int16, ctypes.c_int16, ctypes.c_char_p,
                  ctypes.c_char_p, ctypes.c_int16]
    f.restype = ctypes.c_int16
    cols_c = (ctypes.c_uint8 * 4).in_dll(lib, 'font_colors')
    port_at = 0xfff0 - 16
    for case in range(N):
        m.reset_heap()
        w, h = rng.choice([(320, 200), (60, 20)])
        b = Buffers(m, w, h, rng)
        font = make_font(rng)
        fseg = m.alloc(len(font))
        m.write(fseg, 0, font)
        text = bytes(rng.choice([rng.randrange(1, 0x80), rng.randrange(0x80, 0x100)])
                     if rng.random() < 0.05 else rng.randrange(1, 0x80)
                     for _ in range(rng.randrange(0, 30))) + b'\0'
        sseg = m.alloc(len(text))
        m.write(sseg, 0, text)
        cols = bytes(rng.choice([0xff, rng.randrange(256)]) for _ in range(4))
        m.write_ds(m.data('font_colors'), cols)
        cols_c[:] = list(cols)
        x, y, gap = rng.randrange(-5, w), rng.randrange(-12, h + 2), rng.randrange(0, 3)
        m.write_ds(port_at, b.port_bytes())
        ax, _ = m.call('font_write', [P(port_at), W(x), W(y), D(fseg << 16), D(sseg << 16), W(gap)])
        p = b.port_c()
        r = f(ctypes.byref(p), x, y, font, text, gap)
        what = 'case %d: %r at (%d,%d) gap %d in %dx%d' % (case, text[:10], x, y, gap, w, h)
        b.compare(what)
        assert (r & 0xffff) == ax, '%s: returned %d, the original %d' % (what, r, ax)


def test_font_string_width(m, lib, rng):
    f = lib.font_string_width
    f.argtypes = [ctypes.c_int16, ctypes.c_char_p, ctypes.c_char_p]
    f.restype = ctypes.c_int16
    for case in range(N):
        m.reset_heap()
        font = make_font(rng)
        fseg = m.alloc(len(font))
        m.write(fseg, 0, font)
        text = bytes(rng.randrange(1, 0x80) for _ in range(rng.randrange(0, 30))) + b'\0'
        sseg = m.alloc(len(text))
        m.write(sseg, 0, text)
        sp = rng.randrange(-2, 4)
        ax, _ = m.call('font_string_width', [W(sp), D(fseg << 16), D(sseg << 16)])
        r = f(sp, font, text) & 0xffff
        assert r == ax, 'case %d: %r spacing %d: %d, the original %d' % (case, text, sp, r, ax)
