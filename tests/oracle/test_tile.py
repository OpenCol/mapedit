"""The terrain tile drawers (src/ui/tile.c) against the original's machine code."""
import ctypes, struct

from emu import W, D
from test_buffer import DosPort, Buffers

N = 300
# terrain numbers tile_id maps into a 16-tile sheet: 0-7 stay, 9 and 0x11 are 8, 15-30 are 0-15
TERRAINS = list(range(8)) + [9, 0x11] + list(range(15, 31))


def sheet(m, rng):
    data = bytes(rng.randrange(256) for _ in range(16 * 256))
    seg = m.alloc(len(data))
    m.write(seg, 0, data)
    return seg, ctypes.create_string_buffer(data, len(data))


def run(m, lib, rng, name, scaled):
    f = getattr(lib, name)
    f.argtypes = [ctypes.c_char_p, ctypes.c_int16, ctypes.POINTER(DosPort), ctypes.c_int16,
                  ctypes.c_int16] + ([ctypes.c_int16] if scaled else [])
    port_at = 0xfff0 - 16                      # the DosPort, near, in DGROUP's free tail
    for case in range(N):
        m.reset_heap()
        w, h = rng.choice([(320, 200), (40, 30)])
        b = Buffers(m, w, h, rng)
        if 'mask' in name:                     # half the destination already drawn
            blank = bytes(0 if rng.random() < 0.5 else c for c in b.data)
            b.data = blank
            m.write(b.seg, 0, blank)
            ctypes.memmove(b.c, blank, len(blank))
        tseg, tiles = sheet(m, rng)
        t = rng.choice(TERRAINS)
        if scaled:
            shift = rng.randrange(0, 5)
            n = 16 >> shift
            x = rng.randrange(n >> 1, w - n + (n >> 1) + 1)
            y = rng.randrange(n - 1, h)
            extra = [shift]
        else:
            x, y, extra = rng.randrange(0, w - 15), rng.randrange(0, h - 15), []
        m.write_ds(port_at, b.port_bytes())
        m.call(name, [D(tseg << 16), W(t), W(port_at), W(x), W(y)] + [W(v) for v in extra])
        p = b.port_c()
        f(tiles, t, ctypes.byref(p), x, y, *extra)
        b.compare('case %d: %s tile %d at (%d,%d)%s in %dx%d' % (
            case, name, t, x, y, ' shift %d' % extra[0] if extra else '', w, h))


def test_tile_draw(m, lib, rng):
    run(m, lib, rng, 'tile_draw', False)


def test_tile_draw_mask(m, lib, rng):
    run(m, lib, rng, 'tile_draw_mask', False)


def test_tile_draw_scale(m, lib, rng):
    run(m, lib, rng, 'tile_draw_scale', True)


def test_tile_draw_scale_mask(m, lib, rng):
    run(m, lib, rng, 'tile_draw_scale_mask', True)
