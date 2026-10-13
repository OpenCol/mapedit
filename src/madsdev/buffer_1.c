/* Imported from mapedit-decomp src/madsdev/buffer_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_1: MAPEDIT.EXE segment 0c3e (buffer_1.c), file 0xd9ea-0xda66, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_init      0x0d9ea
 * fn buffer_init_name 0x0da24
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl31_787a2).
 *
 * CREATING A DRAWING PORT (overlay 31, para 22). DOS-only engine code; both functions are
 * _fastcall, with the port in bx, w in ax and h in dx.
 *   - buffer_init allocates w * h bytes as one far block (181f:029a) for port->bits, and
 *     fills in w and h. It returns -1, or 0 when the allocation fails.
 *   - buffer_init_name does the same through the tagged allocator (181f:2e90), under the
 *     tag string the caller passes on the stack.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern void *mem_get(i32 size);
extern void *mem_get_name(i32 size, char *tag);
/* buffer_init -- 0c3e:000a, 58 bytes */
i16 buffer_init(DosPort *port, u16 w, u16 h)
{
    port->bits = mem_get((u32)h * w);
    if (port->bits == 0)
        return 0;
    port->w = w;
    port->h = h;
    return -1;
}
/* buffer_init_name -- 0c3e:0044, 66 bytes */
i16 buffer_init_name(DosPort *port, u16 w, u16 h, char *tag)
{
    port->bits = mem_get_name((u32)h * w, tag);
    if (port->bits == 0)
        return 0;
    port->w = w;
    port->h = h;
    return -1;
}

