/* game.c -- the game's state and the game functions the shared modules call. The editor links MicroProse's game modules (the map, popups) without the game: these are its records and its empty stand-ins. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

GameState game;                          /* DS:0x5e2c: 142 bytes */

Unit unit[1];                             /* DS:0x521e: one 28-byte Unit record */

i16 europe;                               /* DS:0x4e80: 2 bytes here (map.c reads it as Nation[]) */

i16 nationality[4];                       /* DS:0x4a74: 8 bytes, unused */

i16 active_unit = 0;                            /* DS:0x0072 */

i16 active_player = 0;                          /* DS:0x0074 */

i16 colony_screen_on = 0;                       /* DS:0x0076 */

i16 game_mode = 1;                              /* DS:0x0070 */

i16 any_action_this_turn;                 /* DS:0x49f0 */

i16 season_name[2];                       /* DS:0x49f6: 4 bytes, unused */

i16 cargo_names[16];                      /* DS:0x49ce: 32 bytes, unused */

i16 job_names[25];                        /* DS:0x4b18: 50 bytes, unused */

i16 order_names[13];                      /* DS:0x5ada: 26 bytes, unused */

i16 country_names[4];                     /* DS:0x697a: 8 bytes, unused */

i16 num_names;                            /* DS:0x521c */

i16 depth_list[90];                       /* DS:0x4bb6: 180 bytes, unused */

i16 depth_list_id[45];                    /* DS:0x4a8e: 90 bytes, unused */

i16 num_screens;                          /* DS:0x4e78 */

i16 automate = 0;                               /* DS:0x008a */

char game_text_name[6] = "GAME";                /* DS:0x0084 */

i16 going;                                /* DS:0x6976 */

i16 *sprite_base = 0;                       /* DS:0x008c */

/* reset_sprite_memory -- 0000:0000, 2 bytes */
void reset_sprite_memory(void)
{
}

/* load_main_sprites -- 0000:0002, 2 bytes */
void load_main_sprites(void)
{
}

/* check_music -- 0000:0004, 2 bytes */
void check_music(void)
{
}

/* build_check -- 0000:0006, 2 bytes */
void build_check(void)
{
}

/* piece_draw -- 0000:0008, 2 bytes */
void piece_draw(i16 piece, i16 nation, i16 x, i16 y, i16 size, i16 scale)
{
    (void)piece; (void)nation; (void)x; (void)y; (void)size; (void)scale;
}

/* sprite_load -- 0000:000a, 2 bytes */
void *sprite_load(char *name, i16 flags)
{
    (void)name; (void)flags;
    return 0;   /* 1994 returned what AX held: only the game's portraits call it */
}

/* danger -- 0000:000c, 2 bytes */
void danger(char *msg, i16 n, i16 x, i16 y)
{
    (void)msg; (void)n; (void)x; (void)y;
}

/* perform_save -- 0000:000e, 2 bytes */
void perform_save(i16 slot)
{
    (void)slot;
}

/* piss_factor -- 0000:0010, 2 bytes */
i16 piss_factor(i16 anger)
{
    (void)anger;
    return 0;   /* as sprite_load */
}

/* pissed -- 0000:0012, 2 bytes */
i16 pissed(i16 tribe, i16 nation)
{
    (void)tribe; (void)nation;
    return 0;   /* as sprite_load */
}

/* say_country -- 0000:0014, 2 bytes */
void say_country(i16 nation, i16 flag, char *buf)
{
    (void)nation; (void)flag; (void)buf;
}

/* cycle_save -- 0000:0016, 2 bytes */
void cycle_save(i16 save)
{
    (void)save;
}

/* computer_turn -- 0000:22ae, 2 bytes */
void computer_turn(void)
{
}
