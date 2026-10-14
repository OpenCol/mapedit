/* text.h -- the text files (MAPEDIT.TXT, NAMES.TXT) and the string store.
 *
 * Defined in sys/text.c and sys/strings.c (strings*). A text file is divided into sections,
 * each opened by an "@NAME" line. One file is open at a time and read forward: text_open
 * finds a section, text_get reads its next line into text_buffer and text_item_get takes the
 * line's comma-separated items one by one, which text_item_number and text_item_string read
 * as a number or as a string for the string store.
 *
 * The string store keeps the names and labels read from the text files end to end in one
 * arena; strings_store adds one and returns its number, and strings(n) finds it again.
 */
#ifndef MAPEDIT_TEXT_H
#define MAPEDIT_TEXT_H

#include "msc.h"

void strings_init(i32 size);
i16 strings_store(char *s);
void text_close(void);
char *text_get(void);
char *text_item_get(void);
i16 text_item_number(void);
i16 text_item_string(void);
i16 text_open(char *name, char *key);

extern i16 num_strings;         /* strings in the store */
extern Arena strings_heap;      /* the store: the strings, end to end */
extern char text_buffer[80];    /* the line read last */
extern MSC_FILE *text_handle;   /* the open text file, or null */
extern char text_item[80];      /* the item read last */
extern char *text_pointer;      /* where the next item starts, in text_buffer */
char *strings(i16 n);

#endif
