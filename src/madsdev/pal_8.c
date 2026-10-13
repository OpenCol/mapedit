/* Imported from mapedit-decomp src/madsdev/pal_8 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE pal_8: MAPEDIT.EXE segment 0ec0 (pal_8.c), file 0x1020a-0x10258, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pal_load_2   0x1020a
 *
 * No VICEROY twin: reconstructed from the shipped bytes. "elg" (fh in di; plain flags put it in si).
 *
 * pal_load_2(name, pal) (_fastcall) reads a 768-byte palette file into the far buffer
 * through env_open; it returns 1 when the file opened and the read succeeded
 * (fileio_fread_f returning 0), else 0.
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern MSC_FILE *env_open(char *name, char *mode);
extern i32 fileio_fread_f(void *p, i32 size, i32 n, MSC_FILE *fp);
/* pal_load_2 -- 0ec0:000a, 78 bytes */
i16 pal_load_2(char *name, char *pal)
{
    i16 ok;
    MSC_FILE *fh;

    ok = 1;
    fh = env_open(name, "rb");
    if (fh != 0) {
        if (fileio_fread_f(pal, 0x300L, 1L, fh) != 0)
            ok = 0;
    }
    if (fh != 0)
        msc_fclose(fh);
    return ok;
}

