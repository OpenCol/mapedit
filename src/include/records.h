/* Imported from mapedit-decomp include/records.h by tools/import.py. */
/* records.h -- the game's records, one definition each, for every reconstruction.
 *
 * The window and dialog classes are not here: their C files use C views and their C++
 * files classes, and the two compile differently where a cast crosses a base class.
 * The files no longer declare these records, so this is now their only definition: a
 * member is renamed HERE and at its uses (the bytes do not see a member name), and
 * `shared_records.py --check` keeps any file from declaring one again.
 */
#ifndef RECORDS_H
#define RECORDS_H

#include "w16types.h"

typedef struct { i16 time; i16 count; i16 kind; } AnimEvent;
/* Audio
 * as audio_open_element names it
 */
typedef struct { char pad[0x8c]; void *mci; void (*on_mci_done)(void); } Audio;
typedef struct { char pad[0xc4]; i16 shown_half; } BackdropWin;
typedef struct { u16 w, h; i16 x, y; u8 nplanes; i8 masking; u8 compression; char pad; i16 transparent; char xasp, yasp; i16 pagew, pageh; } Bmhd;
/* BuildNeed
 * 4 bytes at SEG20:0xdca8
 */
typedef struct { u8 building, expert_job, goods, f3; } BuildNeed;
/* BuildingRec
 * 12 bytes
 * 12 bytes at SEG20:0xd422
 */
typedef struct { i16 name; i8 prereq2; i8 prev_level; i8 next_level; i8 kind; i8 family; i8 tools; i8 min_pop; i8 upkeep; i16 hammers; } BuildingRec;
typedef struct { i16 cmd; i16 x; } ButtonSlot;
/* Colony
 * 0xca bytes
 * 0xca
 * 0xca bytes at SEG20:0x7c28
 * SEG20:0x7c28
 * Colony+0xc6 -- the accumulated bells the SoL percentage divides into: `colony_rebel` computes
 * `bells * 100 / this`, and `new_turn_colony` decays it by a 64th each turn and adds twice the
 * population. Five files had called it bells_cap or bells_max after that denominator role
 * Colony+0xc2 is the bells ACCUMULATED and +0xc6 the figure they are measured against:
 * `colony_rebel` holds both and computes `bells * 100 / bells_cap`, and `new_turn_colony`
 * decays +0xc6 by a 64th a turn and adds twice the population. Twenty files had called +0xc6
 * `bells`, which is the OTHER field
 * 0xca at SEG20:0x7c28
 * Colony+0x98 (`bonus_hammers`) is hammers the colony got without building them: the rush
 * purchase in `ColonyWin_colony_hammer_buy` adds what it bought, and `perform_orders_road`
 * adds 10 when a road is finished near the colony. Nothing reads it back. The community save
 * format (pavelbel/smcol_saves_utility) calls it hammers_purchased, "possibly unused".
 */
typedef struct { u8 x, y; char name[24]; u8 nation; u8 ai_flags; u8 status; u8 ai_plan_flags; u8 want_defenders_threat; i8 pop; u8 jobs[32]; u8 spec[32]; u8 teach_progress[16]; i8 slots[20]; i8 flags[6]; i8 flags2[2]; i8 ai_idle_turns; i8 wanted_goods; i8 want_defenders; i8 turns_since_ship; i16 changed; i16 hammers; i8 item; u8 warehouses; u8 capitols; u8 mine; i16 bonus_hammers; i16 stock[16]; u8 hist_pop[4]; u8 hist_bld[4]; i32 bells; i32 bells_cap; } Colony;
typedef struct { i16 start, end; } CreditCue;
/* CtrlBlock
 * 0x1e bytes
 */
typedef struct { u16 handle; void *ctl; i16 shared_bitmaps; u16 bmp0, bmp1, bmp2; i16 hot, pressed, f12; void *data; i16 f18, f1a, kind; } CtrlBlock;
/* FameEntry
 * 0x2a bytes
 */
