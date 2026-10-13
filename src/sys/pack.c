/* pack.c -- packing and unpacking: the stream plumbing around the FAB codec (fab.c). */
#include <string.h>

#include "msc.h"
#include "error.h"
#include "fileio.h"
#include "font.h"
#include "loader.h"
#include "mem.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "sprite.h"
#include "units.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */

/* MODULE pack_1: MAPEDIT.EXE segment 1359 (PACK_1.C), file 0x14b96-0x14c0a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_read_memory 0x14b96
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_1066).
 *
 * THE MEMORY READER FOR IMPLODE/EXPLODE (root code past the C runtime, frame 1066; pascal).
 * pack_read_memory(buf, n) copies up to *n bytes from pack_read_memory_ptr into buf, capped by pack_read_size
 * unless its high word is -1 (unlimited); a zero allowance returns 0. It advances pack_read_memory_ptr
 * and normalises it with mem_normalize, and counts the bytes into pack_read_count. The body is an
 * `_asm` block (the `push ax; or ax,dx; pop ax` zero test and `rep movsb` are not MSC's).
 */


/* pack_read_memory -- 1359:0006, 116 bytes */
/* PORT: pack_read_memory is in port/ (inline assembly) */


/* The module's uninitialised data (communal, BSS). */
char *pack_read_memory_ptr;           /* DS:0x6348 */

/* MODULE pack_2: MAPEDIT.EXE segment 1360 (PACK_2.C), file 0x14c0a-0x14c78, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_write_memory 0x14c0a
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_106d).
 *
 * THE MEMORY WRITER FOR IMPLODE/EXPLODE (root code past the C runtime, frame 106d; pascal).
 * pack_write_memory(buf, n) is ff_read_mem (root_1066.c) the other way round: up to *n bytes of
 * buf into pack_write_memory_ptr, capped by pack_write_size unless its high word is -1, counted into
 * pack_write_count, pack_write_memory_ptr advanced and normalised. It has no return statement: it returns what
 * mem_normalize left in ax (the new offset), not the count -- the shipped code keeps no
 * local for it.
 */


/* pack_write_memory -- 1360:000a, 110 bytes */
/* PORT: pack_write_memory is in port/ (inline assembly) */


/* The module's uninitialised data (communal, BSS). */
char *pack_write_memory_ptr;          /* DS:0x5a9e */

/* MODULE pack_3: MAPEDIT.EXE segment 1367 (PACK_3.C), file 0x14c78-0x14cf2, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_read_file 0x14c78
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_1074).
 *
 * THE FILE READER FOR IMPLODE/EXPLODE (root code past the C runtime, frame 1074; pascal).
 * pack_read_file(buf, n) is one of the four I/O callbacks ff_decode (ovl28_772ba.c) hands the
 * compression library: it reads up to *n bytes from pack_read_file_handle into buf with fileio_fread_f,
 * capped by pack_read_size when that is not negative (unlimited), and counts them into pack_read_count.
 */



/* pack_read_file -- 1367:0008, 122 bytes */
u16 pack_read_file(void *data, u16 *n)
{
    char *buf = data;
    register u16 got;

    if (pack_read_size >= 0)
        got = (u16)min((i32)*n, pack_read_size);
    else
        got = *n;
    if (got != 0) {
        got = (u16)fileio_fread_f(buf, 1L, (i32)got, pack_read_file_handle);
        if (pack_read_size > 0)
            pack_read_size -= got;
        pack_read_count += got;
    }
    return got;
}

/* The module's uninitialised data (communal, BSS). */
MSC_FILE *pack_read_file_handle;              /* DS:0x4e7c */

/* MODULE pack_4: MAPEDIT.EXE segment 136f (PACK_4.C), file 0x14cf2-0x14d84, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_write_file 0x14cf2
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_107c).
 *
 * THE FILE WRITER FOR IMPLODE/EXPLODE (root code past the C runtime, frame 107c; pascal).
 * pack_write_file(buf, n) writes up to *n bytes of buf to pack_write_file_handle with fileio_fwrite_f, capped
 * by pack_write_size when that is not negative, and counts them into pack_write_count; a failed write
 * returns 0. The twin of ff_read_file (root_1074.c).
 */



/* pack_write_file -- 136f:0002, 146 bytes */
u16 pack_write_file(void *data, u16 *n)
{
    char *buf = data;
    register u16 got;
    i32 len;

    if (pack_write_size >= 0)
        got = (u16)min((i32)*n, pack_write_size);
    else
        got = *n;
    if (got != 0) {
        if (fileio_fwrite_f(buf, len = got, 1L, pack_write_file_handle) == 0)
            got = 0;
        else {
            if (pack_write_size > 0)
                pack_write_size -= len;
            pack_write_count += len;
        }
    }
    return got;
}

