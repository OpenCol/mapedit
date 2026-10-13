/* game_dir.h -- where the player's Colonization is (game_dir.c) */
#ifndef MAPEDIT_GAME_DIR_H
#define MAPEDIT_GAME_DIR_H

/* The game's directory: `given` if it holds the game (NULL if not); else the first of the
   current directory, the program's, the remembered choice; else, when interactive, ask the
   player with a folder dialog. choose_again skips straight to asking. NULL: no game. */
const char *game_dir_find(const char *given, int interactive, int choose_again);

#endif
