/* fileio.c -- files: DOS paths found in the game directory, block reads and writes, text lines. */
#include "msc.h"
#include "fileio.h"
#include "font.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "units.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */

/* MODULE fileio_1: MAPEDIT.EXE segment 101f (FILEIO_1.C), file 0x117fc-0x1185c, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_purge_trailing_spaces 0x117fc
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_1094).
 *
 * TRIMMING TRAILING BLANKS (root code past the C runtime, frame 1094; pascal, "el": with /Og
 * MSC turns the end test into `dec di; cmp di,s`, shipped keeps p and tests `lea ax,[di-1]`).
 * fileio_purge_trailing_spaces(s) cuts spaces and tabs off the end of s, one per pass, re-measuring s each
 * time, and stops at the first other character or at the start of s. Its first _fstrlen
 * is a dead length: the call stays, the store does not.
 */


/* fileio_purge_trailing_spaces -- 101f:000c, 96 bytes */
void fileio_purge_trailing_spaces(char *s)
{
    char *p;
    i16 len;
    i16 more;

    len = msc__fstrlen(s);
    if (len == 0)
        return;     /* 1994 looked at the byte before an empty string; nothing to purge */
    do {
        more = -1;
        p = &s[msc__fstrlen(s) - 1];
        if (*p == ' ' || *p == '\t')
            *p = 0;
        else
            more = 0;
        if (p - 1 < s)
            more = 0;
    } while (more);
}

/* MODULE fileio_5: MAPEDIT.EXE segment 0bc4 (FILEIO_5.C), file 0xd240-0xd2a0, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_fix_lf_input  0x0d240
 * fn fileio_fix_lf_output 0x0d26e
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0afb).
 *
 * NEWLINES IN TEXT LINES (root code, frame 0afb; pascal, "elg").
 *   - fileio_fix_lf_input(s) ends s at its last '\n' and returns where that was (0 if none).
 *   - fileio_fix_lf_output(s) appends "\n".
 */


/* fileio_fix_lf_input -- 0bc4:0000, 46 bytes */
char * fileio_fix_lf_input(char *s)
{
    char *p;

    p = msc__fstrrchr(s, '\n');
    if (p)
        *p = 0;
    return p;
}
/* fileio_fix_lf_output -- 0bc4:002e, 50 bytes */
void fileio_fix_lf_output(char *s)
{
    char *t;

    t = s + msc__fstrlen(s);
    *t++ = '\n';
    *t = 0;
}

/* MODULE fileio_8: MAPEDIT.EXE segment 0bca (fileio_8.c), file 0xd2a0-0xd4a8, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_fread_f 0x0d2a0
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b01).
 *
 * READING INTO A HUGE BUFFER (root code, frame 0b01; "elg"). fileio_fread_f(p, size, n, fp)
 * (_fastcall: size in dx:ax, fp in bx, p and n on the stack) reads size*n bytes into p and
 * returns the number of whole items read. With fileio_suppress_unbuffering clear it empties the stream's buffer
 * first, turns buffering off, and reads the rest with _dos_read in chunks of up to 0xf000;
 * with it set it goes through fread and a 256-byte buffer instead.
 */



