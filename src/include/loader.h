/* loader.h -- MADSPACK files: the loader's open-file record (loader_1/2.c, sprite_e.c).
 *
 * A MADSPACK file starts with a 0xb0-byte header -- "MADSPACK 2.0\x1a", a word, the entry
 * count and 16 directory entries -- that loader_open reads into FileIO.magic onwards and
 * loader_close writes back from there, so that part keeps the file's layout.
 */
#ifndef MAPEDIT_LOADER_H
#define MAPEDIT_LOADER_H

#include "msc.h"

#pragma pack(push, 2)
typedef struct {                /* a directory entry: 10 bytes in the file */
    char type;                  /* the packing method: 0 stored, 1 FAB */
    char flag;                  /* loader_set_priority's */
    i32 size;                   /* bytes when unpacked */
    i32 packed;                 /* bytes in the file */
} FFEntry;

typedef struct {
    i16 msc_open;               /* nonzero while open */
    i16 reading;
    char kind;                  /* where the file is: always 0, on disk */
    MSC_FILE *fp;
    i32 total;                  /* the entries' sizes summed */
    i16 next;                   /* the entry loader_read reads next */
    char magic[12];             /* the 0xb0-byte header starts here */
    i16 x26;
    i16 count;
    FFEntry ent[16];
} FileIO;
#pragma pack(pop)

i16 loader_open(char *how, i16 mode, FileIO *fh, char *name);
i16 loader_close(FileIO *fh);
i32 loader_read(i32 size, void *buf, i32 count, FileIO *fh);
void loader_set_priority(i16 flag, FileIO *fh);

#endif
