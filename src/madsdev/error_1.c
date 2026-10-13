/* Imported from mapedit-decomp src/madsdev/error_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE error_1: MAPEDIT.EXE segment 0ed0 (error_1.c), file 0x10308-0x1076e, 4 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn error_report      0x106d6
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl29_77990).
 *
 * THE ENGINE'S TEXT-FILE AND SHUTDOWN HELPERS (overlay 29, one segment: every thunk has
 * para 0). DOS-only; there are no Windows twins.
 *   - read_nth_line (near _fastcall: dest in bx, name in ax, n in dx, all three spilled
 *     below the frame) opens `name` "rt", reads lines 1..n with fgets(36), stops at EOF,
 *     cuts the last line at its first control character and copies it to dest. It returns
 *     0 when it got the line, 1 otherwise.
 *   - type_file (far _fastcall, name in bx) shows a text file line by line through
 *     show_line(1, line) until EOF or a line starting "***".
 *   - shutdown_screen silences the sound if it is playing, shuts the two engine layers
 *     down and puts the BIOS back in text mode 3.
 *
 *   - error_report (near _fastcall; errtext bx, modtext ax, numtext dx, then extra, memfree
 *     and code on the stack, pushed left to right, `ret 8`) shuts the screen down, shows
 *     `Error "<err>" in module "<mod>" data: <n1> <n2>`, the extra g_dialog_key line, the
 *     allocation failure when g_mem_trace is set, the last memory check, the last DOS
 *     error and the stack indicator. It then types *warn0.dat (*warn6.dat for code -20),
 *     runs the two exit hooks and exits with status 3.
 *   - error_report (far _fastcall; code ax, level dx, module bx, then two longs) reports at
 *     g_err_level and up. It reads the error's text from line -code of *ERRORS.DB and the
 *     module's from line `module` of *MODULES.DB.
 *
 *  What the last two needed:
 *   - error_report uses its register parameters directly (no local copies): MSC then keeps
 *     modtext/numtext in si/di, errtext in a frame local, and reads the spilled numtext
 *     later, as shipped. With `register` local copies it homed all three into new slots.
 *   - error_report: `g_err_level` is volatile (shipped loads it into ax for `cmp dx,ax`),
 *     and the ERRORS.DB line is `~code + 1` (not/inc, not neg).
 *   - The initialised data (exit hooks, g_err_level, g_dialog_key) is defined BEFORE
 *     read_nth_line. MSC emits it after the next function's literals, so it lands after
 *     "rt" and ahead of type_file's literals, as shipped. Defined before type_file, it came
 *     after type_file's literals and split every _DATA reference in two.
 *
 * The module's _DATA runs from read_nth_line's "rt" (DS:0x246a) through an 8-byte zero
 * block, a word initialised to 1 (DS:0x2476) and an 80-byte buffer (DS:0x2478), which
 * the error formatter fills, to type_file's literals at 0x24c8.
 */

#include "msc.h"
#include "msc.h"
#include "msc.h"

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern MSC_FILE *file_open(char *name, char *mode);   /* 181f:0e86 */
extern void show_line(i16 kind, char *text);      /* 181f:2f26 */
extern i16 sound_driver_loaded(void);
extern void sound_control(i16 on);                               /* 181f:04de */
extern void sound_driver_stop(void);
extern void timer_remove(void);
extern void mouse_init(i16 a, i16 b);
extern i32 far_mem_free(void);                                  /* 181f:127a */
extern i16 stack_check();                                        /* 181f:08bc */
extern char g_mem_trace;                                         /* DS:0x264f */
extern i32 g_mem_request;                                       /* DS:0x2652 */
extern i32 g_mem_avail;                                         /* DS:0x2656 */
/* errno (DS:0x27ac) comes from <stdlib.h>: the runtime's */
void shutdown_screen(void);
void type_file(char *name);

/* Defined BEFORE read_nth_line: MSC emits initialised data after the next function's string
 * literals, so this lands after read_nth_line's "rt" as shipped. */
void (*g_exit_hook_a)(void) = 0;        /* DS:0x246e */
void (*g_exit_hook_b)(void) = 0;        /* DS:0x2472 */
volatile i16 g_err_level = 1;               /* DS:0x2476: error_report reports at this level and up */
char g_dialog_key[80] = "";                  /* DS:0x2478: an extra line the error report shows (the key ovl23 copies in) */
/* error_report -- 0ed0:03d6, 152 bytes */
// MSC: #pragma optimize("e", on)
void error_report(i16 code, i16 level, i16 module, i32 data1, i32 data2)
{
    volatile i16 unused;
    char d1[12];
    char d2[12];
    char emsg[40];
    char mname[40];

    unused = 0;
    if (g_err_level <= level) {
        msc_itoa(code, emsg, 10);
        msc_itoa(module, mname, 10);
        msc_ltoa(data1, d1, 10);
        msc_ltoa(data2, d2, 10);
        read_nth_line(emsg, "*ERRORS.DB", ~code + 1);
        read_nth_line(mname, "*MODULES.DB", module);
        error_report(emsg, mname, d1, d2, far_mem_free(), code);
    }
}

/* DGROUP data that follows this module's literals in the image (DS:0x25aa-0x25ab). Defined
 * after the last function, so no function's symbol count moves; the values are the shipped ones. */
i16 g_25aa = 0;                                     /* DS:0x25aa */

