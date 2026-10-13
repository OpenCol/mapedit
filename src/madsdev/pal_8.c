/* From mapedit-decomp src/madsdev/pal_8 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "fileio.h"
#include "loader.h"
#include "mapedit.h"
#include "pal.h"
#include "units.h"
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

