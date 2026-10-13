"""The work-buffer primitives (src/sys/buffer.c) against the original's machine code."""
import ctypes, struct

from emu import W, P, Block

N = 300             # cases per function


class DosPort(ctypes.Structure):
    _fields_ = [('h', ctypes.c_int16), ('w', ctypes.c_int16), ('bits', ctypes.c_void_p)]


class Buffers:
    """The same buffer on both sides: a block in the emulator and a ctypes array."""

    def __init__(self, m, w, h, rng):
        self.m, self.w, self.h = m, w, h
        self.data = bytes(rng.randrange(256) for _ in range(w * h))
        self.seg = m.alloc(w * h)
        m.write(self.seg, 0, self.data)
        self.c = ctypes.create_string_buffer(self.data, w * h)

    def port_bytes(self):
        return struct.pack('<hhHH', self.h, self.w, 0, self.seg)

    def port_c(self):
        return DosPort(self.h, self.w, ctypes.cast(self.c, ctypes.c_void_p))

    def compare(self, what):
        ours, theirs = self.c.raw, self.m.read(self.seg, 0, self.w * self.h)
        if ours != theirs:
            k = next(i for i in range(len(ours)) if ours[i] != theirs[i])
            raise AssertionError('%s: pixel (%d, %d) is %d in the port, %d in the original'
                                 % (what, k % self.w, k // self.w, ours[k], theirs[k]))


def sizes(rng):
    return rng.choice([(320, 200), (37, 23), (16, 16), (64, 3)])


def test_buffer_rect_fill(m, lib, rng):
    f = lib.buffer_rect_fill
    f.argtypes = [ctypes.c_int16] * 3 + [DosPort, ctypes.c_int16, ctypes.c_char]
    f.restype = ctypes.c_int16
    for case in range(N):
        m.reset_heap()
        w, h = sizes(rng)
        b = Buffers(m, w, h, rng)
        x, y = rng.randrange(-20, w + 5), rng.randrange(-20, h + 5)
        rw, rh = rng.randrange(0, w + 10), rng.randrange(0, h + 10)
        color = rng.randrange(256)
        ax, _ = m.call('buffer_rect_fill', [W(x), W(y), W(rw), Block(b.port_bytes()), W(rh), W(color)])
        r = f(x, y, rw, b.port_c(), rh, bytes([color]))
        what = 'case %d: fill (%d,%d) %dx%d in %dx%d' % (case, x, y, rw, rh, w, h)
        b.compare(what)
        if r:       # a clipped-away fill returns a stack leftover in the original (PORT)
            assert r == ax, '%s: returned %d, the original %d' % (what, r, ax)


def test_buffer_rect_copy(m, lib, rng):
    f = lib.buffer_rect_copy
    f.argtypes = [ctypes.c_int16] * 3 + [DosPort, DosPort, ctypes.c_int16]
    f.restype = ctypes.c_int16
    g = lib.buffer_rect_copy_2
    g.argtypes = [ctypes.c_int16] * 3 + [DosPort, DosPort] + [ctypes.c_int16] * 3
    g.restype = ctypes.c_int16
    for case in range(N):
        m.reset_heap()
        w, h = sizes(rng)
        src, dst = Buffers(m, w, h, rng), Buffers(m, w, h, rng)
        # no clipping in either routine: keep the rectangle inside
        rw, rh = rng.randrange(0, w + 1), rng.randrange(0, h + 1)
        x, y = rng.randrange(0, w - rw + 1), rng.randrange(0, h - rh + 1)
        if case % 2 == 0:
            ax, _ = m.call('buffer_rect_copy', [W(x), W(y), W(rw), Block(src.port_bytes()),
                                                Block(dst.port_bytes()), W(rh)])
            r = f(x, y, rw, src.port_c(), dst.port_c(), rh)
            what = 'case %d: copy (%d,%d) %dx%d' % (case, x, y, rw, rh)
        else:
            dx, dy = rng.randrange(0, w - rw + 1), rng.randrange(0, h - rh + 1)
            ax, _ = m.call('buffer_rect_copy_2', [W(x), W(y), W(dx), Block(src.port_bytes()),
                                                  Block(dst.port_bytes()), W(dy), W(rw), W(rh)])
            r = g(x, y, dx, src.port_c(), dst.port_c(), dy, rw, rh)
            what = 'case %d: copy (%d,%d) to (%d,%d) %dx%d' % (case, x, y, dx, dy, rw, rh)
        dst.compare(what)
        src.compare(what + ' (source)')
        assert r == ax, '%s: returned %d, the original %d' % (what, r, ax)


def test_buffer_lines(m, lib, rng):
    hl = lib.buffer_hline
    hl.argtypes = [ctypes.c_int16] * 3 + [DosPort, ctypes.c_char]
    vl = lib.buffer_vline
    vl.argtypes = [ctypes.c_int16] * 3 + [DosPort, ctypes.c_char]
    for case in range(N):
        m.reset_heap()
        w, h = sizes(rng)
        b = Buffers(m, w, h, rng)
        color = rng.randrange(256)
        if case % 2 == 0:
            y = rng.randrange(-3, h + 3)
            x0 = rng.randrange(-10, w)
            x1 = rng.randrange(max(x0, 0), w + 10)      # x1 >= x0 (see buffer_hline: PORT)
            m.call('buffer_hline', [W(x0), W(x1), W(y), Block(b.port_bytes()), W(color)])
            hl(x0, x1, y, b.port_c(), bytes([color]))
            b.compare('case %d: hline %d..%d at %d in %dx%d' % (case, x0, x1, y, w, h))
        else:
            x = rng.randrange(-3, w + 3)
            y0 = rng.randrange(-10, h)
            y1 = rng.randrange(max(y0, 0), h + 10)
            m.call('buffer_vline', [W(x), W(y0), W(y1), Block(b.port_bytes()), W(color)])
            vl(x, y0, y1, b.port_c(), bytes([color]))
            b.compare('case %d: vline %d..%d at %d in %dx%d' % (case, y0, y1, x, w, h))


def test_buffer_pointer(m, lib, rng):
    f = lib.buffer_pointer
    f.argtypes = [ctypes.POINTER(DosPort), ctypes.c_int16, ctypes.c_int16]
    f.restype = ctypes.c_void_p
    for case in range(N):
        m.reset_heap()
        w, h = sizes(rng)
        b = Buffers(m, w, h, rng)
        x, y = rng.randrange(0, w), rng.randrange(0, h)
        # the original takes a near pointer to the DosPort: put it in DGROUP's free tail
        m.write_ds(0xfff0 - 16, b.port_bytes())
        ax, dx = m.call('buffer_pointer', [P(0xfff0 - 16), W(x), W(y)])
        theirs = dx * 16 + ax - b.seg * 16
        p = b.port_c()
        ours = f(ctypes.byref(p), x, y) - p.bits
        assert ours == theirs, 'case %d: (%d,%d) in %dx%d at +%d, the original +%d' % (
            case, x, y, w, h, ours, theirs)
