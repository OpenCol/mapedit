/* Imported from mapedit-decomp src/madsdev/himem_4 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE himem_4: MAPEDIT.EXE segment 1345 (himem_4.c), file 0x14a5a-0x14b7e, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn himem_resident 0x14a5a
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * FINDING A FILE IN THE HIGH-MEMORY DIRECTORY (madsdev.lib). himem_resident(name) looks
 * the file up by its base name (after the last '\', else after a '*', else the whole
 * name; compared case-insensitively with each entry's name at +3) and returns its entry
 * number, selecting the entry through himem_get_directory_entry; with name NULL it
 * returns the first free entry (kind -1), which is how ems_get_page_handle takes one.
 * -1 when himem is off, nothing matches, or the directory cannot be read. An EMS directory
 * is mapped in; an XMS one is copied into a temporary "$HIMEM$" block for the search and
 * the block freed again (himem_directory points at it meanwhile).
 * Compiled with the global optimiser on (/Ox or /Oe): see the pragma. The shared tail of the
 * two match tests is cross-jumped (the first arm jumps into the second's `jnz`); a `?:` over
 * the two tests materialises a boolean instead. The module's data
 * (DS:0x4522-0x452d) is the literal and the two counters, which nothing here reads.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

// MSC: #pragma optimize("elg", on)

typedef struct {
    char kind;                  /* 3: in EMS, 4: in XMS, -1: free */
    char x1;
    u8 x2;
    char name[0x57];            /* +3 */
} HimemEntry;                   /* 0x5a bytes */

extern i16 himem_activate_directory(void);                  /* 1290:0006 */
extern i16 himem_get_directory_entry(i16 n);      /* 1292:000a */
extern void *mem_get_name(i32 size, char *tag); /* 0cc9:0136 */
extern i16 mem_free(void *p);                 /* 0cc9:0310 */
extern i16 xms_copy(i32 length, i16 src_handle, i32 src_offset, i16 dst_handle, i32 dst_offset); /* 1343:0004 */
extern char himem_directory_allocation;                     /* DS:0x44fa (HIMEM_1.C) */
extern i16 himem_directory_xms_handle;                      /* DS:0x4500 (HIMEM_1.C) */
extern i16 himem_active;                                    /* DS:0x4504 (HIMEM_1.C) */
extern HimemEntry *himem_directory;                     /* DS:0x49c8 (HIMEM_1.C) */

i16 himem_ems_preloaded = 0;                /* DS:0x452a */
i16 himem_xms_preloaded = 0;                /* DS:0x452c */
/* himem_resident -- 1345:000a, 292 bytes */
i16 himem_resident(char *name)
{
    i16 result;
    i16 i;
    char *p;
    HimemEntry *block;
    i32 size;          /* the directory's 0x34bc bytes; constant-propagated into both calls,
                           but the shipped frame has its word (declared without it the
                           frame is one word short and every slot moves) */

    result = -1;
    block = 0;
    if (himem_active) {
        p = msc__fstrrchr(name, '\\');
        if (p == 0)
            p = msc__fstrchr(name, '*');
        if (p == 0)
            p = name;
        else
            p++;
        if (himem_directory_allocation == 3) {
            if (himem_activate_directory() != 0)
                goto done;
        } else {
            size = 150 * 0x5a;
            block = mem_get_name(size, "$HIMEM$");
            himem_directory = block;
            if (block == 0)
                goto done;
            if (xms_copy(size, himem_directory_xms_handle, 0L, 0, (i32)block) != 0)
                goto done;
        }
        for (i = 0; i < 150; i++) {
            if (result >= 0)
                break;
            if (himem_directory[i].kind == -1) {
                if (name == 0)
                    result = i;
            } else {
                if (msc__fstricmp(p, himem_directory[i].name) == 0)
                    result = i;
            }
        }
    }
done:
    if (result >= 0)
        himem_get_directory_entry(result);
    if (block)
        mem_free(block);
    return result;
}

