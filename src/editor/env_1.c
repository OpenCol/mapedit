/* From mapedit-decomp src/editor/env_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "fileio.h"
#include "loader.h"
#include "mapedit.h"
#include "units.h"
/* MODULE env_1: MAPEDIT.EXE segment 0297 (env_1.obj), file 0x3f7c-0x40be, 5 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn env_catint        0x03f7c
 * fn env_get_path      0x0402a
 * fn env_open          0x04074
 * fn env_get_file_size 0x040a0
 * fn env_exist         0x040b2
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_09f6).
 *
 * NUMBERS AND FILE NAMES (root code, frame 09f6).
 *   - env_catint(s, value, digits) (_fastcall; Windows twin) appends value as
 *     `digits` zero-padded digits.
 *   - env_get_path(dst, name) builds a data-file path: a leading '*' is dropped; the name is
 *     copied as is, or joined with env_path when env_special_path is set (pascal 0b4e:0004).
 *   - env_open(name, mode) (_fastcall) opens the built path with fopen.
 *   - env_exist(name) (_fastcall) calls the assembly routine fileio_exist (0b22:0022), which reads the
 *     name from bx where env_exist received it.
 *
 * "elg" throughout. env_exist is an inline-asm call: inline assembly makes MSC home the
 * register argument (push bx) even though the C code never reads it.
 * Two levers, both needed, none of them in the Windows twin's text:
 *   1. The loop-1 update is a COPY from a temp (`i = j` in the for clause, `j = i + 1` at the
 *      body end). That takes the loop out of the "l" induction-variable pass, which otherwise
 *      regenerates the bottom test as memory-first (`cmp [digits],si; jg`) whatever the source
 *      says; with the copy, si holds a value the statement just computed and the test is the
 *      shipped `inc si; cmp si,[digits]; jl`. (`i++`, `i += 1`, `i = i + 1`, `i = -~i`, every
 *      while/do/goto/exit layout, and ~130 compare spellings stay IV'd.)
 *   2. Alone, the copy form flips loop 1's si/di pairing (i di, pow si; 9 bytes): without the
 *      IV pass every reference of i counts in the "elg" weights. An always-false use of VALUE
 *      inside loop 1 (`&& (unsigned)value >= 0` in the for test, or in the `if (i > 0)`) puts
 *      the pairing back (i si, pow di) and leaves loop 2 (pow si, value di, i in memory) as it
 *      was. The same term on i fixes loop 1 but flips loop 2 (13); on pow, twice on pow, or
 *      `if (0) x = 0;` on any of i/pow/j: nothing; on j: 102; on digits: 150; on digit/digits
 *      in loop 2: 13; on value in loop 2: 22. Register tuple (pow1,i1,i2,pow2,value2) per variant:
 *      base di,si,si,si,di (2); copy si,di,si,si,di (9); copy+i-term di,si,di,di,si (13);
 *      copy+value-term in loop 1 di,si,si,si,di (0). A block-scope ISOLATE on i/pow/digit/j
 *      (k = 1..16) never moves the pairing (pow k = 11,12 flips `value >= pow` instead); nor do
 *      per-function pragma letters, a fresh loop-2 counter (k or j: the slot order of pow and
 *      the counter swaps, 15), or `|| 0` on any test. `digit` as the temp instead of `j` also
 *      matches (no new symbol); j was kept for readability. Earlier passes' lists of what does
 *      not work are in git history (this note as of 100376b1a).
 */



/* env_catint -- 0297:000c, 174 bytes */
char *env_catint(char *s, i16 value, i16 digits)
{
    i16 i;
    i16 pow = 1;
    i16 digit;
    i16 j;

    /* Byte-match shape (see the header): the update is a copy from a temp, and the
     * always-false `value` term keeps value live across the loop.  Logic is the
     * Windows twin's `for (i = 0; i < digits; i++)`. */
    for (i = 0; i < digits && (u16)value >= 0; i = j) {
        if (i > 0)
            pow *= 10;
        msc__fstrcat(s, "0");
        j = i + 1;
    }
    i = msc__fstrlen(s) - digits;
    while (msc__fstrlen(s) > i) {
        if (value >= pow) {
            digit = value / pow;
            value = value - digit * pow;
            s[i] += digit;
        }
        pow = pow / 10;
        i++;
    }
    return s;
}
/* env_get_path -- 0297:00ba, 74 bytes */
char *env_get_path(char *dst, char *name)
{
    if (*name == '*')
        name++;
    if (env_special_path == 0)
        msc__fstrcpy(dst, name);
    else
        fileio_join_path(dst, env_path, name);
    return dst;
}
/* env_open -- 0297:0104, 44 bytes */
MSC_FILE *env_open(char *name, char *mode)
{
    char path[80];

    env_get_path(path, name);
    return msc_fopen(path, mode);
}
/* env_get_file_size -- 0297:0130, 18 bytes */
i32 env_get_file_size(MSC_FILE *fp)
{
    return msc_filelength(fp->_file);
}
/* env_exist -- 0297:0142, 12 bytes */
/* PORT: env_exist is in port/ (inline assembly) */


/* DGROUP data (DS:0x036a-0x036d), after env_catint's "0", in address order after the last function (so no
 * function's symbol count moves; the externs above stay as they were). Shipped values. */
i16 env_search_mode = 0;                                     /* DS:0x036a */
i16 env_special_path = 0;                                /* DS:0x036c */

/* The module's uninitialised data (communal, BSS): 60 bytes to the next communal. */
char env_path[60];                        /* DS:0x4eaa */

