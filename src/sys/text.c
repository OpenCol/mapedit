/* text.c -- the text files (MAPEDIT.TXT, NAMES.TXT): sections, lines and their items.
 *
 * A text file is divided into sections, each starting with a line "@NAME" (in upper case). The
 * reader keeps one file open at a time (text_handle) and reads it forward, one line at a time:
 *
 *   - text_open opens a file and moves to the line after a section's header; text_open with no
 *     file name looks for the next section further on in the file already open.
 *   - text_get reads the next line into text_buffer, trimmed, with each '_' made a space.
 *   - text_item_get takes the next comma-separated item from that line (text_pointer marks the
 *     place); text_item_number and text_item_string read it as a number, or as a string for
 *     the string store (strings.c).
 *
 * Lines are at most 79 characters; fgets splits a longer one, the rest becoming the next line.
 */
#include "msc.h"
#pragma pack(2)   /* records on 2-byte boundaries, as in 1994: the file formats depend on it */
#include "font.h"
#include "map.h"
#include "pal.h"
#include "popup.h"
#include "btype.h"
#include "fileio.h"
#include "loader.h"
#include "mapedit.h"
#include "records.h"
#include "text.h"
#include "units.h"

#include "msc.h"
#include "msc.h"

MSC_FILE *text_handle = NULL;               /* the open text file, NULL when there is none */
/* Closes the text file, if one is open. */
void text_close(void)
{
    if (text_handle != NULL) {
        msc_fclose(text_handle);
        text_handle = NULL;
    }
}
/* Opens the text file `name` (".TXT" is added when it has no extension; the file is opened in
 * text mode) and reads lines until one is "@" plus `key` upper-cased. A NULL name searches on
 * in the file already open instead: there, reaching the end of the file is allowed once, as an
 * empty line, but as nothing rewinds the file the next read fails, so a section before the
 * current place is not found. Sections must therefore be read in the order the file has them.
 *
 * Returns 0 when the section was found, with text_pointer at the end of the header line (so
 * text_item_get gives nothing until text_get reads the next line); otherwise 1, with the file
 * closed. */
i16 text_open(char *name, char *key)
{
    i16 flag;
    i16 wrap;
    i16 found;
    char tag[0x50];
    char path[0x50];

    flag = 1;
    wrap = 0;
    tag[0] = '@';
    tag[1] = 0;
    msc_strcat(tag, key);
    msc_strupr(tag);
    if (name != NULL) {
        text_close();
        msc_strcpy(path, name);
        fileio_add_ext(path, "TXT");
        text_handle = env_open(path, "rt");
        if (text_handle == NULL)
            goto out;
    } else
        wrap = 1;
    if (key != NULL) {
        found = 0;
        do {
            if (msc_fgets(text_buffer, 0x50, text_handle) == NULL) {
                if (wrap == 0)
                    goto out;
                wrap = 0;
                text_buffer[0] = 0;
            }
            fileio_fix_lf_input(text_buffer);
            fileio_purge_all_spaces(text_buffer);
            if (msc_strcmp(text_buffer, tag) == 0)
                found = 1;
        } while (found == 0);
        text_pointer = text_buffer + msc_strlen(text_buffer);
    }
    flag = 0;
out:
    if (flag)
        text_close();
    return flag;
}
/* Reads the next line into text_buffer: without its '\n', trimmed at both ends, each '_' made
 * a space. Points text_pointer at its start and returns text_buffer. At the end of the file it
 * returns NULL and closes the file; with no file open it does the same (the port's fgets
 * returns NULL for a null FILE). */
char *text_get(void)
{
    char *p;

    p = NULL;
    if (msc_fgets(text_buffer, 0x50, text_handle) != NULL) {
        fileio_fix_lf_input(text_buffer);
        fileio_purge_all_spaces(text_buffer);
        do {
            p = msc_strchr(text_buffer, '_');
            if (p != NULL)
                *p = ' ';
        } while (p != NULL);
        p = text_pointer = text_buffer;
    }
    if (p == NULL)
        text_close();
    return p;
}
/* Copies the line's next item -- up to the next ',' or the end of the line -- into text_item,
 * trimmed, moves text_pointer past it and its comma, and returns text_item. At the end of the
 * line it returns "". */
char *text_item_get(void)
{
    char *d = text_item;

    while (*text_pointer != 0 && *text_pointer != ',') {
        *d = *text_pointer;
        text_pointer++;
        d++;
    }
    if (*text_pointer != 0)
        text_pointer++;
    *d = 0;
    fileio_purge_all_spaces(text_item);
    return text_item;
}
/* The next item as a number (stoi: decimal, or 0x/0b/0d prefixed). */
i16 text_item_number(void)
{
    return stoi(text_item_get());
}
/* Puts the next item in the string store; returns its number there. */
i16 text_item_string(void)
{
    text_item_get();
    return strings_store(text_item);
}

char text_item[80];                       /* the item text_item_get read last */
char *text_pointer;                       /* where text_item_get reads next, in text_buffer */
char text_buffer[80];                     /* the line read last */
