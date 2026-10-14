/* loader.c -- MADSPACK files: opening one and reading its packed entries.
 *
 * A MADSPACK file is an archive of up to 16 entries. It starts with a 0xb0-byte header:
 * "MADSPACK 2.0" and 0x1a, a word, the entry count, and 16 ten-byte directory entries (the
 * packing method, a flag, the unpacked and the packed size; FFEntry in loader.h). The entries'
 * data follows, one after another, in directory order.
 *
 * A FileIO (loader.h) is an open MADSPACK file: the stdio FILE, the header as it is in the file,
 * and the number of the next entry to read. loader_open opens one and reads its header,
 * loader_read reads the entries in turn, unpacking them through pack_data (pack.c) and the FAB
 * codec (fab.c), and loader_close closes it. The editor only reads MADSPACK files (its sprites
 * and fonts); opening one for writing gives an archive with no entries.
 */
#include "msc.h"
#include "fileio.h"
#include "font.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mem.h"
#include "pack.h"
#include "popup.h"
#include "records.h"
#include "sprite.h"
#include "units.h"
#pragma pack(2)   /* records on 2-byte boundaries, as in 1994: the file formats depend on it */

/* Statistics the original's loader kept. Only loader_found_on_disk is counted (one per
 * successful loader_open); the others stay 0, and nothing in the editor reads any of them. */
i32 loader_found_in_ems = 0;
i32 loader_found_in_xms = 0;
i32 loader_found_on_disk = 0;
i32 loader_timing_ems = 0;
i32 loader_timing_xms = 0;
i32 loader_timing_disk = 0;
i32 loader_size_ems = 0;
i32 loader_size_xms = 0;
i32 loader_size_disk = 0;
i16 loader_ems_search_disabled = 0;     /* not read by anything */
char loader_last[14] = "";              /* the name last given to loader_open, up to 13 chars */
/* Opens the MADSPACK file `name` (a data file name, see env_open) into fh with the fopen mode
 * `how`, which it lower-cases in place. Returns 0 when it opened, -1 when it did not.
 *
 * For reading, it reads the header's first 16 bytes, checks the magic, reads the `count`
 * directory entries, moves to the end of the header and sums the entries' unpacked sizes into
 * fh->total. The count is not checked against the 16 entries fh has room for.
 * For writing, it writes a header with no entries, with `mode` as the first entry's packing
 * method. Copying the 13-character magic and its NUL also fills the word after it, as in 1994,
 * so the header's word is 0x1a.
 *
 * A file that opens but fails the header check is left open: fh->msc_open stays 0, so
 * loader_close will not close it either. */
i16 loader_open(char *how, i16 mode, FileIO *fh, char *name)
{
    i16 reading;
    i16 err;
    i16 i;
    fpos_t pos;

    err = -1;
    msc__fstrncpy(loader_last, name, 13);
    fh->msc_open = 0;
    reading = msc_strchr(msc_strlwr(how), 'r') != 0;
    fh->kind = 0;
    if ((fh->fp = env_open(name, how)) == 0)
        goto out;
    fh->reading = reading;
    fh->next = 0;
    if (reading) {
        msc_fgetpos(fh->fp, &pos);
        if (fileio_fread_f(fh->magic, 16L, 1L, fh->fp) == 0)
            goto out;
        if (msc__fstrncmp(fh->magic, "MADSPACK 2.0\x1a", 12) != 0)
            goto out;
        if (fileio_fread_f(fh->ent, (i32)(u16)(fh->count * sizeof(FFEntry)), 1L, fh->fp) == 0)
            goto out;
        pos += 0xb0;
        msc_fsetpos(fh->fp, &pos);
        fh->total = 0;
        for (i = 0; i < fh->count; i++)
            fh->total += fh->ent[i].size;
    } else {
        fh->count = 0;
        fh->ent[0].type = (char)mode;
        msc__fstrcpy(fh->magic, "MADSPACK 2.0\x1a");
        if (fileio_fwrite_f(fh->magic, 0xb0L, 1L, fh->fp) == 0)
            goto out;
        fh->total = 0;
    }
    loader_found_on_disk++;
    fh->msc_open = -1;
    err = 0;
out:
    return err;
}
/* Sets the first directory entry's flag byte. */
void loader_set_priority(i16 flag, FileIO *fh)
{
    fh->ent[0].flag = (char)flag;
}
/* Closes fh if it is open. A file opened for writing gets its header written again first.
 * Returns 1 when that write failed, otherwise 0. */
i16 loader_close(FileIO *fh)
{
    i16 err;

    err = 0;
    if (fh->msc_open) {
        if (fh->reading == 0) {
            msc_rewind(fh->fp);
            err = fileio_fwrite_f(fh->magic, 0xb0L, 1L, fh->fp) == 0;
        }
        msc_fclose(fh->fp);
    }
    fh->msc_open = 0;
    return err;
}

/* fread for a MADSPACK file: reads fh's next entry (fh->next, which it advances), unpacked,
 * into buf. `size` times `count` is the number of bytes wanted; it returns 1 when it got `size`
 * bytes, otherwise the bytes got divided by size (0 for size 0).
 *
 * It sets pack_strategy to the entry's packing method. A stored entry (method 0) is copied
 * straight from the file. A packed one is read whole into a block from mem_get and unpacked
 * from there; when no block can be had, it is unpacked straight from the file instead, and the
 * file is then moved to the end of the entry. A failed read of the block returns 0. */
i32 loader_read(i32 size, void *buf, i32 count, FileIO *fh)
{
    i32 total;
    i16 kind2;
    void *tmp9;
    fpos_t pos8;
    i32 packed;
    i16 done4;
    i32 unused6;
    i32 got;
    register i16 i;

    tmp9 = 0;
    done4 = 0;
    if (size == 0)
        return 0;
    if (count == 1)
        total = size;
    else
        total = size * count;
    i = fh->next++;
    got = 0;
    pack_strategy = (u8)fh->ent[i].type;
    packed = fh->ent[i].packed;
    kind2 = pack_strategy == 0 ? 2 : 1;
    if (kind2 == 1) {
        tmp9 = mem_get(packed);
        if (tmp9 != 0) {
            if (fileio_fread_f(tmp9, packed, 1L, fh->fp) == 0)
                goto out;
            got = pack_data(kind2, 0, 0, total, tmp9, buf);
            done4 = -1;
        }
    }
    if (done4 == 0) {
        msc_fgetpos(fh->fp, &pos8);
        got = pack_data(kind2, 1, 0, total, (void *)fh->fp, buf);
        if (kind2 == 1)
            msc_fseek(fh->fp, packed + pos8, 0);
    }
out:
    if (tmp9)
        mem_free(tmp9);
    if (got == size)
        return 1;
    return got / size;
}
