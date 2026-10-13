/* sort.c -- sorting. */
#include <string.h>

#include "msc.h"
#include "error.h"
#include "font.h"
#include "loader.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "sort.h"
#include "units.h"
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

/* sort_insertion_8 -- 12e0:0008: sort n byte keys ascending, carrying a payload byte each.
   Exactly the original's steps, which decide the order of equal keys: find the first i whose
   key[i+1] < key[i] (unsigned); lift key[i+1] and its payload out, closing the gap; put them
   back before the first key not below it; scan again from that i. */
void sort_insertion_8(i16 n, char *payload, u8 *key)
{
    i16 i = 0;
    for (;;) {
        i16 s = i, last = (i16)(n - 1), cnt, k;
        u8 al;
        char dl;
        while (s < last && !(key[s + 1] < key[s]))
            i = ++s;
        if (s >= last)
            return;
        al = key[s + 1];
        cnt = (i16)(last - s - 1);
        memmove(&key[s + 1], &key[s + 2], (size_t)cnt);
        dl = payload[s + 1];
        memmove(&payload[s + 1], &payload[s + 2], (size_t)cnt);
        for (k = 0; k < last && al > key[k]; k++)
            ;
        cnt = (i16)(last - k);
        if (cnt) {
            memmove(&key[k + 1], &key[k], (size_t)cnt);
            memmove(&payload[k + 1], &payload[k], (size_t)cnt);
        }
        payload[k] = dl;
        key[k] = al;
    }
}

/* MODULE sort_3: MAPEDIT.EXE segment 12e0 (sort_3.c), file 0x14408-0x144c8, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn sort_insertion_8 0x14408
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0d05).
 *
 * SORTING BYTE KEYS WITH A BYTE PAYLOAD (root code, frame 0d05). sort_insertion_8(n, idx, keys)
 * (_fastcall: n in ax, the far arrays on the stack, popped by the callee) is sort_16x16
 * (root_0cf8.c) for byte keys: the same hand-written insertion sort, byte for byte except
 * the element size. `pad` only gives the frame its 16 bytes.
 */

/* sort_insertion_8 -- 12e0:0008, 192 bytes */
/* PORT: sort_insertion_8 is in port/ (inline assembly) */
