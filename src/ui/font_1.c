/* From mapedit-decomp src/madsdev/font_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "menu.h"
#include "error.h"
#include "pack.h"
#include "sprite.h"
#include "map.h"
#include "popup.h"
#include "font.h"
#include "mem.h"
#include "records.h"
#include "units.h"
#include "loader.h"
/* MODULE font_1: MAPEDIT.EXE segment 0d43 (font_1.c), file 0xea3a-0xeb38, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn font_load    0x0ea3a
 *
 * Near twin: `ovl27_76c70` ff_load (0.99): the same source with the library
 * names. This module also defines the five font pointers (DS:0x3a96-0x3aa6).
 *
 * LOADING A WHOLE .FF FONT FILE INTO FAR MEMORY. font_load(name) (_fastcall, name in bx):
 *   1. Copy the name and add ".FF" when it has no extension.
 *   2. Skip a leading '*' and take the first 8 characters as the allocation's tag.
 *   3. Open the file "rb", allocate a block of the file's size under that tag
 *      (mem_get_name), and read the whole file into it (loader_read).
 *   4. On a failed read, free the block. Close the file if it was opened.
 * It returns the block, or 0. It sets mem_last_alloc_loader to 0x0f first: the module
 * number a file error is reported under.
 *
 * Global register allocation plus the global optimiser (`#pragma optimize("eg")`), as the twin.
 */

/* font_load -- 0d43:000a, 254 bytes */
void *font_load(char *name)
{
    FileIO fh;
    char path[80];
    char tag[20];
    void *result;
    i32 size;
    void *buf;
    char *p;

    buf = 0;
    result = 0;
    mem_last_alloc_loader = 0x0f;
    fh.msc_open = 0;
    msc_strcpy(path, name);
    if (msc_strchr(path, '.') == 0)
        msc_strcat(path, ".FF");
    p = path;
    if (*p == '*')
        p = path + 1;
    msc_strncpy(tag, p, 8);
    if (loader_open("rb", -1, &fh, path) == 0) {
        size = fh.ent[0].size;   /* PORT */
        buf = mem_get_name(size, tag);
        if (buf != 0) {
            if (loader_read(size, buf, 1L, &fh) != 0)
                result = buf;
        }
    }
    if (buf != 0 && result == 0)
        mem_free(buf);
    if (fh.msc_open)
        loader_close(&fh);
    return result;
}

/* The five loaded fonts (CodeView: font_1.c), defined after the last function: they land after
 * font_load's ".FF" and "rb" at DS:0x3a96-0x3aa9. */
void *font_inter = 0;                 /* DS:0x3a96 */
void *font_main = 0;                  /* DS:0x3a9a */
void *font_conv = 0;                  /* DS:0x3a9e */
void *font_menu = 0;                  /* DS:0x3aa2 */
void *font_misc = 0;                  /* DS:0x3aa6 */

/* font_string_width -- 0d6c:0002: the glyph widths of s, plus spacing after each glyph that
   has a width and is not the last character. (The original sign-extends the character, so a
   code from 80h up reads before the width table; the port reads it as unsigned.) */
i16 font_string_width(i16 spacing, DosFont *font_, char *s)
{
    const u8 *font = (const u8 *)font_;
    i16 total = 0;
    const u8 *p = (const u8 *)s;
    if (!*p)
        return 0;
    do {
        u8 w = font[2 + (u8)(*p - 1)];
        i16 c = w;
        p++;
        if (c > 0 && *p != 0)
            c = (i16)(c + spacing);
        total = (i16)(total + c);
    } while (*p);
    return total;
}
