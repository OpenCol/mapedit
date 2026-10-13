/* Imported from mapedit-decomp src/madsdev/echo_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE echo_1: MAPEDIT.EXE segment 127b (ECHO_1.C), file 0x13db0-0x13dec, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn echo         0x13db0
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_1088).
 *
 * TEXT TO STANDARD OUTPUT (root code past the C runtime, frame 1088; _fastcall).
 * echo(kind, text) writes text to handle 1 with DOS function 40h and, when kind is
 * not 0, ends the line with LF then CR (in that order) through function 2. The memory
 * report (ovl29_77990.c) prints with it. All but the _fstrlen is an `_asm` block.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

/* echo -- 127b:0000, 60 bytes */
/* PORT: echo is in port/ (inline assembly) */
void echo(i16 kind, char *text);


