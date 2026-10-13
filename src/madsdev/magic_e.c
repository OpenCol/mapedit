/* Imported from mapedit-decomp src/madsdev/magic_e by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE magic_e: MAPEDIT.EXE segment 133e (magic_e.c), file 0x149ea-0x14a34, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn magic_hash_color 0x149ea
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * magic_hash_color(rgb) returns r*38 + g*76 + b*14 for a far RGB triple: a luminance-like
 * hash (the weights sum to 128). Hand-written: shifts and adds in cx/si, `xor` for the
 * zeroes (MSC's own zero is `sub`).
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;
/* magic_hash_color -- 133e:000a, 74 bytes */
/* PORT: magic_hash_color is in port/ (inline assembly) */
i16 magic_hash_color(u8 *rgb);


