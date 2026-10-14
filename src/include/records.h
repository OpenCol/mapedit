/* records.h -- the records of Colonization, the game the editor shares its modules with.
 *
 * One definition of each record the game keeps (its DOS and Windows versions), so that every
 * file that uses one agrees on its layout. The editor uses three: GameState (the popups read
 * the player's nation), Unit (the popups that show units) and MapSize (map_file, the .MP
 * header). The rest describe the game and are not used here. units.h describes the Unit
 * record and the unit types.
 */
#ifndef RECORDS_H
#define RECORDS_H

#include "w16types.h"

typedef struct { i16 time; i16 count; i16 kind; } AnimEvent;
typedef struct { char pad[0x8c]; void *mci; void (*on_mci_done)(void); } Audio;
typedef struct { char pad[0xc4]; i16 shown_half; } BackdropWin;
typedef struct { u16 w, h; i16 x, y; u8 nplanes; i8 masking; u8 compression; char pad; i16 transparent; char xasp, yasp; i16 pagew, pageh; } Bmhd;
typedef struct { u8 building, expert_job, goods, f3; } BuildNeed;
typedef struct { i16 name; i8 prereq2; i8 prev_level; i8 next_level; i8 kind; i8 family; i8 tools; i8 min_pop; i8 upkeep; i16 hammers; } BuildingRec;
typedef struct { i16 cmd; i16 x; } ButtonSlot;
/* A colony. bells is the bells it has accumulated and bells_cap the figure they are measured
 * against: the Sons of Liberty percentage is bells * 100 / bells_cap, and each turn bells_cap
 * loses a 64th and gains twice the population. bonus_hammers counts the hammers the colony
 * got without building them (a rush purchase, 10 for a road finished nearby); nothing reads
 * it back. */
typedef struct { u8 x, y; char name[24]; u8 nation; u8 ai_flags; u8 status; u8 ai_plan_flags; u8 want_defenders_threat; i8 pop; u8 jobs[32]; u8 spec[32]; u8 teach_progress[16]; i8 slots[20]; i8 flags[6]; i8 flags2[2]; i8 ai_idle_turns; i8 wanted_goods; i8 want_defenders; i8 turns_since_ship; i16 changed; i16 hammers; i8 item; u8 warehouses; u8 capitols; u8 mine; i16 bonus_hammers; i16 stock[16]; u8 hist_pop[4]; u8 hist_bld[4]; i32 bells; i32 bells_cap; } Colony;
typedef struct { i16 start, end; } CreditCue;
typedef struct { u16 handle; void *ctl; i16 shared_bitmaps; u16 bmp0, bmp1, bmp2; i16 hot, pressed, f12; void *data; i16 f18, f1a, kind; } CtrlBlock;
typedef struct { char name[0x18]; i16 nation, indep, granted, year, season, difficulty, raw_score, rating, rank; } FameEntry;
typedef struct { i16 name; u8 cat; u8 weight[3]; } Father;
typedef struct { i32 size; i16 type; i16 chunks; char reserved[8]; } FlicFrame;
typedef struct { char pad[0x100]; i16 count; } FontFile;
typedef struct { u16 hfont; i16 owned; } FontRec;
typedef struct { i16 f0; i16 step; i16 phase; i16 awaiting_human; i16 nation; i16 unit_held; } GameBrains;
/* The game's state, as a save file holds it after its 16-byte header. */
typedef struct { i16 tutorial_flags, game_flags, colony_options, hint_flags, america_map, year, season, turn_counter, view_mode, selected_unit, turn_nation, view_nation, player_nation, settlement_count, unit_count, colony_count, trade_route_count, reveal_all, view_override; u8 difficulty; i8 year_hi, year_lo; i8 father_owner[25]; i16 game_in_progress, human_turn_active, end_of_turn_pending; i16 war_started[4]; i16 rebel_sentiment, mother_country_idx, intervention_nation, merc_nation, rebel_step; i16 royal_forces[4]; i16 force[4]; i16 base_sales[16]; u8 woodcuts_seen[4]; } GameState;
typedef struct { i8 start_lo, start_hi, price_min, price_max, spread, up, down, drift; u8 shift; char f9; } GoodsRec;
typedef struct { u16 kind[9]; i16 count[9]; i16 part[9]; } GoodsRow;
/* The Windows version's WinG drawing surface. */
typedef struct { u16 handle; u16 hdc; i16 hobj; u16 hbm; u16 old; i16 topdown; i16 w; i16 h; i16 stride; } GrafRec;
typedef struct { u16 handle; } Handle;
typedef struct { i16 left, top, w, h; u8 flags; } ImgDesc;
typedef struct { u8 a, b, c, pad; } LeaderFace;
/* A tribe's level of civilisation from the LEVELS table, e.g. "Semi-Nomadic, Camp, Camps":
 * its name, and the words for one settlement and for several. */
