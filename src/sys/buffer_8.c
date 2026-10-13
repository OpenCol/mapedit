/* From mapedit-decomp src/madsdev/buffer_8 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "menu.h"
#include "popup.h"
#include "buffer.h"
#include "loader.h"
/* MODULE buffer_8: MAPEDIT.EXE segment 0c78 (buffer_8.c), file 0xdd80-0xdd96, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_get   0x0dd80
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bbb).
 *
 * READING ONE BYTE FROM A PORT (root code, frame 0bbb). buffer_get(x, y, port) (_fastcall).
 */



/* buffer_get -- 0c78:0000, 22 bytes */
u8 buffer_get(i16 x, i16 y, DosPort port)
{
    return *buffer_pointer(&port, x, y);
}

