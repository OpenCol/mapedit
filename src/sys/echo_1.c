/* From mapedit-decomp src/madsdev/echo_1 (f55aff8), now maintained here: docs/origin.md. */
#include <stdio.h>
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "menu.h"
#include "error.h"
#include "pack.h"
#include "popup.h"
#include "sprite.h"
#include "font.h"
#include "loader.h"
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

/* echo -- 127b:0000, 60 bytes */
/* PORT: echo is in port/ (inline assembly) */

/* echo -- 127b:0000: text to the console (DOS handle 1), and LF CR after it if kind != 0 */
void echo(i16 kind, char *text)
{
    fputs(text, stdout);
    if (kind)
        fputs("\n", stdout);
    fflush(stdout);
}
