/* mapedit.h -- the editor's startup and commands, the file menu; data file paths; a clamp.
 *
 * Three groups of declarations:
 *   editor/mapedit.c   the program: editor_main reads the command line (flag_parse), and
 *                      viceroy_game sets up the screen, the fonts and the art, has a map chosen
 *                      and loaded or created (start_new_game) and runs the editor. It builds the
 *                      menu bar from MAPMENU.TXT (construct_mapedit_menu) and carries out its
 *                      commands (execute_menu_event); file_menu is the paged popup that picks a
 *                      map file.
 *   sys/fileio.c       data file paths: env_get_path drops a leading '*' and leaves the name
 *                      relative to the game folder (env_path is never used); env_open opens one.
 *   core/distance.c    the clamp minimax.
 */
#ifndef MAPEDIT_MAPEDIT_H
#define MAPEDIT_MAPEDIT_H

#include "msc.h"
#include "menu.h"
#include "font.h"
#include "records.h"


/* sys/fileio.c */
char *env_catint(char *s, i16 value, i16 digits);
char *env_get_path(char *dst, char *name);
MSC_FILE *env_open(char *name, char *mode);
/* editor/mapedit.c */
i16 checked_write_map(void);             /* port: write_map_file after the checks */
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
/* core/distance.c */
i16 minimax(i16 value, i16 lo, i16 hi);
i16 construct_main_menu(void);         /* not defined: the editor builds its own menu */

extern char env_path[60];          /* the data files' directory, when env_special_path */
extern i16 env_special_path;       /* data files are under env_path (never set) */
extern char file_names[10][13];    /* file_menu's page: up to ten 8.3 names */
extern char *file_select;          /* the name file_menu chose */
extern i16 file_set;               /* unused, as is mads_mode */
extern i16 mads_mode;

#endif
