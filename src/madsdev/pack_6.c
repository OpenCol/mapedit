/* Imported from mapedit-decomp src/madsdev/pack_6 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE pack_6: MAPEDIT.EXE segment 1309 (pack_6.c), file 0x1469a-0x149ea, 4 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn far_to_near             0x1469a
 * fn io_call_hook            0x146a4
 * fn pack_set_special_buffer 0x146ba
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

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern void (*pack_special_buffer)(void);       /* DS:0x245a */
extern void (*pack_special_function)(void);         /* DS:0x245e */
/* far_to_near -- 1309:000a, 10 bytes */
/* PORT: far_to_near is in port/ (inline assembly) */
u16 far_to_near(void *p);

/* io_call_hook -- 1309:0014, 22 bytes */
/* PORT: io_call_hook is in port/ (inline assembly) */
void io_call_hook(void);

/* pack_set_special_buffer -- 1309:002a, 32 bytes */
void pack_set_special_buffer(void (*a)(void), void (*b)(void))
{
    pack_special_buffer = a;
    pack_special_function = b;
}

// MSC: #pragma optimize("elg", on)
typedef u16 (*FFIO)(void *buf, u16 *n);

extern void *far_alloc_tagged(i32 size, char *tag);
extern void fatal_error(i16 code, i16 level, i16 module, i32 data1, i32 data2);
extern i16 POPUP_DTOR(void *p);
extern i16 ff_codec(i16 method, i16 kind);
extern u16 ff_read_mem(void *buf, u16 *n);
extern u16 ff_read_file(void *buf, u16 *n);
extern u16 ff_write_mem(void *buf, u16 *n);
extern u16 ff_write_file(void *buf, u16 *n);
extern i16 g_ff_type;                   /* DS:0x26ca */
extern void *g_ff_implode;          /* DS:0x26cc */
extern void *g_ff_explode;          /* DS:0x26d0 */
extern void *g_ff_implode_b;        /* DS:0x26d4 */
extern void *g_ff_explode_b;        /* DS:0x26d8 */
extern void *g_ff_explode_to_mem;   /* DS:0x26dc */
extern void *g_ff_explode_mem;      /* DS:0x26e0 */
extern void *g_ff_work;             /* DS:0xa622 */
extern u16 g_ff_work_size;         /* DS:0xa626 */
extern i32 g_ff_out;                   /* DS:0xa628 */
extern i32 g_ff_out_left;              /* DS:0xa62c */
extern i32 g_ff_in_left;               /* DS:0xa630 */
extern i32 g_ff_in;                    /* DS:0xa634 */
extern i16 g_ff_dst_fp;                 /* DS:0xa638 */
extern FFIO g_ff_writer;                /* DS:0xa63a */
extern void *g_ff_dst;              /* DS:0xa63e */
extern i16 g_ff_src_fp;                 /* DS:0xa642 */
extern FFIO g_ff_reader;                /* DS:0xa644 */
extern void *g_ff_src;              /* DS:0xa648 */


