"""emu.py -- run one function of the player's MAPEDIT.EXE in a real-mode x86 emulator.

The oracle for the port's per-function tests (tests/oracle/test_*.py): the original machine
code, from the player's own copy of the program, called with the arguments a test chooses,
in the memory the test sets up. Nothing of the program is in this repository: the EXE comes
from $MAPEDIT_EXE, and functions are found by their names in its own symbol table (the
CodeView information MS LINK appended to it).

    m = Machine(path_to_mapedit_exe)
    seg = m.alloc(64000)                        # a far block, returns its segment
    ax, dx = m.call('buffer_rect_fill', [W(10), W(20), W(30), Block(port_bytes), W(5), W(7)])

Calling conventions (Microsoft C 6.00A, medium model): every function is far. `_name` is
cdecl (arguments pushed right to left, the caller pops); `@name` is _fastcall (the first
near pointer in BX; the other word-sized arguments -- int, char, near pointer -- in AX, then
DX, then BX if no pointer took it; the rest, and every far pointer, long and structure,
pushed left to right; the callee pops: seen in buffer_pointer, which reads its DosPort
through BX, and in buffer_rect_fill's call of buffer_conform); an
upper-case name is pascal (pushed left to right, the callee pops). Results come back in AX,
or DX:AX.

Requires the `unicorn` Python package.
"""
import struct

from unicorn import Uc, UC_ARCH_X86, UC_MODE_16, UC_HOOK_INTR, UcError
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DX,
                               UC_X86_REG_SI, UC_X86_REG_DI, UC_X86_REG_BP, UC_X86_REG_SP,
                               UC_X86_REG_CS, UC_X86_REG_DS, UC_X86_REG_ES, UC_X86_REG_SS,
                               UC_X86_REG_IP)

LOAD = 0x1000          # the program's load segment
RET_SEG, RET_OFF = 0x0050, 0x0000   # where a call returns to: a HLT
HEAP = 0x4000          # the first segment handed out by alloc()
TOP = 0x9f00           # and the last


class W:
    """A word argument (int, unsigned, char, near pointer)."""
    def __init__(self, v):
        self.v = v & 0xffff


class P(W):
    """A near pointer argument (a DS offset): _fastcall passes the first one in BX."""


class D:
    """A doubleword argument (long, far pointer as seg << 16 | off)."""
    def __init__(self, v):
        self.v = v & 0xffffffff


class Block:
    """A structure passed by value: its bytes, as they lie in memory."""
    def __init__(self, data):
        self.data = bytes(data) + (b'\0' if len(data) % 2 else b'')


def codeview_publics(raw):
    """{symbol: (seg, off)} from the NB02 information at the end of the EXE."""
    if raw[-8:-4] != b'NB02':
        raise ValueError('this EXE has no CodeView information')
    base = len(raw) - struct.unpack_from('<I', raw, len(raw) - 4)[0]
    d = base + struct.unpack_from('<I', raw, base + 4)[0]
    out = {}
    for i in range(struct.unpack_from('<H', raw, d)[0]):
        typ, imod, lfo, cb = struct.unpack_from('<HHIH', raw, d + 2 + 10 * i)
        if typ != 0x102:
            continue
        p = q = base + lfo
        while q < p + cb:
            off, seg, ptype, ln = struct.unpack_from('<HHHB', raw, q)
            name = raw[q + 7:q + 7 + ln].decode('latin-1')
            q += 7 + ln
            if ptype == 0:
                out[name] = (seg, off)
    return out


