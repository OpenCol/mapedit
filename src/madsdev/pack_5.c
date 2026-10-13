/* From mapedit-decomp src/madsdev/pack_5 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
#include "pack.h"
#include "records.h"
#include "units.h"
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

