/* loader.h -- MADSPACK files: opening one and reading its entries in turn.
 *
 * Defined in sys/loader.c. A MADSPACK file is an archive of up to 16 entries, each stored or
 * packed with the FAB codec (pack.h). It opens with a 0xb0-byte header: the magic
 * "MADSPACK 2.0\x1a" and its terminating 0, the entry count, and 16 ten-byte directory
 * entries. loader_open reads that header into FileIO from `magic` on, and loader_close of a
 * file opened for writing writes it back from there, so that part of the record keeps the
 * file's layout. loader_read then unpacks the entries one after another, in order. The
 * sprite series (.SS) and the other art the editor loads are MADSPACK files.
 */
#ifndef MAPEDIT_LOADER_H
#define MAPEDIT_LOADER_H

#include "msc.h"

#pragma pack(push, 2)
typedef struct {                /* a directory entry: 10 bytes in the file */
    char type;                  /* the packing method: 0 stored, 1 FAB */
    char flag;                  /* set by loader_set_priority; not read */
    i32 size;                   /* bytes when unpacked */
    i32 packed;                 /* bytes in the file */
} FFEntry;

typedef struct {
    i16 msc_open;               /* nonzero while the file is open */
    i16 reading;                /* opened for reading */
    char kind;                  /* where the file is: always 0, on disk */
    MSC_FILE *fp;
    i32 total;                  /* the entries' unpacked sizes summed */
    i16 next;                   /* the entry loader_read reads next */
    char magic[12];             /* the file's header starts here */
    i16 x26;                    /* the magic's last byte, 1Ah, and its 0: the magic string
                                   is copied over magic[] and runs on into this word */
    i16 count;                  /* entries in the file */
    FFEntry ent[16];
} FileIO;
#pragma pack(pop)

i16 loader_open(char *how, i16 mode, FileIO *fh, char *name);
i16 loader_close(FileIO *fh);
i32 loader_read(i32 size, void *buf, i32 count, FileIO *fh);
void loader_set_priority(i16 flag, FileIO *fh);

#endif