class Machine:
    def __init__(self, exe_path):
        raw = open(exe_path, 'rb').read()
        lastpage, pages, nrel, hdrpar = struct.unpack_from('<HHHH', raw, 2)
        ss, sp, _, ip, cs, relo = struct.unpack_from('<HHHHHH', raw, 0x0e)
        hdr = hdrpar * 16
        end = (pages - 1) * 512 + (lastpage or 512)
        image = bytearray(raw[hdr:end])
        for i in range(nrel):
            off, seg = struct.unpack_from('<HH', raw, relo + 4 * i)
            p = seg * 16 + off
            struct.pack_into('<H', image, p, (struct.unpack_from('<H', image, p)[0] + LOAD) & 0xffff)
        self.publics = codeview_publics(raw)
        k = image.index(b'\xbf', cs * 16 + ip)          # crt0: mov di, DGROUP
        self.dgroup = struct.unpack_from('<H', image, k + 1)[0]   # relocated above already
        # the test heap must lie above all of DGROUP's 64 KB, and DGROUP inside the image
        assert self.dgroup * 16 + 0x10000 <= HEAP * 16 and self.dgroup < LOAD + len(image) // 16, \
            'DGROUP %04x does not fit the emulator layout' % self.dgroup
        # crt0 moves the stack into DGROUP (SS = DS, as the medium model needs: a pointer to a
        # local is a DS offset), keeping its linear address
        self.ss = self.dgroup
        self.sp0 = ((LOAD + ss - self.dgroup) * 16 + sp) & 0xffff
        self.uc = Uc(UC_ARCH_X86, UC_MODE_16)
        self.uc.mem_map(0, 0x110000)
        self.uc.mem_write(LOAD * 16, bytes(image))
        self.uc.mem_write(RET_SEG * 16 + RET_OFF, b'\xf4')     # HLT
        self.uc.hook_add(UC_HOOK_INTR, self._intr)
        self.image = bytes(image)
        self.next_seg = HEAP
        self.interrupts = []

    # ---- memory ----
    def alloc(self, size, fill=0):
        """A far block of `size` bytes (offset 0); returns its segment."""
        seg = self.next_seg
        self.next_seg += (size + 15) // 16 + 1
        if self.next_seg > TOP:
            raise MemoryError('emulator heap exhausted')
        self.uc.mem_write(seg * 16, bytes([fill]) * size)
        return seg

    def reset_heap(self):
        self.next_seg = HEAP

    def read(self, seg, off, n):
        return bytes(self.uc.mem_read(seg * 16 + off, n))

    def write(self, seg, off, data):
        self.uc.mem_write(seg * 16 + off, bytes(data))

    def read_ds(self, off, n):
        return self.read(self.dgroup, off, n)

    def write_ds(self, off, data):
        self.write(self.dgroup, off, data)

    def address(self, name):
        """(seg, off) of a function by its C name, and its convention."""
        for sym, conv in (('_' + name, 'cdecl'), ('@' + name, 'fastcall'), (name.upper(), 'pascal')):
            if sym in self.publics:
                seg, off = self.publics[sym]
                return LOAD + seg, off, conv
        raise KeyError(name)

    # ---- calls ----
    def _intr(self, uc, intno, ud):
        self.interrupts.append(intno)
        uc.emu_stop()

    def call(self, name, args, max_instr=50_000_000):
        """Call a function with W/D/Block arguments; returns (ax, dx)."""
        cs, ip, conv = self.address(name)
        uc = self.uc
        sp = self.sp0
        regs = {}

        def push_words(words):
            nonlocal sp
            for w in words:
                sp = (sp - 2) & 0xffff
                uc.mem_write(self.ss * 16 + sp, struct.pack('<H', w & 0xffff))

        def words_of(a):
            """the words of one argument, in push order (the high word first)"""
            if isinstance(a, W):
                return [a.v]
            if isinstance(a, D):
                return [a.v >> 16, a.v & 0xffff]
            data = a.data
            return [struct.unpack_from('<H', data, i)[0] for i in range(len(data) - 2, -1, -2)]

        if conv == 'fastcall':
            stack = []
            first_ptr = next((a for a in args if isinstance(a, P)), None)
            if first_ptr is not None:
                regs[UC_X86_REG_BX] = first_ptr.v
            reg_order = [UC_X86_REG_AX, UC_X86_REG_DX] + ([] if first_ptr is not None else [UC_X86_REG_BX])
            for a in args:
                if a is first_ptr:
                    continue
                if isinstance(a, W) and reg_order:
                    regs[reg_order.pop(0)] = a.v
                else:
                    stack.append(a)
            for a in stack:                         # left to right
                push_words(words_of(a))
        elif conv == 'pascal':
            for a in args:
                push_words(words_of(a))
        else:
            for a in reversed(args):                # right to left
                push_words(words_of(a))
        push_words([RET_SEG, RET_OFF])
        for r, v in ((UC_X86_REG_SS, self.ss), (UC_X86_REG_SP, sp), (UC_X86_REG_DS, self.dgroup),
                     (UC_X86_REG_ES, self.dgroup), (UC_X86_REG_BP, 0), (UC_X86_REG_SI, 0),
                     (UC_X86_REG_DI, 0), (UC_X86_REG_CX, 0), (UC_X86_REG_AX, 0),
                     (UC_X86_REG_BX, 0), (UC_X86_REG_DX, 0)):
            uc.reg_write(r, v)
        for r, v in regs.items():
            uc.reg_write(r, v)
        uc.reg_write(UC_X86_REG_CS, cs)
        uc.reg_write(UC_X86_REG_IP, ip)
        self.interrupts = []
        try:
            uc.emu_start(cs * 16 + ip, RET_SEG * 16 + RET_OFF, count=max_instr)
        except UcError as e:
            raise RuntimeError('%s faulted at %04x:%04x: %s' % (
                name, uc.reg_read(UC_X86_REG_CS), uc.reg_read(UC_X86_REG_IP), e))
        if self.interrupts:
            raise RuntimeError('%s called INT %02xh, which this harness does not provide'
                               % (name, self.interrupts[0]))
        here = uc.reg_read(UC_X86_REG_CS) * 16 + uc.reg_read(UC_X86_REG_IP)
        if here != RET_SEG * 16 + RET_OFF:
            raise RuntimeError('%s did not return (stopped at %05x)' % (name, here))
        return uc.reg_read(UC_X86_REG_AX), uc.reg_read(UC_X86_REG_DX)
