/* Imported from mapedit-decomp src/madsdev/sort_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE sort_1: MAPEDIT.EXE segment 12c7 (SORT_1.C), file 0x1427e-0x14408, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn sort_insertion 0x1427e
 *
 * No VICEROY twin (VICEROY's sort_16x16, root_0cf8, is hand-written asm): reconstructed
 * from the shipped bytes.
 *
 * sort_insertion(n, vals, keys) (_fastcall; n in ax, the two far arrays on the stack) sorts
 * n long keys ascending and carries an int payload along. Each pass scans for the first key
 * that is bigger than its successor, lifts that pair out (two _fmemmoves close the gap),
 * finds the first key bigger than it, opens a gap there (two more _fmemmoves) and drops it
 * in; a pass that moved something is followed by another. The scan keeps byte offsets into
 * both arrays by hand (koffj, voff, bytesj) rather than indexing.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

/* sort_insertion -- 12c7:000e, 394 bytes */
void sort_insertion(i16 n, i16 *vals, i32 *keys)
{
    i16 koffj;
    i32 key7;
    i16 flag6;
    i16 ul;
    i16 bytesj;
    i16 i8;
    i16 voff;
    i16 val;
    i32 *p4;
    i16 last;
    register i16 k;

    last = n - 1;
restart:
    flag6 = 0;
    i8 = 0;
    if (last > 0) {
        bytesj = (n - 1) * 2;
        voff = 0;
        koffj = 0;
        do {
        if (flag6)
            break;
        p4 = (i32 *)((char *)keys + koffj);
        if (p4[1] < p4[0]) {
            key7 = *p4;
            val = *(i16 *)((char *)vals + voff);
            if (bytesj > 0) {
                msc__fmemmove(p4, p4 + 1, bytesj * 2);
                msc__fmemmove((char *)vals + voff, (char *)vals + voff + 2, bytesj);
            }
            for (k = 0; k < last; k++) {
                if (flag6)
                    break;
                if (keys[k] > key7)
                    flag6 = 1;
            }
            flag6 = -1;
            bytesj = (n - k - 1) * 2;
            if (bytesj > 0) {
                msc__fmemmove(keys + k + 1, keys + k, bytesj * 2);
                msc__fmemmove(vals + k + 1, vals + k, bytesj);
            }
            keys[k] = key7;
            vals[k] = val;
        }
        voff += 2;
        bytesj -= 2;
        koffj += 4;
        i8++;
        } while (i8 < last);
    }
    if (flag6)
        goto restart;
}

