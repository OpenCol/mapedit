/* pack.c -- packing and unpacking: the stream plumbing around the FAB codec (fab.c).
 *
 * pack_data moves a block of bytes from a source to a destination through a codec: method 0
 * compresses, method 1 expands, any other method copies the bytes unchanged (a "stored"
 * entry). The source and the destination are each either memory or an open file.
 *
 * The codecs never see the source or the destination. They pull bytes through a reader and
 * push them through a writer (FFIO callbacks), and pack_data installs the matching pair in
 * pack_read_routine and pack_write_routine:
 *
 *     pack_read_memory    from pack_read_memory_ptr        pack_read_file    from a FILE
 *     pack_write_memory   to pack_write_memory_ptr         pack_write_file   to a FILE
 *
 * Each callback is asked for *n bytes and moves no more than its allowance (pack_read_size,
 * pack_write_size) has left; a negative allowance means no limit. It subtracts what it moved
 * from the allowance and adds it to its counter (pack_read_count, pack_write_count).
 *
 * Two codec families plug in through function pointers: PKWARE's implode/explode
 * (pack_implode_routine, pack_explode_routine) and MicroProse's FAB (pack_pFAB*_routine).
 * pack_strategy 1 selects FAB. The editor installs only FAB (pack_enable_pfab, at startup); the
 * PKWARE pointers stay null, and asking for a codec that is not installed is a fatal error.
 * The editor's one caller is the MADSPACK loader (loader.c), which sets pack_strategy from each
 * entry's type and always unpacks into memory.
 */
#include <string.h>

#include "msc.h"
#include "error.h"
#include "fileio.h"
#include "font.h"
#include "loader.h"
#include "mem.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "sprite.h"
#include "units.h"
#pragma pack(2)   /* records on 2-byte boundaries, as the 1994 compiler laid them out */

/* Where pack_read_memory reads next. */
char *pack_read_memory_ptr;

/* Where pack_write_memory writes next. */
char *pack_write_memory_ptr;

/* pack_read_file -- the reader over a file: reads up to *n bytes from pack_read_file_handle
 * into data, no more than pack_read_size allows, and returns how many it got (fewer at the end
 * of the file). *n is not written back. */
u16 pack_read_file(void *data, u16 *n)
{
    char *buf = data;
    register u16 got;

    if (pack_read_size >= 0)
        got = (u16)min((i32)*n, pack_read_size);
    else
        got = *n;
    if (got != 0) {
        got = (u16)fileio_fread_f(buf, 1L, (i32)got, pack_read_file_handle);
        if (pack_read_size > 0)
            pack_read_size -= got;
        pack_read_count += got;
    }
    return got;
}

/* The file pack_read_file reads from. */
MSC_FILE *pack_read_file_handle;

/* pack_write_file -- the writer to a file: writes up to *n bytes of data to
 * pack_write_file_handle, no more than pack_write_size allows. Returns the count, or 0 when the
 * write fails (and then counts nothing). */
u16 pack_write_file(void *data, u16 *n)
{
    char *buf = data;
    register u16 got;
    i32 len;

    if (pack_write_size >= 0)
        got = (u16)min((i32)*n, pack_write_size);
    else
        got = *n;
    if (got != 0) {
        if (fileio_fwrite_f(buf, len = got, 1L, pack_write_file_handle) == 0)
            got = 0;
        else {
            if (pack_write_size > 0)
                pack_write_size -= len;
            pack_write_count += len;
        }
    }
    return got;
}

/* The file pack_write_file writes to. */
MSC_FILE *pack_write_file_handle;

/* pack_raw_copy -- the stored method: while pack_read_size has bytes left, moves up to
 * pack_buffer_size of them at a time through pack_buffer from the reader to the writer.
 * Returns 0, or 4 when the reader comes back short (the source ended early). */
i16 pack_raw_copy(void)
{
    u16 n;
    u16 got;
    i16 err;

    err = 0;
    while (pack_read_size > 0 && err == 0) {
        n = (u16)((i32)pack_buffer_size > pack_read_size ? pack_read_size : (i32)pack_buffer_size);
        got = pack_read_routine(pack_buffer, &n);
        if (got != n)
            err = 4;
        else
            pack_write_routine(pack_buffer, &n);
    }
    return err;
}

/* pack_a_packet -- runs the codec once with the installed reader, writer and work buffer, and
 * returns its error code (0 for success).
 *   method 0  compress: FAB when pack_strategy is 1, else implode, with pack_mode as the
 *             compression type and a 4 KB dictionary;
 *   method 1  expand: under FAB, kind picks the variant -- 0 reader to writer, 1 reader to
 *             memory (pack_write_memory_ptr), 2 memory to memory (pack_read_memory_ptr to
 *             pack_write_memory_ptr); otherwise explode, reader to writer;
 *   other     pack_raw_copy.
 * The routine pointers are not checked here: pack_data has already reported a missing one. */
i16 pack_a_packet(i16 method, i16 kind)
{
    u16 dsize;

    switch (method) {
    default:
        return pack_raw_copy();
    case 0:
        dsize = 0x1000;
        if (pack_strategy == 1)
            return pack_pFABcomp_routine(pack_read_routine, pack_write_routine, pack_buffer, &pack_mode, &dsize);
        return pack_implode_routine(pack_read_routine, pack_write_routine, pack_buffer, &pack_mode, &dsize);
    case 1:
        if (pack_strategy == 1) {
            switch (kind) {
            default:
                return pack_pFABexp0_routine(pack_read_routine, pack_write_routine, pack_buffer);
            case 1:
                return pack_pFABexp1_routine(pack_read_routine, pack_write_memory_ptr, pack_buffer);
            case 2:
                return pack_pFABexp2_routine(pack_read_memory_ptr, pack_write_memory_ptr, pack_buffer);
            }
        }
        return pack_explode_routine(pack_read_routine, pack_write_routine, pack_buffer);
    }
}