/* The module's uninitialised data (communal, BSS). */
MSC_FILE *pack_write_file_handle;             /* DS:0x4e7a */

/* MODULE pack_5: MAPEDIT.EXE segment 1202 (pack_5.c), file 0x13628-0x137a0, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_raw_copy 0x13628
 * fn pack_a_packet 0x136a6
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl28_776f4).
 *
 * MADSPACK CODEC DISPATCH (overlay 28, para 138: the segment starts at 0x776f0, inside
 * far_fwrite's last bytes). DOS-only engine code; the state is the g_ff_* block that
 * ff_decode (ovl28_772ba.c) fills.
 *   - pack_raw_copy() (thunk 181f:2fe8) is the stored method: while pack_read_size > 0 and no
 *     error, it moves min(pack_buffer_size, pack_read_size) bytes through the work buffer with
 *     the pascal reader and writer callbacks (which keep the counters). A short read is
 *     error 4.
 *   - pack_a_packet(method, kind) (_fastcall, thunk 181f:2ee4) runs one codec call: method 0
 *     calls an implode-style packer (pack_strategy 1: DS:0x26d4, else 0x26cc) with the level
 *     word DS:0x26c6 and a 0x1000 dictionary size; method 1 an explode-style unpacker
 *     (pack_strategy 1: kind 0 reader->writer 0x26d8, kind 1 reader->memory 0x26dc, kind 2
 *     memory->memory 0x26e0; else 0x26d0); anything else pack_raw_copy. Every codec is a far
 *     pascal pointer (the calls are followed by `leave`, so cdecl is not ruled out).
 *
 * `#pragma optimize("elg")`. Both switches open with `default:` (the dispatch chain then
 * falls into the default arm). pack_raw_copy's min must be computed as a long and narrowed.
 */



/* pack_raw_copy -- 1202:0008, 126 bytes */
i16 pack_raw_copy(void)
{
    u16 n;
    u16 got;
    i16 err;

    err = 0;
    while (pack_read_size > 0 && err == 0) {
        n = (u16)((i32)pack_buffer_size > pack_read_size ? pack_read_size : (i32)pack_buffer_size);
        got = pack_read_routine(pack_buffer, &n);
        if (got != n)
            err = 4;
        else
            pack_write_routine(pack_buffer, &n);
    }
    return err;
}
/* pack_a_packet -- 1202:0086, 250 bytes */


i16 pack_a_packet(i16 method, i16 kind)
{
    u16 dsize;

    switch (method) {
    default:
        return pack_raw_copy();
    case 0:
        dsize = 0x1000;
        if (pack_strategy == 1)
            return pack_pFABcomp_routine(pack_read_routine, pack_write_routine, pack_buffer, &pack_mode, &dsize);
        return pack_implode_routine(pack_read_routine, pack_write_routine, pack_buffer, &pack_mode, &dsize);
    case 1:
        if (pack_strategy == 1) {
            switch (kind) {
            default:
                return pack_pFABexp0_routine(pack_read_routine, pack_write_routine, pack_buffer);
            case 1:
                return pack_pFABexp1_routine(pack_read_routine, pack_write_memory_ptr, pack_buffer);
            case 2:
                return pack_pFABexp2_routine(pack_read_memory_ptr, pack_write_memory_ptr, pack_buffer);
            }
        }
        return pack_explode_routine(pack_read_routine, pack_write_routine, pack_buffer);
    }
}

/* The packer's data, defined here after the code (a definition after the last function
 * moves no symbol position). The initialised ones land at DS:0x4488-0x44a6 in this order
 * (the module has no literals); the rest are communals (BSS). */
i16 pack_mode = 0;                        /* DS:0x4488 */
char pack_zip_enabled = 0;                /* DS:0x448a */
char pack_pfab_enabled = 0;               /* DS:0x448b */
i16 pack_strategy = 1;                    /* DS:0x448c */
FFPACK pack_implode_routine = 0;          /* DS:0x448e */
FFUNPACK pack_explode_routine = 0;        /* DS:0x4492 */
FFPACK pack_pFABcomp_routine = 0;         /* DS:0x4496 */
FFUNPACK pack_pFABexp0_routine = 0;       /* DS:0x449a */
FFUNPACK_TO pack_pFABexp1_routine = 0;    /* DS:0x449e */
FFUNPACK_MEM pack_pFABexp2_routine = 0;   /* DS:0x44a2 */
i32 pack_write_size;                     /* DS:0x4a84 */
i32 pack_read_count;                     /* DS:0x4b0a */
void *pack_buffer;                    /* DS:0x4ea4 */
i32 pack_read_size;                      /* DS:0x5290 */
u16 pack_buffer_size;                /* DS:0x6028 */
FFIO pack_read_routine;                   /* DS:0x6042 */
FFIO pack_write_routine;                  /* DS:0x64e8 */
i32 pack_write_count;                    /* DS:0x6984 */

