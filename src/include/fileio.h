/* fileio.h -- the functions, data and records of fileio.c, fileio_1.c, fileio_5.c, fileio_8.c, fileio_9.c, fileio_i.c, fileio_m.c, fileio_u.c. */
#ifndef MAPEDIT_FILEIO_H
#define MAPEDIT_FILEIO_H

#include "msc.h"

i16 fileio_exist(char *name);
void fileio_purge_trailing_spaces(char *s);
char * fileio_fix_lf_input(char *s);
void fileio_fix_lf_output(char *s);
i32 fileio_fread_f(void *data, i32 size, i32 n, MSC_FILE *fp);
i32 fileio_fwrite_f(void *data, i32 size, i32 n, MSC_FILE *fp);
void fileio_add_ext(char *name, char *ext);
void fileio_new_ext(char *dst, char *src, char *ext);
char * fileio_join_path(char *dst, char *dir, char *name);
void fileio_purge_all_spaces(char *s);

extern i16 fileio_suppress_unbuffering;

#endif
