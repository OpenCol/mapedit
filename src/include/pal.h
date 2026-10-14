/* pal.h -- the palette: sharing it among sprite series, and loading a palette file.
 *
 * Defined in ui/palette.c. master_palette is the 256 colours the editor shows, three bytes
 * each (red, green, blue, 0..3Fh). The palette manager shares it among sprite series: a
 * series' colour list (a PalList) asks for colours, and pal_allocate gives each an entry --
 * an existing one of the same or the nearest colour, or a free one it fills -- under an
 * allocation "flag" (0..31). color_status keeps a bit per flag for every entry. pal_load_2
 * reads a palette file such as VICEROY.PAL. The DAC itself is mcga.h.
 */
#ifndef MAPEDIT_PAL_H
#define MAPEDIT_PAL_H

#include "msc.h"

#pragma pack(push, 2)
/* Colour cycling's records, which nothing uses. A range of entries to cycle: how many, the
 * current rotation step, the first entry, and the timer ticks between steps. */
/* Up to eight ranges, n of them in use. */
/* One colour of a request list: the colour wanted (r, g, b), the entry pal_allocate gave it
 * (index), whether an exact request reserves that entry to this series (extra), and how it
 * may be placed (flags): 80h may share an entry another series needs, 40h may take the
 * nearest colour when the palette is full, 20h always takes the nearest colour, 10h a shadow
 * colour, 08h always the transparent entry FDh. */
typedef struct { u8 r, g, b, index, extra, flags; } PalEntry;
/* A request list: n colours. The record runs on past e[0] for the rest. */
typedef struct { i16 n; PalEntry e[1]; } PalList;
/* A shadow: up to three colours (n of them) that take fixed places, as indices into a list
 * or into the palette. */
typedef struct { i16 n; i16 idx[3]; } Shadow;
#pragma pack(pop)

i16 pal_allocate(i16 flags, PalList *list, Shadow *sh);
i16 pal_count_free(i16 *first);
i16 pal_find_free_flag(void);
void pal_init_shadow(Shadow *sh, PalList *list);
void pal_shadow_sort(Shadow *sh, PalList *list);
i16 pal_load_2(char *name, char *pal);

extern i16 cycling_active;               /* cycling is on; popup.c clears it around
                                            sprite loads, and nothing cycles */
extern i32 color_status[256];            /* per entry: a bit per flag using it; 0 free */
extern i16 flag_used[32];                /* -1 for each allocation flag taken */
extern u8 master_palette[0x300];         /* the 256 colours shown */
extern Shadow *master_shadow;            /* the shadow pal_allocate uses, or 0 */
extern i16 palette_high_search_limit;
extern i16 palette_low_search_limit;
extern i16 palette_reserved_bottom;      /* stock entries at the bottom (never set) */
extern i16 palette_reserved_top;         /* and at the top */
i16 pal_get_hash(u8 *b, u8 *a);
i16 magic_hash_color(const void *color);

#endif
