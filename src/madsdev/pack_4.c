/* Imported from mapedit-decomp src/madsdev/pack_4 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern i32 pack_write_count;                    /* DS:0xa628 */
extern i32 pack_write_size;               /* DS:0xa62c */
extern MSC_FILE *pack_write_file_handle;                /* DS:0xa638 */
extern i32 fileio_fwrite_f(char *p, i32 size, i32 n, MSC_FILE *fp);
#define min(a, b) (((a) < (b)) ? (a) : (b))     /* <stdlib.h>'s */
/* pack_write_file -- 136f:0002, 146 bytes */
u16 pack_write_file(char *buf, u16 *n)
{
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

