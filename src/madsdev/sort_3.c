/* From mapedit-decomp src/madsdev/sort_3 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE sort_3: MAPEDIT.EXE segment 12e0 (sort_3.c), file 0x14408-0x144c8, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn sort_insertion_8 0x14408
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0d05).
 *
 * SORTING BYTE KEYS WITH A BYTE PAYLOAD (root code, frame 0d05). sort_insertion_8(n, idx, keys)
 * (_fastcall: n in ax, the far arrays on the stack, popped by the callee) is sort_16x16
 * (root_0cf8.c) for byte keys: the same hand-written insertion sort, byte for byte except
 * the element size. `pad` only gives the frame its 16 bytes.
 */

/* sort_insertion_8 -- 12e0:0008, 192 bytes */
/* PORT: sort_insertion_8 is in port/ (inline assembly) */
void sort_insertion_8(i16 n, char *payload, u8 *key);