/* The packer's state. */
i16 pack_mode = 0;                        /* the compression type handed to the packers */
char pack_pfab_enabled = 0;               /* -1 once pack_enable_pfab has run */
i16 pack_strategy = 1;                    /* 1: the FAB codecs; otherwise implode/explode */
FFPACK pack_implode_routine = 0;          /* PKWARE implode (never installed) */
FFUNPACK pack_explode_routine = 0;        /* PKWARE explode (never installed) */
FFPACK pack_pFABcomp_routine = 0;         /* FAB compressor */
FFUNPACK pack_pFABexp0_routine = 0;       /* FAB expander, reader to writer */
FFUNPACK_TO pack_pFABexp1_routine = 0;    /* FAB expander, reader to memory */
FFUNPACK_MEM pack_pFABexp2_routine = 0;   /* FAB expander, memory to memory */
i32 pack_write_size;                     /* bytes the writer may still write; negative: no limit */
i32 pack_read_count;                     /* bytes the reader has read */
void *pack_buffer;                    /* the codec's work buffer */
i32 pack_read_size;                      /* bytes the reader may still read; negative: no limit */
u16 pack_buffer_size;                /* the work buffer's size */
FFIO pack_read_routine;                   /* the installed reader */
FFIO pack_write_routine;                  /* the installed writer */
i32 pack_write_count;                    /* bytes the writer has written */

/* pack_data -- packs, unpacks or copies `bytes` bytes from src to dst.
 *   method     0 compress, 1 expand, anything else copy unchanged;
 *   from_file  0: src is memory; otherwise src is a MSC_FILE * to read from;
 *   to_file    0: dst is memory; otherwise dst is a MSC_FILE * to write to;
 *   bytes      for a compress or a copy, the size of the input; for an expand, the size of the
 *              output.
 * It installs the reader and writer, sets their allowances, picks the work-buffer size for the
 * codec, gets the buffer ("$pack$") unless a special one is set, and runs the codec: once when
 * expanding to memory, otherwise packet by packet until the allowance runs out.
 *
 * Returns the bytes copied (a copy), the bytes read (a compress), or the bytes written (an
 * expand to a file). An expand to memory returns `bytes` itself, whatever the codec produced.
 * Returns 0 when the work buffer cannot be had. A codec that is not installed, or one that
 * fails, goes to error_report, which ends the program. */
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
        pack_read_file_handle = (MSC_FILE *)src;   /* Port: 1994 kept the offset */
    }
    if (to_file == 0) {
        pack_write_routine = pack_write_memory;
        pack_write_memory_ptr = dst;
    } else {
        pack_write_routine = pack_write_file;
        pack_write_file_handle = (MSC_FILE *)dst;   /* Port: 1994 kept the offset */
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
    pack_buffer = mem_get_name((i32)pack_buffer_size, "$pack$");
    if (pack_buffer == 0) {
        *count = 0;
        goto out;
    }
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
    if (pack_buffer != 0)
        mem_free(pack_buffer);
    return *count;
}

/* pack_read_memory -- the reader over memory: copies *n bytes from pack_read_memory_ptr into
 * data, advances the pointer, counts them and returns how many. A negative allowance (one whose
 * high word is FFFFh) means no limit; an allowance of 0 returns 0. Only an allowance under
 * 64 KB clamps the count, as in the original: a larger one is just decremented. *n is not
 * written back. */
u16 pack_read_memory(void *data, u16 *n)
{
    char *buf = data;
    u16 c = *n;
    if (((u32)pack_read_size >> 16) != 0xffff) {
        if (pack_read_size == 0)
            return 0;
        if (((u32)pack_read_size >> 16) == 0 && c > (u16)pack_read_size)
            c = (u16)pack_read_size;
        pack_read_size -= c;
    }
    pack_read_count += c;
    if (c) {
        memcpy(buf, pack_read_memory_ptr, c);
        pack_read_memory_ptr += c;
    }
    return c;
}

/* pack_write_memory -- the writer to memory, pack_read_memory the other way round: copies up
 * to *n bytes of data to pack_write_memory_ptr, under the same allowance rules.
 * Port: the original has no return statement and returns whatever its pointer normalisation
 * left behind (the new pointer's offset, 0-15, which depends on where DOS put the block). No
 * codec uses a writer's result. The port returns the count. */
u16 pack_write_memory(void *data, u16 *n)
{
    char *buf = data;
    u16 c = *n;
    if (((u32)pack_write_size >> 16) != 0xffff) {
        if (pack_write_size == 0)
            return 0;
        if (((u32)pack_write_size >> 16) == 0 && c > (u16)pack_write_size)
            c = (u16)pack_write_size;
        pack_write_size -= c;
    }
    pack_write_count += c;
    if (c) {
        memcpy(pack_write_memory_ptr, buf, c);
        pack_write_memory_ptr += c;
    }
    return c;
}

/* pack_enable_pfab -- installs the FAB codec (fab.c) and selects it: pack_strategy 1,
 * pack_pfab_enabled set, and the compressor and the three expanders in the routine pointers. */
void pack_enable_pfab(void)
{
    pack_strategy = 1;
    pack_pfab_enabled = -1;
    pack_pFABcomp_routine = pfabcomp;
    pack_pFABexp0_routine = pfabexp0;
    pack_pFABexp1_routine = pfabexp1;
    pack_pFABexp2_routine = pfabexp2;
}
