/* From mapedit-decomp src/madsdev/pack_6 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "pal.h"
#include "popup.h"
#include "error.h"
#include "loader.h"
#include "mem.h"
#include "pack.h"
#include "records.h"
#include "units.h"
/* MODULE pack_6: MAPEDIT.EXE segment 1309 (pack_6.c), file 0x1469a-0x149ea, 4 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn far_to_near             0x1469a
 * fn io_call_hook            0x146a4
 * fn pack_set_special_buffer 0x146ba
 * fn pack_data                 0x146da
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl28_772ba).
 *
 * FILE-I/O HOOKS (overlay 28, para 70: the segment starts at 0x772b0, inside the previous
 * module's last function, and this module's code at 0x772ba). DOS-only engine code.
 *   - far_to_near returns the offset half of a far pointer (inline les).
 *   - io_call_hook calls the far hook pack_special_function, if one is set (inline asm, which is why
 *     di is saved).
 *   - pack_set_special_buffer installs two far hooks: pack_special_buffer (DS:0x245a) and pack_special_function
 *     (DS:0x245e).
 *   - ff_decode(method, from_file, to_file, bytes, src, dst) (_fastcall, thunk 181f:2eba)
 *     is the MADSPACK codec driver, a wrapper round the PKWARE Data Compression Library
 *     (the work-buffer sizes are implode's 0x89b8 and explode's 0x311e). It installs the
 *     pascal reader/writer callbacks in g_ff_reader/g_ff_writer (root code at 181f:2f10 /
 *     2f06 for a far buffer / FILE source, 2efc / 2ef2 for the destination; to_file 2 keeps
 *     the writer), sets the byte counters g_ff_in/out(_left), and picks a work size and
 *     codec per method: 0 = compress (g_ff_type 1: 0x71be, else implode 0x89b8), 1 =
 *     expand (g_ff_type 1: three variants, memory-to-memory, to memory, or to the writer;
 *     else explode 0x311e), anything else = stored copy (0x1000). A missing codec pointer
 *     is fatal_error(-29, 3, 24, method, g_ff_type). The work buffer is pack_special_buffer when
 *     set (so that "hook" is really a caller-supplied work block), otherwise
 *     far_alloc_tagged(size, "$pack$"). It then runs ff_codec (181f:2ee4, ovl28_775ec.c)
 *     once (expanding to memory) or until the counter reaches 0; a codec error is
 *     fatal_error(-28, 3, 24, ...). Returns the selected byte count.
 *     `#pragma optimize("elg")` for this function only (si/di register locals). The
 *     `to_file != 1 && to_file != 2` spelling puts the 0x820 arm first, as shipped.
 */


void (*pack_special_buffer)(void) = 0;          /* DS:0x4512 (defined here: before pack_data's "$pack$") */
void (*pack_special_function)(void) = 0;        /* DS:0x4516 */
/* far_to_near -- 1309:000a, 10 bytes */
/* PORT: far_to_near is in port/ (inline assembly) */

/* io_call_hook -- 1309:0014, 22 bytes */
/* PORT: io_call_hook is in port/ (inline assembly) */

/* pack_set_special_buffer -- 1309:002a, 32 bytes */
void pack_set_special_buffer(void (*a)(void), void (*b)(void))
{
    pack_special_buffer = a;
    pack_special_function = b;
}
/* pack_data -- 1309:004a, 784 bytes */


i32 pack_data(i16 method, i16 from_file, i16 to_file, i32 bytes,
                         void *src, void *dst)
{
    i32 *count;
    i16 kind;
    i32 *left;
    i16 r;

    kind = 0;
    if (from_file == 0) {
        pack_read_routine = pack_read_memory;
        pack_read_memory_ptr = src;
    } else {
        pack_read_routine = pack_read_file;
        pack_read_file_handle = (MSC_FILE *)src;   /* PORT: the whole pointer (1994: its offset, far_to_near) */
    }
    if (to_file == 0) {
        pack_write_routine = pack_write_memory;
        pack_write_memory_ptr = dst;
    } else {
        pack_write_routine = pack_write_file;
        pack_write_file_handle = (MSC_FILE *)dst;   /* PORT: the whole pointer (1994: its offset, far_to_near) */
    }
    pack_read_count = pack_write_count = 0;
    switch (method) {
    default:
        pack_buffer_size = 0x1000;
        pack_write_size = pack_read_size = bytes;
        left = &pack_read_size;
        count = &pack_write_count;
        break;
    case 0:
        if (pack_strategy == 1) {
            pack_buffer_size = 0xd1d0;
            if (pack_pFABcomp_routine == 0)
                error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
        } else {
            pack_buffer_size = 0x89b8;
            if (pack_implode_routine == 0)
                error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
        }
        pack_read_size = bytes;
        pack_write_size = -1L;
        left = &pack_read_size;
        count = &pack_read_count;
        break;
    case 1:
        pack_read_size = -1L;
        pack_write_size = bytes;
        left = &pack_write_size;
        if (pack_strategy == 1) {
            if (from_file == 0 && to_file == 0 && pack_pFABexp2_routine != 0) {
                count = &bytes;
                pack_buffer_size = 4;
                kind = 2;
            } else if (to_file != 1 && to_file != 2) {
                count = &bytes;
                pack_buffer_size = 0x822;
                kind = 1;
                if (pack_pFABexp1_routine == 0)
                    error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
            } else {
                count = &pack_write_count;
                pack_buffer_size = 0x382e;
                kind = 0;
                if (pack_pFABexp0_routine == 0)
                    error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
            }
        } else {
            count = &pack_write_count;
            pack_buffer_size = 0x311e;
            if (pack_explode_routine == 0)
                error_report(-29, 3, 24, (i32)method, (i32)pack_strategy);
        }
        break;
    }
    pack_buffer = 0;
    if (pack_special_buffer == 0) {
        pack_buffer = mem_get_name((i32)pack_buffer_size, "$pack$");
        if (pack_buffer == 0) {
            *count = 0;
            goto out;
        }
    } else
        pack_buffer = (void *)pack_special_buffer;
    if (method == 1 && to_file == 0) {
        r = pack_a_packet(method, kind);
        if (r != 0) {
            *count = 0;
            error_report(-28, 3, 24, (i32)(method * 1000 + kind), (i32)r);
        }
    } else {
        while (*left > 0) {
            if (pack_a_packet(method, 0) != 0) {
                *count = 0;
                error_report(-28, 3, 24, (i32)method, (i32)to_file);
                break;
            }
        }
    }
out:
    if (pack_special_buffer == 0) {
        if (pack_buffer != 0)
            mem_free(pack_buffer);
    } else
        io_call_hook();
    return *count;
}

