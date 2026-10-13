/* pal.h -- the functions, data and records of cycle_1.c, misc.c, pal_1.c, pal_4.c, pal_8.c, rest.c. */
#ifndef MAPEDIT_PAL_H
#define MAPEDIT_PAL_H

#include "msc.h"

#pragma pack(push, 2)
typedef struct { u8 count, pos, start, speed; } CycleRange;
typedef struct { i16 n; CycleRange r[8]; } ColorCycle;
typedef struct { u8 r, g, b, index, extra, flags; } PalEntry;
typedef struct { i16 n; PalEntry e[1]; } PalList;
typedef struct { i16 n; i16 idx[3]; } Shadow;
#pragma pack(pop)

void cycle_init(ColorCycle *ranges, i16 on);
void pal_activate_shadow(Shadow *sh);
i16 pal_allocate(i16 flags, PalList *list, Shadow *sh);
void pal_call_manager(i32 fn, i16 code);
void pal_compact(i16 flag, i16 n, i16 *list);
i16 pal_count_free(i16 *first);
i16 pal_deallocate(i16 flag);
i16 pal_find_free_flag(void);
void pal_init(i16 bottom, i16 top);
void pal_init_shadow(Shadow *sh, PalList *list);
void pal_lock(void);
void pal_shadow_sort(Shadow *sh, PalList *list);
void pal_unlock(void);
void pal_grey(i16 start, i16 count, char *pal, i16 lo, i16 hi);
void pal_interface(char *pal);
void pal_white(char *pal);
i16 pal_load_2(char *name, char *pal);

extern ColorCycle cycle_list;
extern i32 cycle_timing[8];
extern i16 cycling_active;
extern i16 cycling_delay;
extern u8 cycling_palette[0x300];
extern i16 cycling_threshold;
extern i16 total_cycle_colors;
extern i32 color_status[256];
extern i16 flag_used[32];
extern u8 master_palette[0x300];
extern Shadow *master_shadow;
extern i16 pal_manager_active;
extern i16 pal_manager_colors;
extern i32 pal_manager_update;
extern i16 palette_ever_initialized;
extern i16 palette_high_search_limit;
extern i16 palette_locked;
extern i16 palette_low_search_limit;
extern i16 palette_reserved_bottom;
extern i16 palette_reserved_top;
i16 pal_get_hash(u8 *b, u8 *a);
void cycle_colors(void);
i16 magic_hash_color(const void *color);

#endif
