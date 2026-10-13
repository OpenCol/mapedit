/* From mapedit-decomp src/madsdev/loader_2 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "fileio.h"
#include "mem.h"
#include "pack.h"
#include "records.h"
#include "units.h"
#include "loader.h"
/* MODULE loader_2: MAPEDIT.EXE segment 1258 (loader_2.c), file 0x13b82-0x13db0, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn loader_read  0x13b82
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * loader_read(size, buf, count, fh) (_fastcall; size in ax:dx, the rest on the stack) is
 * fread for a MADSPACK FileIO: it reads the file's next entry (fh->next, bumped) into buf.
 *   - kind 1 (the file is in EMS): ems_copy_it_down from the entry's page/offset;
 *   - kind 2 (in XMS): xms_copy from the handle's position, which it advances;
 *   - otherwise from disk: the entry's type sets pack_strategy; a type-0 entry is
 *     stored (pack_data method 2, straight from the FILE), any other is packed and is
 *     read whole into a mem_get block and unpacked from there (method 1); when the block
 *     cannot be had, it is unpacked from the FILE instead and the file position is moved
 *     past the entry (fgetpos/fseek).
 * It frees the block and returns 1 when it got size bytes, otherwise got / size.
 *
 * `#pragma optimize("elg")` like loader_1.c: fh's offset lives in si for the whole function
 * and the entry index in di. Under it the frame sorts by weight, so the names are free; the
 * `done` flag's store-then-test from memory comes out of "elg" by itself (plain /Ot needs it
 * volatile, which under "elg" moves its slot).
 */



/* loader_read -- 1258:0002, 558 bytes */
i32 loader_read(i32 size, void *buf, i32 count, FileIO *fh)
{
    i32 total;
    i16 kind2;
    void *tmp9;
    fpos_t pos8;
    i32 packed;                /* after pos8: `packed + pos8` loads packed first */
    i16 done4;
    i32 unused6;               /* a 4-byte slot the shipped frame has above done4; never touched */
    i32 got;
    register i16 i;

    tmp9 = 0;
    done4 = 0;
    if (size == 0)
        return 0;
    if (count == 1)
        total = size;
    else
        total = size * count;
    i = fh->next++;
    got = 0;
    pack_strategy = (u8)fh->ent[i].type;
    packed = fh->ent[i].packed;
    kind2 = pack_strategy == 0 ? 2 : 1;
    if (kind2 == 1) {
        tmp9 = mem_get(packed);
        if (tmp9 != 0) {
            if (fileio_fread_f(tmp9, packed, 1L, fh->fp) == 0)
                goto out;
            got = pack_data(kind2, 0, 0, total, tmp9, buf);
            done4 = -1;
        }
    }
    if (done4 == 0) {
        msc_fgetpos(fh->fp, &pos8);
        got = pack_data(kind2, 1, 0, total, (void *)fh->fp, buf);
        if (kind2 == 1)
            msc_fseek(fh->fp, packed + pos8, 0);
    }
out:
    if (tmp9)
        mem_free(tmp9);
    if (got == size)
        return 1;
    return got / size;
}

