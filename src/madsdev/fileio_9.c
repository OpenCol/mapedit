/* Imported from mapedit-decomp src/madsdev/fileio_9 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE fileio_9: MAPEDIT.EXE segment 0bea (fileio_9.c), file 0xd4a8-0xd5b0, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_fwrite_f 0x0d4a8
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl28_775ec).
 *
 * WRITING A FAR BLOCK TO A FILE (overlay 28, para 121: its own segment, 0x775e0-0x776f0).
 * DOS-only engine code.
 *   - fileio_fwrite_f(p, size, n, fp) (_fastcall, thunk 181f:2c9c) writes n items of `size`
 *     bytes from a far block. It makes the FILE unbuffered first (setbuf(fp, 0) unless
 *     _IONBF is set), then writes in chunks of at most 0xf000 bytes straight to the handle
 *     with _dos_write (0d1d:0e9d), advancing p as a HUGE pointer. Each chunk is
 *     min(total - written, 0xf000), where `written` counts the bytes DOS reported, while
 *     the loop counter `done` adds the requested chunk. A _dos_write error ends the loop.
 *     It returns 1 when written == size, otherwise written / size (so, like ff_read, the
 *     count is right only for n == 1 or a complete write); size 0 returns 0 by falling off
 *     the end (ax = dx = 0 from the test).
 *
 * `#pragma optimize("elg")`. The pointer arithmetic is MSC's own huge-pointer code
 * (`sub bx,bx; add off,ax; adc bx,dx; mov cx,__AHSHIFT; shl bx,cl; add seg,bx`): the
 * parameter is `char huge *p` and the statement is `p += got`. The `mov cx,0xc` immediate is
 * the __AHSHIFT fixup, which the linker resolves to 12; a literal `<< 12` gives MSC's byte
 * trick (mov dh,dl; shl dh,4) instead. Ten pads set the _aFlmul push order (n first);
 * the `written == size` operand order then needs ten unused locals before `written`
 * (the only working cell of a 17 x 16 sweep of pads x unused locals).
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;



extern i16 pad_1, pad_2, pad_3, pad_4, pad_5, pad_6, pad_7, pad_8, pad_9, pad_10;
/* fileio_fwrite_f -- 0bea:0008, 264 bytes */
i32 fileio_fwrite_f(char *p, i32 size, i32 n, MSC_FILE *fp)
{
    i32 total;
    i32 done;
    i16 unused_1, unused_2, unused_3, unused_4, unused_5;
    i16 unused_6, unused_7, unused_8, unused_9, unused_10;
    i32 written;
    u16 got;
    u16 chunk;

    written = 0;
    done = 0;
    if (size != 0) {
        if (!(fp->_flag & 4))
            msc_setbuf(fp, 0);
        total = n == 1 ? size : n * size;
        while (done < total) {
            chunk = (u16)(total - written > 0xf000 ? 0xf000 : total - written);
            if (msc__dos_write(fp->_file, p, chunk, &got) != 0)
                break;
            written += got;
            p += got;
            done += chunk;
        }
        if (written == size)
            return 1;
        return written / size;
    }
    return 0;   /* PORT: size 0; the shipped code arrives here with dx:ax = size = 0 */
}

