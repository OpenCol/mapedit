/* From mapedit-decomp src/madsdev/fileio_8 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "pal.h"
#include "popup.h"
#include "fileio.h"
#include "loader.h"
#include "records.h"
#include "units.h"
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