typedef struct { i16 name, settlement, settlements; } LevelRec;
typedef struct { i16 hwnd; char rest[0x90]; } ListItem;
typedef struct { char pad[4]; i32 *head; i32 *cur; } ListIter;
typedef struct { char pad[0x26]; i16 count; char pad2[6]; ListItem *items; } ListWin;
typedef struct { char pad0[2]; i16 id; char pad[0x1c - 4]; void (*handler)(i16 id); } ListboxHandlers;
/* A Windows file: whether it is open for writing, its handle and its OFSTRUCT. */
typedef struct { i16 writable; i16 handle; char ofs[0x88]; } MSFile;
typedef struct { u16 handle; u16 hwnd; u16 hdc; i16 focus; u16 hbrush; u16 hpal; u16 hcursor; void *pattern; i32 f12; void *icon; i16 drag_by_client; i16 size_by_edges; i16 drag_height; i16 edge_width; i16 destroyed; u16 flags; } MSWin;
typedef struct { i16 left, top, right, bottom; } MapRect;
/* The map's width and height, in squares: the first four bytes of a .MP file. */
typedef struct { i16 w, h; } MapSize;
typedef struct { char pad0[4]; u16 devid; char pad6[8 - 6]; u32 parm; } Mci;
typedef struct { char pad[4]; u16 id; } MciDev;
typedef struct { u8 kind; char owns_handle; i16 handle; void *ptr; u16 used; u16 size; u16 avail; } MemBlock;
typedef struct { u16 hwnd; RECT rect; i16 index; i16 style; char text[0x8e - 0xe]; char accel; char pad8f; i16 mark; } MenuItem;
typedef struct { char active; char pad; i16 frame; i16 count; } MonsterSlot;
/* A European nation; support is its rebel support. */
typedef struct { u8 flags; i8 tax; u8 recruit[3]; u8 grants_taken; u8 recruit_count; u8 father_bits[4]; char pad_0b[1]; i16 bells; i16 bells_total; i16 last_tax_delta; i16 next_father; u8 fathers; char pad_15[1]; i16 f16; u8 villages_burned; u8 support; u8 shown; u8 f1b[2]; char pad_1d[1]; i16 artillery; i16 boycott; i32 royal_gold; i32 net_take; i32 gold; i16 crosses; i16 need; u8 start_x; u8 start_y; u8 rel[4]; char pad_38[8]; u8 timer[4]; u8 seed; u8 seed_add; i16 founded; u8 artillery_credit; u8 muskets; u16 horses; i8 price[16]; i16 drift[16]; i32 trade_gold[16]; i32 trade_tons[16]; i32 sold[16]; } Nation;
/* A European nation's leader, its country, and who plays it. */
typedef struct { char leader[24]; char country[24]; u8 once_flags; i8 control; i16 colony_names_used; } NationText;
typedef struct { i16 version; i16 count; struct { u8 r, g, b, reserved; } e[256]; u16 hpal; } Palette;
/* A Colonizopedia list entry. */
typedef struct PediaEntry { i16 name, category, index; } PediaEntry;
typedef struct { i16 colony; i16 value; i8 loads; u8 troops; } Pickup;
typedef struct { i16 id; RECT rect; i16 state; } PlacementRec;
typedef struct { i8 x, y, kind, priority; } Plan;
typedef struct { i16 x, y; } Point;
/* The Windows version's popup entry field. */
typedef struct PopupEdit { i16 f0; i16 width; i16 maxlen; i16 index; char *label; char *buf; struct PopupEdit *next; } PopupEdit;
typedef struct PopupItem { i16 flags; i16 id; i16 index; char *text; struct PopupItem *next; struct PopupItem *prev; } PopupItem;
typedef struct PopupLine { i16 flags; char *text; struct PopupLine *next; } PopupLine;
typedef struct { i16 f0; i16 width; i16 height; i16 stride; char pad[0x18 - 8]; u8 *bits; } Port;
typedef struct { i16 x, y; } Pos;
typedef struct { u8 flags[3]; u8 pad; i16 v; i16 lo; i16 hi; } RGBTreeNode;
/* A Windows radio button: key is its mnemonic and accel the mnemonic's position in the label
 * (-1: none). */
