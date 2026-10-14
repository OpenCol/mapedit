/* sort.c -- sorting.
 *
 * Two small sorts, both used by the palette code (ui/palette.c): sort_insertion orders 32-bit
 * keys and sort_insertion_8 byte keys, each carrying a payload array along. Both repeat the
 * original's steps exactly, since those decide where equal keys end up and the port must put
 * them in the same places.
 */
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
#pragma pack(2)   /* records on 2-byte boundaries, as in 1994: the file formats depend on it */

/* Sorts the n keys ascending, moving vals[i] wherever keys[i] goes. Each pass looks for the
 * first key bigger than the one after it, takes it out (closing the gap) and puts it back
 * just AFTER the first remaining key that is bigger than it -- one place later than an
 * insertion sort would -- or at the end when there is none; then it starts again from the
 * beginning. It stops when a pass finds the keys in order.
 * The pass walks byte offsets: koffj into keys and voff into vals at the key looked at (i8),
 * and bytesj counts the bytes of vals after it. */
void sort_insertion(i16 n, i16 *vals, i32 *keys)
{
    i16 koffj;
    i32 key7;
    i16 flag6;
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

/* Sorts the n byte keys ascending (unsigned), moving payload[i] wherever key[i] goes. It finds
 * the first i whose key[i+1] is below key[i], takes key[i+1] and its payload out (closing the
 * gap), puts them back before the first key not below it, and scans again from that i. */
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
