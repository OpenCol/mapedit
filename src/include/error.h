/* error.h -- the functions, data and records of error_1.c, misc.c, rest.c. */
#ifndef MAPEDIT_ERROR_H
#define MAPEDIT_ERROR_H

#include "msc.h"

void error_dump_file(char *name);
i16 error_read_line(char *dest, char *name, i16 n);
void error_report(i16 code, i16 level, i16 module, i32 data1, i32 data2);

extern volatile i16 error_abort;
extern void (*error_service_routine)(void);
extern void (*error_service_routine_2)(void);
extern char error_string[80];
extern i16 exit_value;
void error_shutdown_report(char *errtext, char *modtext, char *numtext, char *extra, i32 memfree, i16 code);
void echo(i16 kind, char *text);

#endif
