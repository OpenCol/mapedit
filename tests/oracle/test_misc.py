"""Small library routines (port/lib/misc.c) against the original's machine code."""
import ctypes, struct

from emu import W, D

N = 300


def test_magic_hash_color(m, lib, rng):
    f = lib.magic_hash_color
    f.argtypes = [ctypes.c_char_p]
    f.restype = ctypes.c_int16
    seg = m.alloc(16)
    for case in range(N):
        rgb = bytes(rng.randrange(256) for _ in range(3))
        m.write(seg, 0, rgb)
        ax, _ = m.call('magic_hash_color', [D(seg << 16)])
        r = f(rgb) & 0xffff
        assert r == ax, 'rgb %s: %04x, the original %04x' % (rgb.hex(), r, ax)


def test_sort_insertion_8(m, lib, rng):
    f = lib.sort_insertion_8
    f.argtypes = [ctypes.c_int16, ctypes.c_char_p, ctypes.c_char_p]
    for case in range(N):
        m.reset_heap()
        n = rng.randrange(1, 40)
        keys = bytes(rng.choice([rng.randrange(256), rng.randrange(4)]) for _ in range(n))
        pay = bytes(range(n))
        ks, ps = m.alloc(n), m.alloc(n)
        m.write(ks, 0, keys)
        m.write(ps, 0, pay)
        m.call('sort_insertion_8', [W(n), D(ps << 16), D(ks << 16)])
        ck, cp = ctypes.create_string_buffer(keys, n), ctypes.create_string_buffer(pay, n)
        f(n, cp, ck)
        assert ck.raw == m.read(ks, 0, n) and cp.raw == m.read(ps, 0, n), \
            'case %d: keys %s -> %s / %s, payload %s / %s' % (case, keys.hex(), ck.raw.hex(),
                                                             m.read(ks, 0, n).hex(), cp.raw.hex(),
                                                             m.read(ps, 0, n).hex())


def test_pack_memory_streams(m, lib, rng):
    for which in ('read', 'write'):
        f = getattr(lib, 'pack_%s_memory' % which)
        f.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_uint16)]
        f.restype = ctypes.c_uint16
        size_c = ctypes.c_int32.in_dll(lib, 'pack_%s_size' % which)
        count_c = ctypes.c_int32.in_dll(lib, 'pack_%s_count' % which)
        ptr_c = ctypes.c_void_p.in_dll(lib, 'pack_%s_memory_ptr' % which)
        for case in range(N):
            m.reset_heap()
            size = rng.choice([-1, -2, 0, rng.randrange(1, 300), 0x10000 + rng.randrange(100),
                               rng.randrange(0, 0x7fffffff)])
            n = rng.randrange(0, 300)
            data = bytes(rng.randrange(256) for _ in range(300))
            buf_seg, mem_seg, n_at = m.alloc(300), m.alloc(300), 0xfff0 - 4
            m.write(buf_seg, 0, data if which == 'write' else bytes(300))
            m.write(mem_seg, 0, data if which == 'read' else bytes(300))
            m.write_ds(m.data('pack_%s_size' % which), struct.pack('<i', size))
            m.write_ds(m.data('pack_%s_count' % which), struct.pack('<i', 7))
            m.write_ds(m.data('pack_%s_memory_ptr' % which), struct.pack('<HH', 0, mem_seg))
            m.write(m.dgroup, n_at, struct.pack('<H', n))
            ax, _ = m.call('pack_%s_memory' % which, [D(buf_seg << 16), D(m.dgroup << 16 | n_at)])
            cbuf = ctypes.create_string_buffer(data if which == 'write' else bytes(300), 300)
            cmem = ctypes.create_string_buffer(data if which == 'read' else bytes(300), 300)
            size_c.value, count_c.value = size, 7
            ptr_c.value = ctypes.cast(cmem, ctypes.c_void_p).value
            cn = ctypes.c_uint16(n)
            r = f(cbuf, ctypes.byref(cn))
            what = 'pack_%s_memory case %d: size %d, n %d' % (which, case, size, n)
            if which == 'read':       # the writer's result is undefined (port/lib/misc.c)
                assert r == ax, '%s: returned %d, the original %d' % (what, r, ax)
            assert size_c.value == struct.unpack('<i', m.read_ds(m.data('pack_%s_size' % which), 4))[0], what + ': size'
            assert count_c.value == struct.unpack('<i', m.read_ds(m.data('pack_%s_count' % which), 4))[0], what + ': count'
            assert cbuf.raw == m.read(buf_seg, 0, 300) and cmem.raw == m.read(mem_seg, 0, 300), what + ': bytes'


def test_pal_get_hash(m, lib, rng):
    f = lib.pal_get_hash
    f.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    f.restype = ctypes.c_int16
    seg = m.alloc(16)
    for case in range(N):
        a = bytes(rng.randrange(256) for _ in range(3))
        b = bytes(rng.randrange(256) for _ in range(3))
        m.write(seg, 0, b + a)
        ax, _ = m.call('pal_get_hash', [D(seg << 16), D(seg << 16 | 3)])
        r = f(b, a) & 0xffff
        assert r == ax, 'b %s a %s: %04x, the original %04x' % (b.hex(), a.hex(), r, ax)
