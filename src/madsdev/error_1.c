/* From mapedit-decomp src/madsdev/error_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE error_1: MAPEDIT.EXE segment 0ed0 (error_1.c), file 0x10308-0x1076e, 4 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn error_read_line     0x10308
 * fn error_dump_file     0x103ac
 * fn error_shutdown_report 0x10456
 * fn error_report        0x106d6
 *
 * THE LIBRARY'S FATAL-ERROR REPORT (madsdev.lib).
 *   - error_read_line (static, near _fastcall: dest in bx, name in ax, n in dx, all three
 *     spilled below the frame) opens `name` "rt" through env_open, reads lines 1..n with
 *     fgets(36), stops at EOF, cuts the last line at its first control character and
 *     copies it to dest. It returns 0 when it got the line, -1 otherwise (VICEROY: 1).
 *   - error_dump_file (far _fastcall, name in bx) echoes a text file line by line through
 *     echo(-1, line) until EOF or a line starting "***". Its `err` is set to -1 and never
 *     read (the dead store is kept, as VICEROY keeps its `err = 1`).
 *   - error_shutdown_report (static, near _fastcall; errtext bx, modtext ax, numtext dx,
 *     then extra, memfree and code on the stack, pushed left to right, `ret 8`) shuts the
 *     sound, timer, keyboard and high-memory layers down, resets the mouse and the screen
 *     and puts the BIOS back in text mode 3. Then it shows `Error "<err>" in module
 *     "<mod>" data: <n1> <n2>`, the extra error_string line, the allocation failure when
 *     mem_last_alloc_failed is set, the last memory check, the last DOS error and the
 *     stack indicator. It then types *warn0.dat (*warn6.dat for code -20), runs the two
 *     service routines and exits with status 3.
 *   - error_report (far _fastcall; code ax, level dx, module bx, then two longs) reports at
 *     error_abort and up. It reads the error's text from line -code of *ERRORS.DB and the
 *     module's from line `module` of *MODULES.DB.
 *
 * What the DOS twin's notes said still holds: error_report uses its register parameters
 * directly (MSC keeps modtext/numtext in si/di), error_abort is volatile (shipped loads it
 * into ax for `cmp dx,ax`), the ERRORS.DB line is `~code + 1`, and the initialised data is
 * defined BEFORE error_read_line so that it lands after that function's "rt" (DS:0x3b24)
 * and ahead of error_dump_file's literals (DS:0x3b82), as shipped.
 */

#include "msc.h"
#include "msc.h"
#include "msc.h"


extern MSC_FILE *env_open(char *name, char *mode);    /* 0297:0104 */
extern void echo(i16 kind, char *text);           /* 127b:0000 */
extern i16 sound_initialized(void);                              /* 1115:0119 */
extern void sound_queue(i16 on);                                 /* 1127:0004 */
extern void sound_system_shutdown(void);                         /* 1127:0059 */
extern void timer_remove(void);                                  /* 0d1c:01f3 */
extern void keys_remove(void);                                   /* 0ffe:01c7 */
extern void himem_shutdown(void);                                /* 0f3e:000c */
extern void mouse_init(i16 a, i16 b);                            /* 0f65:0086 */
extern void screen_dominant_mode(i16 mode);            /* 0c3c:000a */
extern i32 mem_get_avail(void);                       /* 0d08:0082 */
extern i16 stack_check();                                        /* 12aa:001b */
extern char mem_last_alloc_failed;                               /* DS:0x74b */
extern i32 mem_last_alloc_size;                                 /* DS:0x74e */
extern i32 mem_last_alloc_avail;                                /* DS:0x752 */
/* errno (DS:0x4568) comes from <stdlib.h>: the runtime's */
void error_dump_file(char *name);

/* Defined BEFORE error_read_line: MSC emits initialised data after the next function's string
 * literals, so this lands after error_read_line's "rt" as shipped. */
void (*error_service_routine)(void) = 0;    /* DS:0x3b28 */
void (*error_service_routine_2)(void) = 0;  /* DS:0x3b2c */
volatile i16 error_abort = 1;                   /* DS:0x3b30: error_report reports at this level and up */
char error_string[80] = "";                     /* DS:0x3b32: an extra line the error report shows */
/* error_read_line -- 0ed0:0008, 164 bytes */
i16 error_read_line(char *dest, char *name, i16 n)
{
    i16 unused;
    i16 pad;
    char line[36];
    i16 err;
    i16 k;
    MSC_FILE *handle;

    err = -1;
    handle = env_open(name, "rt");
    if (handle != NULL) {
        for (k = 1; k <= n; k++) {
            if (feof(handle))
                goto done;
            if (msc_fgets(line, 36, handle) == NULL)
                goto done;
        }
        for (k = 0; k < (i16)msc_strlen(line); k++)
            if (line[k] < ' ')
                line[k] = 0;
        msc_strcpy(dest, line);
        err = 0;
    }
done:
    if (handle != NULL)
        msc_fclose(handle);
    return err;
}
/* error_dump_file -- 0ed0:00ac, 170 bytes */
void error_dump_file(char *name)
{
    char line[80];
    i16 err;
    i16 loop;
    i16 k;
    MSC_FILE *handle;

    err = -1;
    handle = env_open(name, "rt");
    if (handle != NULL) {
        loop = -1;
        do {
            if (feof(handle))
                break;
            if (msc_fgets(line, 0x4f, handle) == NULL)
                break;
            for (k = 0; k < (i16)msc_strlen(line); k++)
                if (line[k] < ' ')
                    line[k] = 0;
            if (msc_strncmp(line, "***", 3) == 0)
                loop = 0;
            else
                echo(-1, line);
        } while (loop != 0);
    }
    if (handle != NULL)
        msc_fclose(handle);
}
/* error_shutdown_report -- 0ed0:0156, 640 bytes */
/* PORT: error_shutdown_report is in port/ (inline assembly) */
void error_shutdown_report(char *errtext, char *modtext, char *numtext, char *extra, i32 memfree, i16 code);

/* error_report -- 0ed0:03d6, 152 bytes */
void error_report(i16 code, i16 level, i16 module, i32 data1, i32 data2)
{
    volatile i16 unused;
    char d1[12];
    char d2[12];
    char emsg[40];
    char mname[40];

    unused = 0;
    if (error_abort <= level) {
        msc_itoa(code, emsg, 10);
        msc_itoa(module, mname, 10);
        msc_ltoa(data1, d1, 10);
        msc_ltoa(data2, d2, 10);
        error_read_line(emsg, "*ERRORS.DB", ~code + 1);
        error_read_line(mname, "*MODULES.DB", module);
        error_shutdown_report(emsg, mname, d1, d2, mem_get_avail(), code);
    }
}

