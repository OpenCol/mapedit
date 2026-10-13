/* From mapedit-decomp src/madsdev/magic_e (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
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
/* magic_hash_color -- 133e:000a, 74 bytes */
/* PORT: magic_hash_color is in port/ (inline assembly) */


