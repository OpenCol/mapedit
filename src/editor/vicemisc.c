/* Imported from mapedit-decomp src/editor/vicemisc by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE vicemisc: MAPEDIT.EXE segment 08f9 (vicemisc.obj), file 0xa594-0xb176, 3 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn construct_main_menu 0x0a594
 * fn load_terrain_series 0x0b09e
 * fn load_terrain_tiles  0x0b152
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl26_72090). CENSUS: the layout cuts construct_main_menu at 2274 bytes and
 * files the next 552 bytes (0xae76-0xb09e) under `__acrtused`; that public is MSC's absolute
 * link marker for the module holding main (not code), and 0xae76 is the middle of
 * construct_main_menu (`add sp,0xc; push 0; push ds; push "" ...`, and its first `jmp out`
 * lands at +0xaff). construct_main_menu is one 2826-byte function, 0xa594-0xb09e.
 *
 * THE EDITOR'S MAIN MENU BAR. construct_main_menu() builds menu_bar from MENU.TXT: seven
 * submenus (game, view, orders, reports, trade, the Continental Congress "cup", pedia),
 * each item's text the next line of its section, with "" separators. The "cup" menu's
 * debug item is hidden unless the cheat bit (game_flags 0x2000) is set. It returns 0, or 1
 * when the menu or a section cannot be had. The call sequence and every item id are
 * VICEROY's create_main_menu's, one to one; text_get() takes no argument (it returns the
 * section's next line). "elg": the returned item's offset lives in di, its segment in bp-2.
 * (The editor's own menu handling is in mapedit.obj; the game's item ids are kept.)
 *
 * load_terrain_series(name) draws the 12 16x16 terrain tiles of an icon sheet into a tile
 * sheet (tile_allocate; exit_value 0x321/0x322 on failure); load_terrain_tiles loads
 * "terrain" into terrain_1. Both still compile under "elg" (the induction variable di walks
 * the tile sheet), as in VICEROY.
 */

#include "records.h"
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 id; i16 kind; } MenuItemHead;

