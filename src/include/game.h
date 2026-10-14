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
extern i16 europe;
extern i16 nationality[4];
extern i16 active_unit;
extern i16 active_player;
extern i16 colony_screen_on;
extern i16 game_mode;
extern i16 any_action_this_turn;
extern i16 season_name[2];
extern i16 cargo_names[16];
extern i16 job_names[25];
extern i16 order_names[13];
extern i16 country_names[4];
extern i16 num_names;
extern i16 depth_list[90];
extern i16 depth_list_id[45];
extern i16 num_screens;
extern i16 automate;
extern char game_text_name[6];
extern i16 going;
extern i16 *sprite_base;        /* a large scratch block Find Continents borrows */
void reset_sprite_memory(void);
void load_main_sprites(void);
void check_music(void);
void build_check(void);
void piece_draw(i16 piece, i16 nation, i16 x, i16 y, i16 size, i16 scale);
void *sprite_load(char *name, i16 flags);
void danger(char *msg, i16 n, i16 x, i16 y);
void perform_save(i16 slot);
i16 piss_factor(i16 anger);
i16 pissed(i16 tribe, i16 nation);
void say_country(i16 nation, i16 flag, char *buf);
void cycle_save(i16 save);
void computer_turn(void);

#endif
