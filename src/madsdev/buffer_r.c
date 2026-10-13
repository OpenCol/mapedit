/* Imported from mapedit-decomp src/madsdev/buffer_r by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_r: MAPEDIT.EXE segment 1025 (buffer_r.c), file 0x1185c-0x119da, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_to_disk   0x1185c
 * fn buffer_from_disk 0x1193a
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bdd).
 *
 * SAVING WHAT A WINDOW COVERS TO DISK (root code, frame 0bdd). Backs port_save_under /
 * port_restore_under (ovl31) when there is no memory for a scratch port.
 *   - buffer_to_disk(port, x, y, w, h) (_fastcall) takes the next free one of ten slots
 *     (round robin), writes the rectangle row by row to "$MPOP.$<slot>", and returns the
 *     slot, or -1.
 *   - buffer_from_disk(port, slot, keep, x, y, w, h) (_fastcall) reads it back, then deletes
 *     the file and frees the slot unless `keep`.
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern char *buffer_pointer(DosPort *port, i16 x, i16 y);
extern char *env_catint(char *s, i16 value, i16 digits);
extern i32 fileio_fwrite_f(void *p, i32 size, i32 n, MSC_FILE *fp);
extern i32 fileio_fread_f(void *p, i32 size, i32 n, MSC_FILE *fp);

static char g_swap_wb[] = "wb";          /* DS:0x43e8: a module static (no public) */
char buffer_disk_filename[] = "$MPOP.$";          /* DS:0x2634 */
u8 buffer_accum = 0;           /* DS:0x263c */
u8 buffer_tracking[10] = { 0 };   /* DS:0x263e */
static char g_swap_rb[] = "rb";          /* DS:0x4400: a module static (no public) */
/* buffer_to_disk -- 1025:000c, 222 bytes */
i16 buffer_to_disk(DosPort *port, i16 x, i16 y, i16 w, i16 h)
{
    char name[40];
    i16 result;
    MSC_FILE *fp;
    i16 found;
    i16 tries;
    i16 i;

    result = -1;
    fp = 0;
    tries = 0;
    found = 0;
    do {
        buffer_accum = (u8)((buffer_accum + 1) % 10);
        msc_strcpy(name, buffer_disk_filename);
        env_catint(name, buffer_accum, 1);
        if (buffer_tracking[buffer_accum] == 0)
            found = -1;
        else if (++tries > 10)
            goto out;
    } while (!found);
    buffer_tracking[buffer_accum] = 0xff;
    fp = msc_fopen(name, g_swap_wb);
    if (fp) {
        for (i = 0; i < h; i++)
            if (fileio_fwrite_f(buffer_pointer(port, x, y + i), (i32)w, 1L, fp) == 0)
                goto out;
        result = buffer_accum;
    }
out:
    if (fp)
        msc_fclose(fp);
    return result;
}
/* buffer_from_disk -- 1025:00ea, 160 bytes */
void buffer_from_disk(DosPort *port, i16 slot, i16 keep, i16 x, i16 y, i16 w, i16 h)
{
    char name[40];
    MSC_FILE *fp;
    i16 i;

    msc_strcpy(name, buffer_disk_filename);
    env_catint(name, slot, 1);
    fp = msc_fopen(name, g_swap_rb);
    if (fp)
        for (i = 0; i < h; i++)
            if (fileio_fread_f(buffer_pointer(port, x, y + i), (i32)w, 1L, fp) == 0)
                break;
    if (fp) {
        msc_fclose(fp);
        if (!keep) {
            buffer_tracking[slot] = 0;
            msc_remove(name);
        }
    }
}

