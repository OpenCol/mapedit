/* fileio.c -- files: DOS paths found in the game directory, block reads and writes, text lines.
 *
 * The small helpers the rest of the program does its file handling with. Names are DOS names;
 * the run-time (port/msc_runtime.c) looks them up in the game directory.
 *
 *   - Names: fileio_add_ext, fileio_new_ext and fileio_join_path build file names, always in
 *     upper case. env_get_path turns a data file's name into the path to open, and env_open
 *     opens it.
 *   - Blocks: fileio_fread_f and fileio_fwrite_f read and write blocks of any size. They take
 *     the stream's buffering off and go straight to the file handle, in chunks of at most
 *     0xf000 bytes.
 *   - Text lines: fileio_fix_lf_input takes off a line's '\n'; fileio_purge_trailing_spaces
 *     and fileio_purge_all_spaces trim spaces and tabs.
 *   - env_catint appends a number with leading zeros.
 */
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
#pragma pack(2)   /* records on 2-byte boundaries, as in 1994: the file formats depend on it */

/* Cuts the spaces and tabs off the end of s, one character per pass (measuring s again each
 * time), stopping at the first other character or at the start of s.
 * Port: for an empty string the original looked at the byte before s; the port returns. */
void fileio_purge_trailing_spaces(char *s)
{
    char *p;
    i16 len;
    i16 more;

    len = msc__fstrlen(s);
    if (len == 0)
        return;
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

/* Ends s at its last '\n' and returns where that was, or 0 when s has none. */
char * fileio_fix_lf_input(char *s)
{
    char *p;

    p = msc__fstrrchr(s, '\n');
    if (p)
        *p = 0;
    return p;
}

/* Nonzero makes fileio_fread_f read through fread and a 256-byte buffer instead of taking the
 * stream's buffering off. Nothing in the editor sets it. */
i16 fileio_suppress_unbuffering = 0;
/* fread for blocks of any size: reads n items of `size` bytes from fp into data and returns
 * the number of whole items read (0 for size 0).
 *
 * Normally it first takes what it can from the stream's buffer; once that is empty it turns
 * the buffering off (unless it already is) and reads the rest from the handle in chunks of up
 * to 0xf000 bytes. A chunk that comes back short does not end the loop: it runs until the
 * chunks asked for add up to the total, and only a read error stops it early. With
 * fileio_suppress_unbuffering set it reads 256-byte chunks through fread instead; a short
 * chunk ends the read, and its bytes are not counted. */
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
    return 0;
}

/* fwrite for blocks of any size: writes n items of `size` bytes from data to fp and returns
 * the number of whole items written (0 for size 0). It turns the stream's buffering off
 * (unless it is already off) and writes straight to the handle in chunks of up to 0xf000
 * bytes. Each chunk's size comes from the bytes the system reported written, while the loop
 * ends when the chunks asked for add up to the total; a write error ends it early. */
i32 fileio_fwrite_f(void *data, i32 size, i32 n, MSC_FILE *fp)
{
    char *p = data;
    i32 total;
    i32 done;
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
    return 0;
}

/* Appends "." and ext to name when name has no '.', then upper-cases name. */
void fileio_add_ext(char *name, char *ext)
{
    if (!msc__fstrrchr(name, '.')) {
        msc__fstrcat(name, ".");
        msc__fstrcat(name, ext);
    }
    msc__fstrupr(name);
}
/* Copies src to dst (unless they are the same string), drops everything from the last '.',
 * appends "." and ext, and upper-cases the result. */
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

/* Joins dir and name into dst with a '\' between them (unless dir already ends with one),
 * upper-cases the result and returns dst. An empty dir makes the test for its last character
 * read the byte before buf, in the original as in the port; nothing passes one. */
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

/* Trims s at both ends: the trailing spaces and tabs (fileio_purge_trailing_spaces), then the
 * leading ones, through a 256-byte copy. */
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

/* Appends value to s as `digits` decimal digits with leading zeros and returns s. It appends
 * `digits` '0's, then adds each digit of value into its place. A value with more digits than
 * that puts a character past '9' in the first place; a negative value leaves all zeros.
 * The loop's always-true `(u16)value >= 0` and its `i = j` copy amount to an ordinary
 * `i < digits; i++`. */
char *env_catint(char *s, i16 value, i16 digits)
{
    i16 i;
    i16 pow = 1;
    i16 digit;
    i16 j;

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
/* Builds a data file's path in dst and returns dst. A leading '*' on name is dropped; the
 * name is then used as it is, or joined to env_path when env_special_path is set. */
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
/* Opens a data file (the path from env_get_path) with fopen's mode; NULL when it fails. */
MSC_FILE *env_open(char *name, char *mode)
{
    char path[80];

    env_get_path(path, name);
    return msc_fopen(path, mode);
}

i16 env_special_path = 0;               /* nonzero: data files are under env_path (never set) */

char env_path[60];                      /* the data files' directory, with env_special_path */

