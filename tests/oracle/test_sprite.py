"""The run-length sprite drawer (port/lib/sprite.c) against the original's machine code."""
import ctypes, struct

from emu import W, P, D
from test_buffer import DosPort, Buffers

N = 300


class IconRec(ctypes.Structure):
    _fields_ = [('data', ctypes.c_void_p), ('x0', ctypes.c_int16), ('y0', ctypes.c_int16),
                ('w', ctypes.c_int16), ('h', ctypes.c_int16)]


def series_type(n):
    class Series(ctypes.Structure):
        _fields_ = [('head', ctypes.c_int16 * 22), ('packed', ctypes.c_char), ('pad', ctypes.c_char),
                    ('ptrs', ctypes.c_void_p * 5), ('icon', IconRec * n)]
    return Series


def rle_row(rng, w):
    kind = rng.random()
    if kind < 0.1:
        return bytes([0xff])
    out = bytearray()
    cols = 0
    target = rng.randrange(0, w + 8)
    if kind < 0.5:
        out.append(0xfd)
        while cols < target:
            n = rng.randrange(1, 12)
            out += bytes([n, rng.choice([0xfd, rng.randrange(0, 0xfd)])])
            cols += n
    else:
        out.append(0xfc)
        while cols < target:
            if rng.random() < 0.3:
                n = rng.randrange(1, 12)
                out += bytes([0xfe, n, rng.choice([0xfd, rng.randrange(0, 0xfd)])])
                cols += n
            else:
                out.append(rng.choice([0xfd, rng.randrange(0, 0xfd)]))
                cols += 1
    out.append(0xff)
    return bytes(out)


def run(m, lib, rng, name):
    f = getattr(lib, name)
    scaled = 'scaled' in name
    f.argtypes = [ctypes.c_int16, ctypes.c_void_p, ctypes.POINTER(DosPort), ctypes.c_int16,
                  ctypes.c_int16] + ([ctypes.c_int16] if scaled else [])
    port_at = 0xfff0 - 16
    for case in range(N):
        m.reset_heap()
        w, h = rng.choice([(320, 200), (40, 30)])
        b = Buffers(m, w, h, rng)
        if name.endswith('black'):             # half the destination still empty
            blank = bytes(0 if rng.random() < 0.5 else c for c in b.data)
            b.data = blank
            m.write(b.seg, 0, blank)
            ctypes.memmove(b.c, blank, len(blank))
        nframes = rng.randrange(1, 4)
        frames = []
        for k in range(nframes):
            fw, fh = rng.randrange(1, 40), rng.randrange(1, 30)
            frames.append((fw, fh, b''.join(rle_row(rng, fw) for _ in range(fh))))
        # the original's sheet: 36h bytes, then 12-byte records from icon 0 (the series' -1)
        sheet = bytearray(0x36 + 12 * (nframes + 1))
        Series = series_type(nframes)
        cs = Series()
        keep = []
        for k, (fw, fh, data) in enumerate(frames):
            seg = m.alloc(len(data))
            m.write(seg, 0, data)
            struct.pack_into('<HHhhhh', sheet, 0x36 + 12 * (k + 1), 0, seg, 0, 0, fw, fh)
            cbuf = ctypes.create_string_buffer(data, len(data))
            keep.append(cbuf)
            cs.icon[k] = IconRec(ctypes.cast(cbuf, ctypes.c_void_p).value, 0, 0, fw, fh)
        sseg = m.alloc(len(sheet))
        m.write(sseg, 0, bytes(sheet))
        n = rng.randrange(1, nframes + 1)
        icon = n - 0x8000 if rng.random() < 0.4 else n    # bit 15: mirrored
        fw, fh, _ = frames[n - 1]
        x, y = rng.randrange(-fw - 2, w + 2), rng.randrange(-fh - 2, h + 2)
        m.write_ds(port_at, b.port_bytes())
        extra = [rng.choice([100, rng.randrange(1, 100)])] if scaled else []
        m.call(name, [W(icon), D(sseg << 16), P(port_at), W(x), W(y)] + [W(v) for v in extra])
        p = b.port_c()
        f(icon, ctypes.addressof(cs), ctypes.byref(p), x, y, *extra)
        b.compare('case %d: frame %d (%dx%d) at (%d,%d) in %dx%d' % (case, icon, fw, fh, x, y, w, h))


def test_sprite_draw(m, lib, rng):
    run(m, lib, rng, 'sprite_draw')


def test_sprite_draw_black(m, lib, rng):
    run(m, lib, rng, 'sprite_draw_black')


def test_sprite_draw_scaled(m, lib, rng):
    run(m, lib, rng, 'sprite_draw_scaled')


def test_sprite_draw_scaled_black(m, lib, rng):
    run(m, lib, rng, 'sprite_draw_scaled_black')