typedef struct { char name[0x18]; i16 nation, indep, granted, year, season, difficulty, raw_score, rating, rank; } FameEntry;
/* Father
 * 6 bytes at SEG20:0xd6fa
 * 6 bytes
 * 6 bytes at SEG20:0xd6fa, the same struct choose_freedom declares; see fathers.h
 */
typedef struct { i16 name; u8 cat; u8 weight[3]; } Father;
typedef struct { i32 size; i16 type; i16 chunks; char reserved[8]; } FlicFrame;
typedef struct { char pad[0x100]; i16 count; } FontFile;
typedef struct { u16 hfont; i16 owned; } FontRec;
typedef struct { i16 f0; i16 step; i16 phase; i16 awaiting_human; i16 nation; i16 unit_held; } GameBrains;
/* GameState
 * 0x8e bytes: the save file's block at offset 0x10
 */
typedef struct { i16 tutorial_flags, game_flags, colony_options, hint_flags, america_map, year, season, turn_counter, view_mode, selected_unit, turn_nation, view_nation, player_nation, settlement_count, unit_count, colony_count, trade_route_count, reveal_all, view_override; u8 difficulty; i8 year_hi, year_lo; i8 father_owner[25]; i16 game_in_progress, human_turn_active, end_of_turn_pending; i16 war_started[4]; i16 rebel_sentiment, mother_country_idx, intervention_nation, merc_nation, rebel_step; i16 royal_forces[4]; i16 force[4]; i16 base_sales[16]; u8 woodcuts_seen[4]; } GameState;
typedef struct { i8 start_lo, start_hi, price_min, price_max, spread, up, down, drift; u8 shift; char f9; } GoodsRec;
typedef struct { u16 kind[9]; i16 count[9]; i16 part[9]; } GoodsRow;
/* GrafRec
 * the WinG surface record MSInitGraf builds
 * 0x12 bytes
 */
typedef struct { u16 handle; u16 hdc; i16 hobj; u16 hbm; u16 old; i16 topdown; i16 w; i16 h; i16 stride; } GrafRec;
typedef struct { u16 handle; } Handle;
typedef struct { i16 left, top, w, h; u8 flags; } ImgDesc;
typedef struct { u8 a, b, c, pad; } LeaderFace;
/* LevelRec
 * 6 bytes
 * 6 bytes at SEG20:0xcb7c, the LEVELS resource: "Semi-Nomadic, Camp, Camps"
 */
typedef struct { i16 name, settlement, settlements; } LevelRec;
/* ListItem
 * 0x92 bytes
 */
typedef struct { i16 hwnd; char rest[0x90]; } ListItem;
typedef struct { char pad[4]; i32 *head; i32 *cur; } ListIter;
typedef struct { char pad[0x26]; i16 count; char pad2[6]; ListItem *items; } ListWin;
typedef struct { char pad0[2]; i16 id; char pad[0x1c - 4]; void (*handler)(i16 id); } ListboxHandlers;
/* MSFile
 * OFSTRUCT at +4
 */
typedef struct { i16 writable; i16 handle; char ofs[0x88]; } MSFile;
/* MSWin
 * the HWND is at +2
 * 0x26 bytes
 */
typedef struct { u16 handle; u16 hwnd; u16 hdc; i16 focus; u16 hbrush; u16 hpal; u16 hcursor; void *pattern; i32 f12; void *icon; i16 drag_by_client; i16 size_by_edges; i16 drag_height; i16 edge_width; i16 destroyed; u16 flags; } MSWin;
typedef struct { i16 left, top, right, bottom; } MapRect;
/* MapSize
 * the map's width and height
 */
typedef struct { i16 w, h; } MapSize;
typedef struct { char pad0[4]; u16 devid; char pad6[8 - 6]; u32 parm; } Mci;
typedef struct { char pad[4]; u16 id; } MciDev;
typedef struct { u8 kind; char owns_handle; i16 handle; void *ptr; u16 used; u16 size; u16 avail; } MemBlock;
typedef struct { u16 hwnd; RECT rect; i16 index; i16 style; char text[0x8e - 0xe]; char accel; char pad8f; i16 mark; } MenuItem;
typedef struct { char active; char pad; i16 frame; i16 count; } MonsterSlot;
/* Nation
 * 0x13c bytes
 * 316 bytes at SEG20:0xc2f4
 * SEG20:0xc2f4
 * 0x13c bytes; +0x19 the rebel support
 * 316 bytes at SEG20:0xc2f4; this file needs the SIZE and no member
 */
