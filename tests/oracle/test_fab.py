"""The FAB codec (port/lib/fab.c): the game's own packed art, decoded by the original and the
port; and the port's encoder read back by the port's decoder."""
import ctypes, glob, os, struct

from emu import D

FFIO = ctypes.CFUNCTYPE(ctypes.c_uint16, ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint16))


def fab_entries(game_dir, limit):
    out = []
    for path in sorted(glob.glob(os.path.join(game_dir, '*.SS')) + glob.glob(os.path.join(game_dir, '*.ss'))):
        data = open(path, 'rb').read()
        if not data.startswith(b'MADSPACK 2.0\x1a'):
            continue
        count = struct.unpack_from('<H', data, 0x0e)[0]
        at = 0xb0
        for i in range(min(count, 16)):
            typ, flag, size, packed = struct.unpack_from('<BBII', data, 0x10 + 10 * i)
            if typ == 1 and size <= 60000:
                out.append((os.path.basename(path), i, data[at:at + packed], size))
            at += packed
        if len(out) >= limit:
            break
    return out[:limit]


def test_fab_decode_game_art(m, lib, rng):
    game = os.path.dirname(os.environ['MAPEDIT_EXE'])
    f = lib.pfabexp2
    f.argtypes = [ctypes.c_char_p, ctypes.c_void_p, ctypes.c_void_p]
    f.restype = ctypes.c_int16
    entries = fab_entries(game, 40)
    assert entries, 'no packed art found beside MAPEDIT.EXE'
    for name, i, packed, size in entries:
        m.reset_heap()
        sseg, dseg, wseg = m.alloc(len(packed) + 16), m.alloc(size + 16), m.alloc(64)
        m.write(sseg, 0, packed)
        ax, _ = m.call('pfabexp2', [D(sseg << 16), D(dseg << 16), D(wseg << 16)])
        theirs = m.read(dseg, 0, size)
        out = ctypes.create_string_buffer(size + 16)
        r = f(packed, ctypes.cast(out, ctypes.c_void_p), None)
        assert r == 0, '%s entry %d: the port returned %d' % (name, i, r)
        assert out.raw[:size] == theirs, '%s entry %d: %d bytes differ' % (
            name, i, sum(a != b for a, b in zip(out.raw[:size], theirs)))


def test_fab_round_trip(m, lib, rng):
    comp, exp0 = lib.pfabcomp, lib.pfabexp0
    comp.argtypes = [FFIO, FFIO, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
    exp0.argtypes = [FFIO, FFIO, ctypes.c_void_p]
    comp.restype = exp0.restype = ctypes.c_int16
    for case in range(30):
        data = bytes(rng.randrange(256) for _ in range(rng.randrange(0, 5000)))
        state = {'at': 0, 'out': bytearray()}

        def reader(src):
            def rd(buf, n):
                k = min(n[0], len(src) - state['at'])
                ctypes.memmove(buf, src[state['at']:state['at'] + k], k)
                state['at'] += k
                return k
            return rd

        def wr(buf, n):
            state['out'] += ctypes.string_at(buf, n[0])
            return n[0]
        rd_c, wr_c = FFIO(reader(data)), FFIO(wr)
        assert comp(rd_c, wr_c, None, None, None) == 0
        packed = bytes(state['out'])
        state['at'], state['out'] = 0, bytearray()
        rd2, wr2 = FFIO(reader(packed)), FFIO(wr)
        assert exp0(rd2, wr2, None) == 0, 'case %d: decode failed' % case
        assert bytes(state['out']) == data, 'case %d: %d bytes back, %d in' % (case, len(state['out']), len(data))
