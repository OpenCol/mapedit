/* game.h -- the game's state, and stand-ins for the game functions the shared code calls.
 *
 * Defined in core/game.c. The editor shares modules with the game, Colonization (the map,
 * popups, sprites), but not the game itself. These are the game's records and variables
 * those modules read, and empty versions of the game functions they call: the stand-ins do
 * nothing and return 0. Most of the data is never read by the editor; game_mode is always 1.
 */
#ifndef MAPEDIT_GAME_H
#define MAPEDIT_GAME_H

#include "msc.h"
#include "records.h"
extern GameState game;
extern Unit unit[1];
extern i16 game_mode;
extern i16 num_names;
extern i16 num_screens;
extern i16 automate;
extern i16 *sprite_base;        /* a large scratch block Find Continents borrows */
void reset_sprite_memory(void);
void load_main_sprites(void);
void check_music(void);
void build_check(void);
void piece_draw(i16 piece, i16 nation, i16 x, i16 y, i16 size, i16 scale);
void *sprite_load(char *name, i16 flags);
i16 piss_factor(i16 anger);
i16 pissed(i16 tribe, i16 nation);
void say_country(i16 nation, i16 flag, char *buf);
void cycle_save(i16 save);

#endif