i16 fileio_suppress_unbuffering = 0;                          /* DS:0x0718 (defined here) */
/* fileio_fread_f -- 0bca:0000, 520 bytes */
i32 fileio_fread_f(void *data, i32 size, i32 n, MSC_FILE *fp)
{
    char *p = data;
    i32 total;
    i32 got;
    i32 done;
    u16 nread;
    char buf[256];
    u16 chunk;

    got = done = 0;
    if (size == 0)
        goto out;
    total = n == 1 ? size : n * size;
    if (fileio_suppress_unbuffering == 0) {
        if (fp->_cnt > 0) {
            chunk = fp->_cnt > total ? total : fp->_cnt;
            msc__fmemcpy(p, fp->_ptr, chunk);
            fp->_ptr += chunk;
            fp->_cnt -= chunk;
            got = done = chunk;
            p += chunk;
        }
        if (fp->_cnt == 0 && !(fp->_flag & 4))
            msc_setbuf(fp, 0);
        while (got < total) {
            chunk = total - done > 0xf000 ? 0xf000 : total - done;
            if (msc__dos_read(fp->_file, p, chunk, &nread))
                break;
            done += nread;
            p += nread;
            got += chunk;
        }
    } else {
        while (got < total) {
            chunk = total - done > 0x100 ? 0x100 : total - done;
            if (msc_fread(buf, chunk, 1, fp) == 0)
                break;
            msc__fmemcpy(p, buf, chunk);
            done += chunk;
            p += chunk;
            got += chunk;
        }
    }
    if (size == done)
        return 1;
    return done / size;
out:
    return 0;   /* PORT: size 0; the shipped code arrives here with dx:ax = size = 0 */
}

/* MODULE fileio_9: MAPEDIT.EXE segment 0bea (fileio_9.c), file 0xd4a8-0xd5b0, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_fwrite_f 0x0d4a8
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl28_775ec).
 *
 * WRITING A FAR BLOCK TO A FILE (overlay 28, para 121: its own segment, 0x775e0-0x776f0).
 * DOS-only engine code.
 *   - fileio_fwrite_f(p, size, n, fp) (_fastcall, thunk 181f:2c9c) writes n items of `size`
 *     bytes from a far block. It makes the FILE unbuffered first (setbuf(fp, 0) unless
 *     _IONBF is set), then writes in chunks of at most 0xf000 bytes straight to the handle
 *     with _dos_write (0d1d:0e9d), advancing p as a HUGE pointer. Each chunk is
 *     min(total - written, 0xf000), where `written` counts the bytes DOS reported, while
 *     the loop counter `done` adds the requested chunk. A _dos_write error ends the loop.
 *     It returns 1 when written == size, otherwise written / size (so, like ff_read, the
 *     count is right only for n == 1 or a complete write); size 0 returns 0 by falling off
 *     the end (ax = dx = 0 from the test).
 *
 * `#pragma optimize("elg")`. The pointer arithmetic is MSC's own huge-pointer code
 * (`sub bx,bx; add off,ax; adc bx,dx; mov cx,__AHSHIFT; shl bx,cl; add seg,bx`): the
 * parameter is `char huge *p` and the statement is `p += got`. The `mov cx,0xc` immediate is
 * the __AHSHIFT fixup, which the linker resolves to 12; a literal `<< 12` gives MSC's byte
 * trick (mov dh,dl; shl dh,4) instead. Ten pads set the _aFlmul push order (n first);
 * the `written == size` operand order then needs ten unused locals before `written`
 * (the only working cell of a 17 x 16 sweep of pads x unused locals).
 */




/* fileio_fwrite_f -- 0bea:0008, 264 bytes */
i32 fileio_fwrite_f(void *data, i32 size, i32 n, MSC_FILE *fp)
{
    char *p = data;
    i32 total;
    i32 done;
    i16 unused_1, unused_2, unused_3, unused_4, unused_5;
    i16 unused_6, unused_7, unused_8, unused_9, unused_10;
    i32 written;
    u16 got;
    u16 chunk;

    written = 0;
    done = 0;
    if (size != 0) {
        if (!(fp->_flag & 4))
            msc_setbuf(fp, 0);
        total = n == 1 ? size : n * size;
        while (done < total) {
            chunk = (u16)(total - written > 0xf000 ? 0xf000 : total - written);
            if (msc__dos_write(fp->_file, p, chunk, &got) != 0)
                break;
            written += got;
            p += got;
            done += chunk;
        }
        if (written == size)
            return 1;
        return written / size;
    }
    return 0;   /* PORT: size 0; the shipped code arrives here with dx:ax = size = 0 */
}