typedef struct { u16 hwnd; RECT rc; i16 index; i16 enabled; char label[0x80]; char key; char pad_8f; i16 accel; } RadioItem;
typedef struct { i16 colony, amount, count; } Rec6;
typedef struct { char f0; u8 type; char tax_cut, f3; i16 price; } Recruit;
typedef struct { i16 f0; i16 handle; void *data; } Resource;
typedef struct { i16 colony; char rest[8]; } Stop;
typedef struct { char name[32]; u8 sea; u8 count; Stop stops[4]; } RouteRec;
typedef struct { i16 num, den; i32 stamp; i16 map[0x400]; } ScaleSlot;
/* An entry of the settlement table: a native village. */
typedef struct { u8 x; u8 y; u8 owner; u8 flags; u8 size; i8 mission; i8 growth; i8 refused; i8 sold; i8 bought; i16 anger[4]; } Settlement;
typedef struct { i16 name; i16 expert; i16 school; i16 price; } SpecRec;
typedef struct { u16 name; u8 movement; u8 defence; u8 work_turns; u8 value; char best_goods_value; u8 yield[5]; u8 lumber; char pad_0d[3]; } TerrainRec;
typedef void (*TimerFn)(i16 slot, i16 count);
/* A native tribe; the game indexes the table by nation - 4. */
typedef struct { u8 x, y; u8 level; u8 flags; u8 f4; u8 count; u8 f6; i8 muskets; i8 horse_herds; char f9; i16 horses; i16 silver_base; i16 goods_drift[16]; i16 mood[4]; i8 calm[4]; u8 b3a[12]; i16 anger[4]; } Tribe;
/* A tribe's words from the TRIBES table, e.g. "Incas, Inca, Jewelled Relics": name is the
 * plural ("Incas"), adjective the singular ("Inca"), treasure what its treasure is called.
 * The game indexes the table by nation number, tribes being nations 4 and up. Where a
 * European nation contributes an adjective to a sentence, a tribe contributes its plural
 * name: the asymmetry is the game's. */
typedef struct { i16 name, adjective, treasure; } TribeName;
/* A unit: units.h describes its fields. */
typedef struct { u8 x, y, type, flags, flags2, moves_used; i8 home; i8 mission; u8 orders, dest_x, dest_y; i8 last_dir; u8 cargo_count; u8 cargo[3]; union { u8 amount[6]; struct { u8 unused, work_turns; i8 work_dir; u8 unused13, trade_loaded, tools; } w; struct { u8 unused10[2]; i16 turn_stamp; } n; } hold; u8 counter; char spec; i16 link, link2; } Unit;
/* A unit type, one row of the UNITS table: units.h describes it. */
typedef struct { i16 name; u8 icon, b3, moves, defence, attack, holds, size, hammers, tools, guns, hull, bits; } UnitType;
typedef struct { u16 handle; u16 hdc; u16 hpal; u16 hbitmap; u16 oldbm; } WinGRec;
typedef struct { char pad0[0xe]; i16 height; i16 rowbytes; } Wing;

#endif
