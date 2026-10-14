/* game.c -- the game's records and empty stand-ins for the game functions shared code calls.
 *
 * The editor is built from modules it shares with the game (the map, the popups, the menus),
 * and those refer to game state and game functions the editor does not have. This file
 * supplies them: the records the shared code reads (most never set, many never read), and
 * functions that do nothing. Where a stand-in returns a value, it returns 0.
 */
#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
#include "font.h"
#include "map.h"
#include "error.h"
#include "game.h"
#include "loader.h"
#include "menu.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "sprite.h"

GameState game;             /* the game state; the editor sets only game_in_progress */

Unit unit[1];               /* one unit record, for the popups' unit sprites */

i16 europe;                 /* unused */

i16 nationality[4];         /* unused */

i16 active_unit = 0;        /* unused */

i16 active_player = 0;      /* unused */

i16 colony_screen_on = 0;   /* unused */

i16 game_mode = 1;          /* 1: the editor (edit.c centres the cursor with the view) */

i16 any_action_this_turn;   /* unused */

i16 season_name[2];         /* unused */

i16 cargo_names[16];        /* unused */

i16 job_names[25];          /* unused */

i16 order_names[13];        /* unused */

i16 country_names[4];       /* unused */

i16 num_names;              /* the file menu: how many files match */

i16 depth_list[90];         /* unused */

i16 depth_list_id[45];      /* unused */

i16 num_screens;            /* the file menu: how many pages of ten names */

i16 automate = 0;           /* autoplay: popups close themselves; always 0 here */

char game_text_name[6] = "GAME"; /* the text section the shared popups read */

i16 going;                  /* unused */

i16 *sprite_base = 0;       /* scratch memory lent to Find Continents */

/* The game frees its main sprites here when a popup's portrait does not fit in memory. */
void reset_sprite_memory(void)
{
}

/* The game reloads its main sprites here, after a portrait popup or Find Continents. */
void load_main_sprites(void)
{
}

/* The game keeps its music going here while a menu or popup waits for input. */
void check_music(void)
{
}

/* Called by Find Continents after each row of the map. */
void build_check(void)
{
}

/* Draws a unit piece in a popup. */
void piece_draw(i16 piece, i16 nation, i16 x, i16 y, i16 size, i16 scale)
{
    (void)piece; (void)nation; (void)x; (void)y; (void)size; (void)scale;
}

void *sprite_load(char *name, i16 flags)
{
    (void)name; (void)flags;
    return 0;   /* Port: the original returned whatever AX held; only portrait popups call it */
}

/* The game's debugging report, called by owner_set. */
void danger(char *msg, i16 n, i16 x, i16 y)
{
    (void)msg; (void)n; (void)x; (void)y;
}

/* The game's emergency save, called by owner_set. */
void perform_save(i16 slot)
{
    (void)slot;
}

i16 piss_factor(i16 anger)
{
    (void)anger;
    return 0;   /* Port: as sprite_load, 0 where the original returned AX */
}

i16 pissed(i16 tribe, i16 nation)
{
    (void)tribe; (void)nation;
    return 0;   /* Port: as sprite_load */
}

/* A nation's name for a popup; buf is left as it was. */
void say_country(i16 nation, i16 flag, char *buf)
{
    (void)nation; (void)flag; (void)buf;
}

/* The game saves (1) and restores (0) its colour cycling here around a portrait load. */
void cycle_save(i16 save)
{
    (void)save;
}

/* Nothing calls it. */
void computer_turn(void)
{
}
