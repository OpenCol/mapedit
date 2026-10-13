/* Imported from mapedit-decomp src/madsdev/pack_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE pack_1: MAPEDIT.EXE segment 1359 (PACK_1.C), file 0x14b96-0x14c0a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_read_memory 0x14b96
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_1066).
 *
 * THE MEMORY READER FOR IMPLODE/EXPLODE (root code past the C runtime, frame 1066; pascal).
 * pack_read_memory(buf, n) copies up to *n bytes from pack_read_memory_ptr into buf, capped by pack_read_size
 * unless its high word is -1 (unlimited); a zero allowance returns 0. It advances pack_read_memory_ptr
 * and normalises it with mem_normalize, and counts the bytes into pack_read_count. The body is an
 * `_asm` block (the `push ax; or ax,dx; pop ax` zero test and `rep movsb` are not MSC's).
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern i32 pack_read_size;                /* DS:0xa630 */
extern i32 pack_read_count;                     /* DS:0xa634 */
extern char *pack_read_memory_ptr;               /* DS:0xa648 */
extern char * mem_normalize(char *p);
/* pack_read_memory -- 1359:0006, 116 bytes */
/* PORT: pack_read_memory is in port/ (inline assembly) */
u16 pack_read_memory(char *buf, u16 *n);


/* The module's uninitialised data (communal, BSS). */
char *pack_read_memory_ptr;           /* DS:0x6348 */

