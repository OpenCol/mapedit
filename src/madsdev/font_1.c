/* Imported from mapedit-decomp src/madsdev/font_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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
// MSC: #pragma optimize("eg", on)

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 msc_open; char pad[0x2a]; i32 size; char rest[0x9a]; } Loader;

extern i16 loader_open(char *how, i16 mode, Loader *fh, char *name);
extern i32 loader_read(i32 size, void *dst, i32 count, Loader *fh);
extern void loader_close(Loader *fh);
extern void *mem_get_name(i32 size, char *tag);
extern void mem_free(void *p);
extern i16 mem_last_alloc_loader;                                            /* DS:0x074c */
/* font_load -- 0d43:000a, 254 bytes */
void *font_load(char *name)
{
    Loader fh;
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
        size = fh.size;
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