typedef struct { u8 flags; i8 tax; u8 recruit[3]; u8 grants_taken; u8 recruit_count; u8 father_bits[4]; char pad_0b[1]; i16 bells; i16 bells_total; i16 last_tax_delta; i16 next_father; u8 fathers; char pad_15[1]; i16 f16; u8 villages_burned; u8 support; u8 shown; u8 f1b[2]; char pad_1d[1]; i16 artillery; i16 boycott; i32 royal_gold; i32 net_take; i32 gold; i16 crosses; i16 need; u8 start_x; u8 start_y; u8 rel[4]; char pad_38[8]; u8 timer[4]; u8 seed; u8 seed_add; i16 founded; u8 artillery_credit; u8 muskets; u16 horses; i8 price[16]; i16 drift[16]; i32 trade_gold[16]; i32 trade_tons[16]; i32 sold[16]; } Nation;
/* NationText
 * 0x34 bytes: a European nation's leader, country, and who plays it
 */
typedef struct { char leader[24]; char country[24]; u8 once_flags; i8 control; i16 colony_names_used; } NationText;
/* Palette
 * hpal at +0x404
 */
typedef struct { i16 version; i16 count; struct { u8 r, g, b, reserved; } e[256]; u16 hpal; } Palette;
/* PediaEntry
 * the list node PediaWin_AddString builds
 */
typedef struct PediaEntry { i16 name, category, index; } PediaEntry;
typedef struct { i16 colony; i16 value; i8 loads; u8 troops; } Pickup;
/* PlacementRec
 * 12 bytes
 */
typedef struct { i16 id; RECT rect; i16 state; } PlacementRec;
typedef struct { i8 x, y, kind, priority; } Plan;
typedef struct { i16 x, y; } Point;
/* PopupEdit
 * 0x14 bytes
 * the edit item Popup_edit builds
 */
typedef struct PopupEdit { i16 f0; i16 width; i16 maxlen; i16 index; char *label; char *buf; struct PopupEdit *next; } PopupEdit;
/* PopupItem
 * 0x12 bytes
 */
typedef struct PopupItem { i16 flags; i16 id; i16 index; char *text; struct PopupItem *next; struct PopupItem *prev; } PopupItem;
typedef struct PopupLine { i16 flags; char *text; struct PopupLine *next; } PopupLine;
typedef struct { i16 f0; i16 width; i16 height; i16 stride; char pad[0x18 - 8]; u8 *bits; } Port;
typedef struct { i16 x, y; } Pos;
typedef struct { u8 flags[3]; u8 pad; i16 v; i16 lo; i16 hi; } RGBTreeNode;
/* RadioItem
 * 0x92 bytes: key is the mnemonic, accel its position in the label (-1: none)
 */
typedef struct { u16 hwnd; RECT rc; i16 index; i16 enabled; char label[0x80]; char key; char pad_8f; i16 accel; } RadioItem;
typedef struct { i16 colony, amount, count; } Rec6;
typedef struct { char f0; u8 type; char tax_cut, f3; i16 price; } Recruit;
typedef struct { i16 f0; i16 handle; void *data; } Resource;
typedef struct { i16 colony; char rest[8]; } Stop;
/* RouteRec
 * 0x4a bytes at SEG20:0xc804
 * 74 bytes at SEG20:0xc804
 */
typedef struct { char name[32]; u8 sea; u8 count; Stop stops[4]; } RouteRec;
/* ScaleSlot
 * 0x808 bytes
 */
typedef struct { i16 num, den; i32 stamp; i16 map[0x400]; } ScaleSlot;
/* Settlement
 * 18 bytes at SEG20:0xcb9a
 * 0x12 bytes at SEG20:0xcb9c
 * 18 bytes
 * 0x12 bytes, the settlement table from its first byte
 * 0x12 bytes, one entry of the settlement table; DGROUP:0x4c2c points at the current one -- NOT
 * the Tribe above, which this file had been using for both pointers
 * SEG20:0xcb9a
 * 0x12
 * 0x12 bytes
 */
