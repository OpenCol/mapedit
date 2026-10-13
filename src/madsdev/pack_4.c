/* From mapedit-decomp src/madsdev/pack_4 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "fileio.h"
#include "loader.h"
#include "pack.h"
#include "records.h"
#include "units.h"
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



#define min(a, b) (((a) < (b)) ? (a) : (b))     /* <stdlib.h>'s */
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

