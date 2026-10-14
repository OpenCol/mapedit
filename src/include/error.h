/* error.h -- error reports, the fatal shutdown, console output.
 *
 * Defined in sys/error.c. error_report is how the program fails: when the error's level
 * reaches error_abort it looks the code up in ERRORS.DB and the module number in MODULES.DB,
 * and error_shutdown_report shuts the clock, the keyboard and the mouse down, prints the
 * report and the memory figures (or WARN6.DAT for code -20), runs the service routines and
 * exits with status 3. echo writes a line to standard output.
 */
#ifndef MAPEDIT_ERROR_H
#define MAPEDIT_ERROR_H

#include "msc.h"

void error_dump_file(char *name);
i16 error_read_line(char *dest, char *name, i16 n);
void error_report(i16 code, i16 level, i16 module, i32 data1, i32 data2);

extern volatile i16 error_abort;                /* error_report reports at this level and up */
extern void (*error_service_routine)(void);     /* run on a fatal error before exiting */
extern void (*error_service_routine_2)(void);   /* the same, run first */
extern char error_string[80];                   /* an extra line for the report */
extern i16 exit_value;                          /* the program's exit status */
void error_shutdown_report(char *errtext, char *modtext, char *numtext, char *extra, i32 memfree, i16 code);
void echo(i16 kind, char *text);

#endif
