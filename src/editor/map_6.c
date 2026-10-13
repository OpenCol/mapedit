/* Imported from mapedit-decomp src/editor/map_6 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE map_6: MAPEDIT.EXE segment 09c4 (map_6.obj), file 0xb242-0xb59a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn map_find_continents 0x0b242
 *
 * Written from `ovl20_63880` (VICEROY.EXE map_find_continents, near 0.96) against
 * the shipped bytes. The editor's copy of the region labeller.
 */
#include "records.h"

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern MapSize map_file;                    /* DS:0x4b12 */
extern i16 *sprite_base;                /* DS:0x008c */
extern DosPort scr_continent;               /* DS:0x5eba */
extern u8 *continent;        /* DS:0x04b0 */
extern i16 region_size[16];                 /* DS:0x5270 */
extern void buffer_fill(char c, DosPort port);
extern i16 on_map(i16 x, i16 y);
extern i16 terrain_is_ocean(i16 x, i16 y);
extern i16 build_check(void);
extern i16 keys_get(void);
extern void load_main_sprites(void);

#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))

extern i16 pad_0, pad_1, pad_2, pad_3, pad_4;
/* map_find_continents -- 09c4:0002, 856 bytes */
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