/* MODULE pack_6: MAPEDIT.EXE segment 1309 (pack_6.c), file 0x1469a-0x149ea, 4 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn far_to_near             0x1469a
 * fn io_call_hook            0x146a4
 * fn pack_set_special_buffer 0x146ba
 * fn pack_data                 0x146da
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl28_772ba).
 *
 * FILE-I/O HOOKS (overlay 28, para 70: the segment starts at 0x772b0, inside the previous
 * module's last function, and this module's code at 0x772ba). DOS-only engine code.
 *   - far_to_near returns the offset half of a far pointer (inline les).
 *   - io_call_hook calls the far hook pack_special_function, if one is set (inline asm, which is why
 *     di is saved).
 *   - pack_set_special_buffer installs two far hooks: pack_special_buffer (DS:0x245a) and pack_special_function
 *     (DS:0x245e).
 *   - ff_decode(method, from_file, to_file, bytes, src, dst) (_fastcall, thunk 181f:2eba)
 *     is the MADSPACK codec driver, a wrapper round the PKWARE Data Compression Library
 *     (the work-buffer sizes are implode's 0x89b8 and explode's 0x311e). It installs the
 *     pascal reader/writer callbacks in g_ff_reader/g_ff_writer (root code at 181f:2f10 /
 *     2f06 for a far buffer / FILE source, 2efc / 2ef2 for the destination; to_file 2 keeps
 *     the writer), sets the byte counters g_ff_in/out(_left), and picks a work size and
 *     codec per method: 0 = compress (g_ff_type 1: 0x71be, else implode 0x89b8), 1 =
 *     expand (g_ff_type 1: three variants, memory-to-memory, to memory, or to the writer;
 *     else explode 0x311e), anything else = stored copy (0x1000). A missing codec pointer
 *     is fatal_error(-29, 3, 24, method, g_ff_type). The work buffer is pack_special_buffer when
 *     set (so that "hook" is really a caller-supplied work block), otherwise
 *     far_alloc_tagged(size, "$pack$"). It then runs ff_codec (181f:2ee4, ovl28_775ec.c)
 *     once (expanding to memory) or until the counter reaches 0; a codec error is
 *     fatal_error(-28, 3, 24, ...). Returns the selected byte count.
 *     `#pragma optimize("elg")` for this function only (si/di register locals). The
 *     `to_file != 1 && to_file != 2` spelling puts the 0x820 arm first, as shipped.
 */

void (*pack_special_buffer)(void) = 0;          /* DS:0x4512 (defined here: before pack_data's "$pack$") */
void (*pack_special_function)(void) = 0;        /* DS:0x4516 */
/* far_to_near -- 1309:000a, 10 bytes */
/* PORT: far_to_near is in port/ (inline assembly) */

/* io_call_hook -- 1309:0014, 22 bytes */
/* PORT: io_call_hook is in port/ (inline assembly) */

/* pack_set_special_buffer -- 1309:002a, 32 bytes */
void pack_set_special_buffer(void (*a)(void), void (*b)(void))
{
    pack_special_buffer = a;
    pack_special_function = b;
}
/* pack_data -- 1309:004a, 784 bytes */