extern void *menu_bar;               /* DS:0x0078 */
extern void *menu_font;              /* DS:0x0080 */
extern GameState game;                   /* DS:0x5e2c */
extern void *menu_create(i16 size, void *font);
extern void *menu_add_bar_item(void *m, char *name, i16 id, i16 right);
extern MenuItemHead *menu_add_item(void *m, i16 sub_id, char *name, i16 id);
extern void menu_bar_hide(void *menu, i16 item, i16 hidden);
extern i16 text_open(char *file, char *section);
extern char *text_get(void);
extern void text_close(void);
/* construct_main_menu -- 08f9:0004, 2826 bytes */
// MSC: #pragma optimize("elg", on)
i16 construct_main_menu(void)
{
    MenuItemHead *p;
    register i16 r;

    r = 1;
    if ((menu_bar = menu_create(0xfa0, menu_font)) == 0)
        goto out;
    if (text_open("menu", "game"))
        goto out;
    menu_add_bar_item(menu_bar, text_get(), 1, 0);
    menu_add_item(menu_bar, 1, text_get(), 1);
    menu_add_item(menu_bar, 1, text_get(), 2);
    menu_add_item(menu_bar, 1, "", 0);
    menu_add_item(menu_bar, 1, text_get(), 3);
    menu_add_item(menu_bar, 1, text_get(), 4);
    menu_add_item(menu_bar, 1, "", 0);
    menu_add_item(menu_bar, 1, text_get(), 26);
    menu_add_item(menu_bar, 1, text_get(), 27);
    menu_add_item(menu_bar, 1, "", 0);
    menu_add_item(menu_bar, 1, text_get(), 28);
    menu_add_item(menu_bar, 1, "", 0);
    menu_add_item(menu_bar, 1, text_get(), 30);
    menu_add_item(menu_bar, 1, text_get(), 31);
    if (text_open(0, "view"))
        goto out;
    menu_add_bar_item(menu_bar, text_get(), 2, 0);
    menu_add_item(menu_bar, 2, text_get(), 32);
    menu_add_item(menu_bar, 2, text_get(), 33);
    menu_add_item(menu_bar, 2, text_get(), 34);
    menu_add_item(menu_bar, 2, "", 0);
    menu_add_item(menu_bar, 2, text_get(), 35);
    menu_add_item(menu_bar, 2, "", 0);
    menu_add_item(menu_bar, 2, text_get(), 36);
    menu_add_item(menu_bar, 2, text_get(), 37);
    menu_add_item(menu_bar, 2, "", 0);
    menu_add_item(menu_bar, 2, text_get(), 38);
    menu_add_item(menu_bar, 2, text_get(), 39);
    menu_add_item(menu_bar, 2, text_get(), 40);
    menu_add_item(menu_bar, 2, text_get(), 41);
    menu_add_item(menu_bar, 2, "", 0);
    menu_add_item(menu_bar, 2, text_get(), 42);
    menu_add_item(menu_bar, 2, text_get(), 43);
    if (text_open(0, "orders"))
        goto out;
    menu_add_bar_item(menu_bar, text_get(), 3, 0);
    menu_add_item(menu_bar, 3, text_get(), 768);
    menu_add_item(menu_bar, 3, text_get(), 769);
    menu_add_item(menu_bar, 3, text_get(), 770);
    menu_add_item(menu_bar, 3, text_get(), 771);
    menu_add_item(menu_bar, 3, text_get(), 772);
    menu_add_item(menu_bar, 3, "", 0);
    menu_add_item(menu_bar, 3, text_get(), 784);
    menu_add_item(menu_bar, 3, text_get(), 785);
    menu_add_item(menu_bar, 3, text_get(), 786);
    menu_add_item(menu_bar, 3, text_get(), 787);
    menu_add_item(menu_bar, 3, text_get(), 788);
    menu_add_item(menu_bar, 3, text_get(), 789);
    menu_add_item(menu_bar, 3, text_get(), 790);
    menu_add_item(menu_bar, 3, text_get(), 791);
    menu_add_item(menu_bar, 3, "", 255);
    menu_add_item(menu_bar, 3, text_get(), 800);
    menu_add_item(menu_bar, 3, text_get(), 801);
    menu_add_item(menu_bar, 3, text_get(), 802);
    menu_add_item(menu_bar, 3, text_get(), 803);
    menu_add_item(menu_bar, 3, "", 256);
    menu_add_item(menu_bar, 3, text_get(), 816);
    menu_add_item(menu_bar, 3, "", 0);
    menu_add_item(menu_bar, 3, text_get(), 817);
    menu_add_item(menu_bar, 3, text_get(), 818);
    if (text_open(0, "reports"))
        goto out;
    menu_add_bar_item(menu_bar, text_get(), 4, 0);
    menu_add_item(menu_bar, 4, text_get(), 64);
    menu_add_item(menu_bar, 4, "", 0);
    menu_add_item(menu_bar, 4, text_get(), 65);
    menu_add_item(menu_bar, 4, text_get(), 66);
    menu_add_item(menu_bar, 4, text_get(), 67);
    menu_add_item(menu_bar, 4, text_get(), 68);
    menu_add_item(menu_bar, 4, "", 0);
    menu_add_item(menu_bar, 4, text_get(), 69);
    menu_add_item(menu_bar, 4, text_get(), 70);
    menu_add_item(menu_bar, 4, text_get(), 71);
    menu_add_item(menu_bar, 4, text_get(), 72);
    menu_add_item(menu_bar, 4, "", 0);
    menu_add_item(menu_bar, 4, text_get(), 73);
    if (text_open(0, "trade"))
        goto out;
    menu_add_bar_item(menu_bar, text_get(), 5, 0);
    menu_add_item(menu_bar, 5, text_get(), 80);
    menu_add_item(menu_bar, 5, text_get(), 81);
    menu_add_item(menu_bar, 5, text_get(), 82);
    if (text_open(0, "cup"))
        goto out;
    menu_add_bar_item(menu_bar, text_get(), 6, 0);
    menu_add_item(menu_bar, 6, text_get(), 98);
    menu_add_item(menu_bar, 6, text_get(), 99);
    menu_add_item(menu_bar, 6, "", 0);
    menu_add_item(menu_bar, 6, text_get(), 101);
    menu_add_item(menu_bar, 6, text_get(), 102);
    menu_add_item(menu_bar, 6, "", 0);
    menu_add_item(menu_bar, 6, text_get(), 103);
    menu_add_item(menu_bar, 6, text_get(), 104);
    menu_add_item(menu_bar, 6, "", 0);
    p = menu_add_item(menu_bar, 6, text_get(), 105);
    menu_add_item(menu_bar, 6, text_get(), 106);
    menu_add_item(menu_bar, 6, "", 0);
    menu_add_item(menu_bar, 6, text_get(), 107);
    menu_add_item(menu_bar, 6, text_get(), 108);
    menu_add_item(menu_bar, 6, text_get(), 111);
    p->kind = 0x13;
    if (!(game.game_flags & 0x2000))
        menu_bar_hide(menu_bar, 6, 1);
    if (text_open(0, "pedia"))
        goto out;
    menu_add_bar_item(menu_bar, text_get(), 7, 1);
    menu_add_item(menu_bar, 7, text_get(), 112);
    menu_add_item(menu_bar, 7, text_get(), 113);
    menu_add_item(menu_bar, 7, text_get(), 114);
    menu_add_item(menu_bar, 7, "", 0);
    menu_add_item(menu_bar, 7, text_get(), 115);
    menu_add_item(menu_bar, 7, text_get(), 116);
    menu_add_item(menu_bar, 7, text_get(), 117);
    r = 0;
out:
    text_close();
    return r;
}
/* load_terrain_series -- 08f9:0b0e, 180 bytes */
typedef struct { i16 h; i16 w; char *bits; } DosPort;

extern i16 exit_value;                   /* DS:0x0062 */
extern char *tile_allocate(i16 n);
extern void *sprite_series_load(char *name, i16 flags);
extern void sprite_draw(i16 icon, void *sheet, DosPort *port, i16 x, i16 y);
extern i16 mem_free(void *p);

void *load_terrain_series(char *name)
{
    DosPort port;
    char *tiles;
    void *sheet;
    void *r;
    i16 n;
    i16 i;

    r = 0;
    n = 12;
    tiles = tile_allocate(n);
    if (tiles == 0) {
        exit_value = 0x321;
        goto out;
    }
    sheet = sprite_series_load(name, 0x4000);
    if (sheet == 0) {
        exit_value = 0x322;
        goto out;
    }
    port.h = port.w = 16;
    for (i = 0; i < n; i++) {
        port.bits = tiles + i * 0x100;
        sprite_draw(i + 1, sheet, &port, 0, 0);
    }
    mem_free(sheet);
    r = tiles;
out:
    return r;
}
/* load_terrain_tiles -- 08f9:0bc2, 36 bytes */
extern void *terrain_1;              /* DS:0x04b8 */

i16 load_terrain_tiles(void)
{
    i16 r;

    r = 1;
    if ((terrain_1 = load_terrain_series("terrain")) != 0)
        r = 0;
    return r;
}

