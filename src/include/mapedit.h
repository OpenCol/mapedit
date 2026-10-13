/* mapedit.h -- the functions, data and records of env_1.c, fileio.c, mapedit.c, stuff.c, vicemisc.c. */
#ifndef MAPEDIT_MAPEDIT_H
#define MAPEDIT_MAPEDIT_H

#include "msc.h"
#include "menu.h"
#include "font.h"
#include "records.h"

#pragma pack(push, 2)
#pragma pack(pop)

i16 env_exist(char *name);
char *env_catint(char *s, i16 value, i16 digits);
i32 env_get_file_size(MSC_FILE *fp);
char *env_get_path(char *dst, char *name);
MSC_FILE *env_open(char *name, char *mode);
i16 construct_mapedit_menu(void);
void editor_main(i16 argc, char **argv);
void execute_menu_event(i16 id);
i16 file_menu(char *file, char *key, char *pattern);
void flag_parse(char **p);
void memory_check(void);
i16 scan_past(char **p, char c);
void show_flags(void);
void show_logo(void);
i16 start_new_game(void);
void viceroy_game(void);
i16 bearing_adjacent(i16 a, i16 b);
i16 map_dist(i16 x1, i16 y1, i16 x2, i16 y2);
i16 map_dist_2(i16 x1, i16 y1, i16 x2, i16 y2);
i16 minimax(i16 value, i16 lo, i16 hi);
void swap(i16 *a, i16 *b);
i16 xy_dist(i16 dx, i16 dy);
i16 xy_dist_2(i16 dx, i16 dy);
i16 construct_main_menu(void);

extern char env_path[60];
extern i16 env_search_mode;
extern i16 env_special_path;
extern char file_names[10][13];
extern char *file_select;
extern i16 file_set;
extern void *game_menu;
extern i16 load_error;
extern i16 mads_mode;

#endif