i32 pack_data(i16 method, i16 from_file, i16 to_file, i32 bytes,
                         void *src, void *dst)
{
    i32 *count;
    i16 kind;
    i32 *left;
    i16 r;

    kind = 0;
    if (from_file == 0) {
        pack_read_routine = pack_read_memory;
        pack_read_memory_ptr = src;
    } else {
        pack_read_routine = pack_read_file;
        pack_read_file_handle = (MSC_FILE *)src;   /* PORT: the whole pointer (1994: its offset, far_to_near) */
    }
    if (to_file == 0) {
        pack_write_routine = pack_write_memory;
        pack_write_memory_ptr = dst;
    } else {
        pack_write_routine = pack_write_file;
        pack_write_file_handle = (MSC_FILE *)dst;   /* PORT: the whole pointer (1994: its offset, far_to_near) */
    }
    pack_read_count = pack_write_count = 0;
    switch (method) {
    default:
        pack_buffer_size = 0x1000;
        pack_write_size = pack_read_size = bytes;
        left = &pack_read_size;
        count = &pack_write_count;
        break;
    case 0:
        if (pack_strategy == 1) {
            pack_buffer_size = 0xd1d0;
            if (pack_pFABcomp_routine == 0)
                error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
        } else {
            pack_buffer_size = 0x89b8;
            if (pack_implode_routine == 0)
                error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
        }
        pack_read_size = bytes;
        pack_write_size = -1L;
        left = &pack_read_size;
        count = &pack_read_count;
        break;
    case 1:
        pack_read_size = -1L;
        pack_write_size = bytes;
        left = &pack_write_size;
        if (pack_strategy == 1) {
            if (from_file == 0 && to_file == 0 && pack_pFABexp2_routine != 0) {
                count = &bytes;
                pack_buffer_size = 4;
                kind = 2;
            } else if (to_file != 1 && to_file != 2) {
                count = &bytes;
                pack_buffer_size = 0x822;
                kind = 1;
                if (pack_pFABexp1_routine == 0)
                    error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
            } else {
                count = &pack_write_count;
                pack_buffer_size = 0x382e;
                kind = 0;
                if (pack_pFABexp0_routine == 0)
                    error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
            }
        } else {
            count = &pack_write_count;
            pack_buffer_size = 0x311e;
            if (pack_explode_routine == 0)
                error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
        }
        break;
    }
    pack_buffer = 0;
    if (pack_special_buffer == 0) {
        pack_buffer = mem_get_name((i32)pack_buffer_size, "$pack$");
        if (pack_buffer == 0) {
            *count = 0;
            goto out;
        }
    } else
        pack_buffer = (void *)pack_special_buffer;
    if (method == 1 && to_file == 0) {
        r = pack_a_packet(method, kind);
        if (r != 0) {
            *count = 0;
            error_report(-28, 3, 24, (i32)(method * 1000 + kind), (i32)r);
        }
    } else {
        while (*left > 0) {
            if (pack_a_packet(method, 0) != 0) {
                *count = 0;
                error_report(-28, 3, 24, (i32)method, (i32)to_file);
                break;
            }
        }
    }
out:
    if (pack_special_buffer == 0) {
        if (pack_buffer != 0)
            mem_free(pack_buffer);
    } else
        io_call_hook();
    return *count;
}

/* far_to_near -- 1309:000a: a far pointer's offset. The port's pointers have no segment;
   what callers hand on is the pointer's low bits, as the original's offset. */
u16 far_to_near(void *p)
{
    return (u16)(uintptr_t)p;
}

/* io_call_hook -- 1309:0014: the packers' special function, when there is one */
void io_call_hook(void)
{
    if (pack_special_function)
        pack_special_function();
}

/* pack_read_memory -- 1359:0006: the packers' reader over a block of memory. Takes *n bytes,
   no more than pack_read_size has left, counts them, returns how many. A size whose high
   word is FFFFh (any negative one) means no limit; only a size under 64 KB clamps. *n is not
   written back. */
u16 pack_read_memory(void *data, u16 *n)
{
    char *buf = data;
    u16 c = *n;
    if (((u32)pack_read_size >> 16) != 0xffff) {
        if (pack_read_size == 0)
            return 0;
        if (((u32)pack_read_size >> 16) == 0 && c > (u16)pack_read_size)
            c = (u16)pack_read_size;
        pack_read_size -= c;
    }
    pack_read_count += c;
    if (c) {
        memcpy(buf, pack_read_memory_ptr, c);
        pack_read_memory_ptr += c;
    }
    return c;
}

/* pack_write_memory -- 1360:000a: the writer, the same way round. The original has no return
   statement and returns what mem_normalize left in AX, the new pointer's offset after
   normalising (0-15), which depends on where DOS put the block; PKWARE's library, which calls
   it, ignores a writer's result. The port returns the count. */
u16 pack_write_memory(void *data, u16 *n)
{
    char *buf = data;
    u16 c = *n;
    if (((u32)pack_write_size >> 16) != 0xffff) {
        if (pack_write_size == 0)
            return 0;
        if (((u32)pack_write_size >> 16) == 0 && c > (u16)pack_write_size)
            c = (u16)pack_write_size;
        pack_write_size -= c;
    }
    pack_write_count += c;
    if (c) {
        memcpy(pack_write_memory_ptr, buf, c);
        pack_write_memory_ptr += c;
    }
    return c;
}

/* MODULE pack_9: MAPEDIT.EXE segment 0f16 (pack_9.c), file 0x1076e-0x107aa, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_enable_pfab 0x1076e
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * pack_enable_pfab() switches the MADSPACK codec to the FAB compressor: strategy 1, the
 * enable flag, and the four pascal routines PFABCOMP and PFABEXP0..2 (pfab*.ASM) into
 * pack_5.c's routine pointers.
 */

/* pack_enable_pfab -- 0f16:000e, 60 bytes */
void pack_enable_pfab(void)
{
    pack_strategy = 1;
    pack_pfab_enabled = -1;
    pack_pFABcomp_routine = pfabcomp;
    pack_pFABexp0_routine = pfabexp0;
    pack_pFABexp1_routine = pfabexp1;
    pack_pFABexp2_routine = pfabexp2;
}
