/* error.c -- errors: reporting one (and shutting down for a fatal one), console output.
 *
 * error_report(code, level, module, data1, data2) is the program's one error path. Every
 * report at error_abort's level or above is fatal, and error_abort is 1, so in practice every
 * call ends the program: error_shutdown_report shuts the subsystems down, closes the window and
 * prints the report on standard output before exiting with status 3.
 *
 * The report names the error and the module from two text files in the game folder, one
 * entry per line: line -code of ERRORS.DB is the error's text (codes are negative) and line
 * `module` of MODULES.DB the module's name; when a line is missing the number stands instead.
 * After the report it prints warn0.dat (warn6.dat alone for code -20) up to a line that starts
 * "***". A leading '*' in these names means "in the game folder" (fileio.c).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msc.h"
#include "host.h"
#include "error.h"
#include "font.h"
#include "keys.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mem.h"
#include "menu.h"
#include "mouse.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "screen.h"
#include "sound.h"
#include "sprite.h"
#include "timer.h"
#include "units.h"
#include "w16types.h"
#pragma pack(2)   /* records on 2-byte boundaries, as the 1994 compiler laid them out */

/* errno comes from <stdlib.h>. */

volatile i16 error_abort = 1;                   /* error_report reports at this level and up */
char error_string[80] = "";                     /* an extra line the error report shows */
/* error_read_line -- copies line n (counting from 1) of the text file name into dest, cut at
 * its first control character, and returns 0; returns -1, leaving dest alone, when the file
 * cannot be opened or ends before line n. Quirks of the original: lines are read 35
 * characters at a time, so a longer line counts as two; a character of 80h and up also cuts
 * the line (char is signed); and for n < 1 nothing is read and dest gets whatever the line
 * buffer held (no caller asks for that). */
i16 error_read_line(char *dest, char *name, i16 n)
{
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
/* error_dump_file -- prints the text file name line by line (79 characters at a time), each
 * cut at its first control character, until the end of the file or a line that starts "***".
 * A missing file prints nothing. */
void error_dump_file(char *name)
{
    char line[80];
    i16 loop;
    i16 k;
    MSC_FILE *handle;

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
/* error_report -- reports error code (negative) in module, with two numbers for context, if
 * level is error_abort or above; that report ends the program (error_shutdown_report). The
 * texts come from ERRORS.DB and MODULES.DB, the numbers when a text is missing. */
void error_report(i16 code, i16 level, i16 module, i32 data1, i32 data2)
{
    char d1[12];
    char d2[12];
    char emsg[40];
    char mname[40];

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

/* The program's exit status: startup sets it when something fails to load, and the editor
 * prints it on the way out when it is not 0. */
i16 exit_value = 0;

/* error_shutdown_report -- the fatal end: shuts down the sound, the clock, the keyboard and the
 * mouse, leaves graphics mode, prints the report and the warning file, calls the service
 * routines and exits with status 3.
 *   errtext, modtext  the error's and the module's text (or number);
 *   numtext, extra    the two context numbers, as text;
 *   memfree           the free memory the report shows (mem_get_avail);
 *   code              the error code; -20 prints only warn6.dat, with no report.
 * Under the main line it shows error_string, when set, and the failed allocation, when the
 * last one failed. Port: the window is closed where the original set text mode through the
 * BIOS, and the last DOS error and the stack use, which have no meaning here, print as 0. */
void error_shutdown_report(char *errtext, char *modtext, char *numtext, char *extra, i32 memfree, i16 code)
{
    char line[80];
    if (sound_initialized()) {
        sound_queue(0);
        sound_system_shutdown();
    }
    timer_remove();
    mouse_init(0, 3);
    screen_dominant_mode(3);
    host_quit();                        /* Port: 1994 switched to text mode */
    if (code != -20) {
        snprintf(line, sizeof line, "Error \"%s\" in module \"%s\" data: %s %s", errtext, modtext, numtext, extra);
        echo(-1, line);
        if (strlen(error_string))
            echo(-1, error_string);
        if (mem_last_alloc_failed) {
            snprintf(line, sizeof line, "Tried to allocate %ld bytes when only %ld bytes were free.",
                     (long)mem_last_alloc_size, (long)mem_last_alloc_avail);
            echo(-1, line);
        }
        echo(-1, " ");
        snprintf(line, sizeof line, "Most recent memory check: %ld", (long)memfree);
        echo(-1, line);
        echo(-1, "Most recent DOS error:    0");
        echo(-1, "Stack use indicator:      0");
        echo(-1, " ");
        error_dump_file("*warn0.dat");
    } else
        error_dump_file("*warn6.dat");
    exit(3);
}

/* echo -- writes text to standard output, then a line break if kind is not 0. Port: the
 * original wrote the text to DOS handle 1 and ended the line with LF then CR; the port writes
 * "\n" and flushes. */
void echo(i16 kind, char *text)
{
    fputs(text, stdout);
    if (kind)
        fputs("\n", stdout);
    fflush(stdout);
}
