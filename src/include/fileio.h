/* fileio.h -- files: data file paths, block reads and writes, text line fix-ups.
 *
 * Defined in sys/fileio.c (the env_* path functions are declared in mapedit.h). The block
 * functions read and write size * n bytes through MSC's FILE record, as the 1994 library did;
 * the others are string helpers for file names and text lines.
 */
#ifndef MAPEDIT_FILEIO_H
#define MAPEDIT_FILEIO_H

#include "msc.h"

void fileio_purge_trailing_spaces(char *s);
char * fileio_fix_lf_input(char *s);
i32 fileio_fread_f(void *data, i32 size, i32 n, MSC_FILE *fp);
i32 fileio_fwrite_f(void *data, i32 size, i32 n, MSC_FILE *fp);
void fileio_add_ext(char *name, char *ext);
void fileio_new_ext(char *dst, char *src, char *ext);
char * fileio_join_path(char *dst, char *dir, char *name);
void fileio_purge_all_spaces(char *s);

extern i16 fileio_suppress_unbuffering;  /* nonzero: block reads go through fread instead */

#endif
