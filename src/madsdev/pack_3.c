/* From mapedit-decomp src/madsdev/pack_3 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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



extern i32 pack_read_size;                /* DS:0xa630 */
extern i32 pack_read_count;                     /* DS:0xa634 */
extern MSC_FILE *pack_read_file_handle;                /* DS:0xa642 */
extern i32 fileio_fread_f(char *p, i32 size, i32 n, MSC_FILE *fp);
#define min(a, b) (((a) < (b)) ? (a) : (b))     /* <stdlib.h>'s */
/* pack_read_file -- 1367:0008, 122 bytes */
u16 pack_read_file(char *buf, u16 *n)
{
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

