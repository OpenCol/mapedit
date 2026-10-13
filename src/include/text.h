/* text.h -- the functions, data and records of misc.c, strings.c, text.c. */
#ifndef MAPEDIT_TEXT_H
#define MAPEDIT_TEXT_H

#include "msc.h"

char *strings(i16 n);
void strings_init(i32 size);
void strings_shrink(void);
i16 strings_store(char *s);
void text_close(void);
char *text_get(void);
u8 text_item_binary(void);
char *text_item_get(void);
i16 text_item_number(void);
i16 text_item_string(void);
void text_next_line(void);
i16 text_open(char *name, char *key);
i16 text_search(char *name, char *key, i16 n);
i16 text_string(void);

extern i16 num_strings;
extern Arena strings_heap;
extern char text_buffer[80];
extern MSC_FILE *text_handle;
extern char text_item[80];
extern char *text_pointer;

#endif