typedef struct { u8 x; u8 y; u8 owner; u8 flags; u8 size; i8 mission; i8 growth; i8 refused; i8 sold; i8 bought; i16 anger[4]; } Settlement;
/* SpecRec
 * 8 bytes, see jobs.h
 * 8 bytes at SEG20:0xd61a
 * 8 bytes
 * 8 bytes at SEG20:0xd61a, see jobs.h
 * see jobs.h
 * 8 bytes to the row at SEG20:0xd61a
 */
typedef struct { i16 name; i16 expert; i16 school; i16 price; } SpecRec;
/* TerrainRec
 * 16 bytes
 * 0x10 bytes
 * 0x10
 */
typedef struct { u16 name; u8 movement; u8 defence; u8 work_turns; u8 value; char best_goods_value; u8 yield[5]; u8 lumber; char pad_0d[3]; } TerrainRec;
typedef void (*TimerFn)(i16 slot, i16 count);
/* Tribe
 * 0x4e bytes at SEG20:0xd182
 * 0x4e bytes: g_tribes[nation - 4] indexes it
 * 0x4e bytes
 */
typedef struct { u8 x, y; u8 level; u8 flags; u8 f4; u8 count; u8 f6; i8 muskets; i8 horse_herds; char f9; i16 horses; i16 silver_base; i16 goods_drift[16]; i16 mood[4]; i8 calm[4]; u8 b3a[12]; i16 anger[4]; } Tribe;
/* TribeName
 * 6 bytes
 * 6 bytes at SEG20:0xd3da, four records before the tribes' own start at 0xd3f2, so indexing it
 * by the nation number lands on tribe n's record. THIS READS +0, THE PLURAL NOUN. The TRIBES
 * resource is "Incas, Inca, Jewelled Relics" and `load_data` stores the columns in order, so +0
 * is "Incas" and +2 "Inca" -- which is what `get_true_adjective` returns. Both functions fall
 * back to the European adjective table for nations under 4, so the asymmetry is the game's: a
 * tribe contributes its plural noun where a European contributes an adjective.
 * 6 bytes at SEG20:0xd3f2; the TRIBES resource is "Incas, Inca, Jewelled Relics", so +2 is the
 * ADJECTIVE -- which is why the European branch falls back to the adjective table
 * 6 bytes at SEG20:0xd3f2, the TRIBES resource: "Incas, Inca, Jewelled Relics"
 */
typedef struct { i16 name, adjective, treasure; } TribeName;
/* Unit
 * 0x1c bytes, DGROUP-far array at SEG20:0xa208
 * 0x1c bytes at SEG20:0xa208
 * SEG20:0xa208
 * 28 bytes at SEG20:0xa208
 * 0x1c bytes
 * 0x1c
 * 0x1c at SEG20:0xa208
 */
typedef struct { u8 x, y, type, flags, flags2, moves_used; i8 home; i8 mission; u8 orders, dest_x, dest_y; i8 last_dir; u8 cargo_count; u8 cargo[3]; union { u8 amount[6]; struct { u8 unused, work_turns; i8 work_dir; u8 unused13, trade_loaded, tools; } w; struct { u8 unused10[2]; i16 turn_stamp; } n; } hold; u8 counter; char spec; i16 link, link2; } Unit;
/* UnitType
 * 14 bytes at SEG20:0x7ac8
 * 14 bytes
 * 14 bytes; load_data
 */
typedef struct { i16 name; u8 icon, b3, moves, defence, attack, holds, size, hammers, tools, guns, hull, bits; } UnitType;
typedef struct { u16 handle; u16 hdc; u16 hpal; u16 hbitmap; u16 oldbm; } WinGRec;
/* Wing
 * +0xe, +0x10
 */
typedef struct { char pad0[0xe]; i16 height; i16 rowbytes; } Wing;

#endif
