/* continents.c -- Find Continents: numbering the land masses and seas of the continent plane.
 *
 * map_find_continents labels the connected regions of the map twice, first the water, then
 * the land, and writes each square's number (1-15) into the continent plane; water and land
 * are numbered independently. Only the playable squares are labelled (the border ring is
 * left 0). The work is done in a scratch block the caller lends through sprite_base: 0x4000
 * region sizes (a word each), then a word label per square.
 */
#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
#include "font.h"
#include "game.h"
#include "menu.h"
#include "popup.h"
#include "sprite.h"
#include "buffer.h"
#include "keys.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "units.h"
#include "records.h"


/* Numbers the water regions, then the land regions, into the continent plane, and copies the
 * land regions' sizes to region_size[0..15]. The whole plane is cleared first, so every owner
 * nibble becomes 0. Returns bit 0 set when there were more than 15 land masses and bit 1 when
 * there were more than 15 seas; the extra regions all get number 15.
 *
 * The labelling scan runs down the rows and, within a row, from right to left. A square joins
 * the region of the square scanned just before it and of the three squares above it
 * (diagonals connect); when two regions meet, the higher number is relabelled as the lower
 * and its size added in. A square that joins nothing opens a new region, the lowest free
 * number. Then a second scan, left to right, keeps numbers 1-15 and gives each region above
 * 15 the lowest free number.
 *
 * Quirks of the original, kept: the run carries over from the end of one row (its leftmost
 * square) to the start of the next (its rightmost), so those two squares count as touching. A
 * land region opened in the top or bottom playable row starts its number search above 16, so
 * an arctic strip touching no other land is numbered after the rest. If 16383 labels were
 * ever in use at once the scan would print "Bachunga" and wait for a key; with at most 12000
 * squares it cannot happen. */
i16 map_find_continents(void)
{
    i16 *sizes;
    u16 *lp2;
    i16 y3;
    i16 k8;
    i16 lo;
    i16 i1;
    i16 x1;
    i16 unused4;
    i16 hi6;
    i16 d2;
    i16 cid5;
    i16 overflow;
    i16 placed9;
    i16 reg3;
    i16 j6;
    i16 pass6;
    i16 next_id4;
    u16 *lab;
    u8 *dst4;

    overflow = 0;
    unused4 = 0;
    sizes = sprite_base;
    lab = (u16 *)(sizes + 0x4000);
    buffer_fill(0, scr_continent);
    for (pass6 = 1; pass6 >= 0; pass6--) {
        msc__fmemset(sizes, 0, 0x8000);
        msc__fmemset(lab, 0, map_file.h * map_file.w * 2);
        next_id4 = 0;
        cid5 = 0;
        placed9 = 0;
        for (y3 = 1; map_file.h - 1 > y3; y3++) {
            for (x1 = map_file.w - 2; x1 >= 1; x1--) {
                if (on_map(x1, y3) == 0 || terrain_is_ocean(x1, y3) != pass6) {
                    placed9 = 0;
                    cid5 = 0;
                    continue;
                }
                for (d2 = -1; d2 <= 1; d2++) {
                    reg3 = lab[(y3 - 1) * map_file.w + x1 + d2];
                    if (reg3 != 0) {
                        lo = reg3;
                        if (cid5 != 0 && cid5 != reg3) {
                            lo = min(reg3, cid5);
                            hi6 = max(reg3, cid5);
                            sizes[lo] += sizes[hi6];
                            sizes[hi6] = 0;
                            for (k8 = 1; k8 <= y3; k8++)
                                for (j6 = 1; j6 <= map_file.w; j6++) {
                                    lp2 = lab + k8 * map_file.w + j6;
                                    if (*lp2 == hi6)
                                        *lp2 = lo;
                                }
                        }
                        cid5 = reg3 = lo;
                    }
                }
                if (cid5 == 0) {
                    if (placed9 == 0) {
                        next_id4 = 0;
                        if ((y3 == 1 || map_file.h - 2 == y3) && pass6 == 0)
                            next_id4 = 0x10;
                        while (sizes[++next_id4] != 0 && next_id4 < 0x4002)
                            ;
                        if (next_id4 >= 0x4000) {
                            next_id4 = 0x3fff;
                            msc_printf("Bachunga\n");
                            keys_get();
                        }
                    }
                    cid5 = next_id4;
                }
                lp2 = lab + y3 * map_file.w + x1;
                *lp2 = cid5;
                sizes[cid5]++;
                placed9 = 1;
            }
            build_check();
        }
        lp2 = lab;
        dst4 = continent;
        for (y3 = 0; y3 < map_file.h; y3++) {
            for (x1 = 0; x1 < map_file.w; x1++) {
                if (*lp2 != 0) {
                    if (*lp2 > 0xf) {
                        if (sizes[*lp2] <= 0) {
                            *lp2 = sizes[*lp2] > 0 ? sizes[*lp2] : ~sizes[*lp2] + 1;
                        } else {
                            next_id4 = 0;
                            while (sizes[++next_id4] != 0)
                                ;
                            if (next_id4 <= 0xf) {
                                sizes[next_id4] = sizes[*lp2];
                                sizes[*lp2] = -next_id4;
                                *lp2 = next_id4;
                            } else {
                                overflow |= 1 << pass6;
                                *lp2 = 0xf;
                            }
                        }
                    }
                    *dst4 = (u8)*lp2;
                }
                lp2++;
                dst4++;
            }
            build_check();
        }
    }
    for (i1 = 0; i1 < 0x10; i1++)
        region_size[i1] = sizes[i1];
    load_main_sprites();
    return overflow;
}