/* MODULE fileio_i: MAPEDIT.EXE segment 0c04 (FILEIO_I.C), file 0xd64e-0xd714, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_add_ext 0x0d64e
 * fn fileio_new_ext 0x0d69c
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b32).
 *
 * FILE NAME EXTENSIONS (root code, frame 0b32; pascal, "elg").
 *   - fileio_add_ext(name, ext) appends "." and ext when name has no '.', then upper-cases it.
 *   - fileio_new_ext(dst, src, ext) copies src to dst (unless they are the same), drops
 *     anything from the last '.', appends "." and ext, and upper-cases the result.
 */


/* fileio_add_ext -- 0c04:000e, 78 bytes */
void fileio_add_ext(char *name, char *ext)
{
    if (!msc__fstrrchr(name, '.')) {
        msc__fstrcat(name, ".");
        msc__fstrcat(name, ext);
    }
    msc__fstrupr(name);
}
/* fileio_new_ext -- 0c04:005c, 120 bytes */
void fileio_new_ext(char *dst, char *src, char *ext)
{
    char *p;

    if (dst != src)
        msc__fstrcpy(dst, src);
    p = msc__fstrrchr(dst, '.');
    if (p)
        *p = 0;
    msc__fstrcat(dst, ".");
    msc__fstrcat(dst, ext);
    msc__fstrupr(dst);
}

/* MODULE fileio_m: MAPEDIT.EXE segment 0c11 (FILEIO_M.C), file 0xd714-0xd7a2, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_join_path 0x0d714
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b4e).
 *
 * JOINING A PATH (root code, frame 0b4e; pascal, "elg"). fileio_join_path(dst, dir, name) copies
 * dir, adds a '\' unless it already ends with one, appends name, upper-cases the result in
 * dst and returns dst.
 */


/* fileio_join_path -- 0c11:0004, 142 bytes */
char * fileio_join_path(char *dst, char *dir, char *name)
{
    char buf[80];
    char *p;

    msc__fstrcpy(buf, dir);
    for (p = buf; *p; p++)
        ;
    if (p[-1] != '\\')
        msc_strcat(buf, "\\");
    msc__fstrcpy(dst, buf);
    msc__fstrcat(dst, name);
    msc__fstrupr(dst);
    return dst;
}

/* MODULE fileio_u: MAPEDIT.EXE segment 0c1a (FILEIO_U.C), file 0xd7a2-0xd810, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_purge_all_spaces 0x0d7a2
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b57).
 *
 * TRIMMING A LINE (root code, frame 0b57; pascal, "elg"). fileio_purge_all_spaces(s) trims
 * trailing blanks (fileio_purge_trailing_spaces, 1094:000a, which walks back over spaces and tabs) and then
 * leading spaces and tabs, through a 256-byte copy. The skip loop runs with DS on s's
 * segment and reloads DGROUP afterwards: MSC's code for a far pointer walked in a loop.
 */


/* fileio_purge_all_spaces -- 0c1a:0002, 110 bytes */
void fileio_purge_all_spaces(char *s)
{
    char buf[256];
    char *p;

    fileio_purge_trailing_spaces(s);
    p = s;
    while (*p && (*p == ' ' || *p == '\t'))
        p++;
    msc__fstrcpy(buf, p);
    msc__fstrcpy(s, buf);
}

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

/* fileio.c -- fileio_c.ASM's fileio_exist, and env_1.c's env_exist, which only jumps to it.
 *
 * fileio_exist builds the data file's path (env_get_path) and opens it for reading, with its
 * own critical-error handler in place so a missing floppy fails instead of asking Abort,
 * Retry, Fail; -1 when it opened. */


i16 fileio_exist(char *name)
{
    char copy[80], path[0x51];
    i16 h;
    u16 i;
    for (i = 0; i < sizeof copy - 1 && name[i]; i++)
        copy[i] = name[i];
    copy[i] = 0;
    env_get_path(path, copy);
    h = msc_open(path, 0);
    if (h < 0)
        return 0;
    msc_close(h);
    return -1;
}

/* env_exist -- 0297:0142 */
i16 env_exist(char *name)
{
    return fileio_exist(name);
}
