/* From mapedit-decomp src/madsdev/buffer_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "buffer.h"
#include "loader.h"
#include "mem.h"
#include "records.h"
#include "units.h"
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

