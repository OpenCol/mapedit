/* Imported from mapedit-decomp src/madsdev/pack_2 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE pack_2: MAPEDIT.EXE segment 1360 (PACK_2.C), file 0x14c0a-0x14c78, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_write_memory 0x14c0a
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_106d).
 *
 * THE MEMORY WRITER FOR IMPLODE/EXPLODE (root code past the C runtime, frame 106d; pascal).
 * pack_write_memory(buf, n) is ff_read_mem (root_1066.c) the other way round: up to *n bytes of
 * buf into pack_write_memory_ptr, capped by pack_write_size unless its high word is -1, counted into
 * pack_write_count, pack_write_memory_ptr advanced and normalised. It has no return statement: it returns what
 * mem_normalize left in ax (the new offset), not the count -- the shipped code keeps no
 * local for it.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern i32 pack_write_count;                    /* DS:0xa628 */
extern i32 pack_write_size;               /* DS:0xa62c */
extern char *pack_write_memory_ptr;               /* DS:0xa63e */
extern char * mem_normalize(char *p);
/* pack_write_memory -- 1360:000a, 110 bytes */
/* PORT: pack_write_memory is in port/ (inline assembly) */
u16 pack_write_memory(char *buf, u16 *n);


