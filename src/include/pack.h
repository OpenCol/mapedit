/* pack.h -- packing and unpacking: the stream plumbing, and the FAB codec it drives.
 *
 * Defined in sys/pack.c and sys/fab.c (the codec, pfab*). pack_data moves a block of bytes
 * from a source to a destination, each memory or an open file, through a codec: method 0
 * compresses, method 1 expands, anything else copies the bytes unchanged. The codecs never
 * see the source or the destination; they pull bytes through a reader and push them through
 * a writer (FFIO), and pack_data installs the matching pair in pack_read_routine and
 * pack_write_routine. Each reader or writer moves no more than its allowance
 * (pack_read_size, pack_write_size; negative: no limit) and counts what it moved
 * (pack_read_count, pack_write_count).
 *
 * Two codec families plug in through the routine pointers: PKWARE's implode/explode and
 * MicroProse's FAB; pack_strategy 1 selects FAB. The editor installs only FAB
 * (pack_enable_pfab, at startup), and its one caller is the MADSPACK loader (loader.h).
 */
#ifndef MAPEDIT_PACK_H
#define MAPEDIT_PACK_H

#include "msc.h"

#pragma pack(push, 2)
/* A reader or a writer: moves up to *n bytes into or out of buf and returns how many. */
typedef u16 (*FFIO)(void *buf, u16 *n);
/* A compressor: reads through rd, writes through wr, with a work buffer; *type is the
 * compression type and *dsize the dictionary size. */
typedef i16 (*FFPACK)(FFIO rd, FFIO wr, void *work, i16 *type,
                                 u16 *dsize);
/* The expanders: reader to writer, memory to memory, reader to memory. Each returns 0, or an
 * error code. */
typedef i16 (*FFUNPACK)(FFIO rd, FFIO wr, void *work);
typedef i16 (*FFUNPACK_MEM)(void *src, void *dst, void *work);
typedef i16 (*FFUNPACK_TO)(FFIO rd, void *dst, void *work);
#pragma pack(pop)

i16 pfabcomp(FFIO rd, FFIO wr, void *work, i16 *type, u16 *dsize);
i16 pfabexp0(FFIO rd, FFIO wr, void *work);
i16 pfabexp1(FFIO rd, void *dst, void *work);
i16 pfabexp2(void *src, void *dst, void *work);
u16 pack_read_file(void *data, u16 *n);
u16 pack_write_file(void *data, u16 *n);
i16 pack_a_packet(i16 method, i16 kind);
i16 pack_raw_copy(void);
i32 pack_data(i16 method, i16 from_file, i16 to_file, i32 bytes, void *src, void *dst);
void pack_enable_pfab(void);

extern char *pack_read_memory_ptr;
extern char *pack_write_memory_ptr;
extern MSC_FILE *pack_read_file_handle;
extern MSC_FILE *pack_write_file_handle;
extern void *pack_buffer;
extern u16 pack_buffer_size;
extern FFUNPACK pack_explode_routine;
extern FFPACK pack_implode_routine;
extern i16 pack_mode;                    /* the compression type handed to the packers */
extern FFPACK pack_pFABcomp_routine;
extern FFUNPACK pack_pFABexp0_routine;
extern FFUNPACK_TO pack_pFABexp1_routine;
extern FFUNPACK_MEM pack_pFABexp2_routine;
extern char pack_pfab_enabled;           /* -1 once pack_enable_pfab has run */
extern i32 pack_read_count;
extern FFIO pack_read_routine;
extern i32 pack_read_size;
extern i16 pack_strategy;                /* 1: the FAB codec; else implode/explode */
extern i32 pack_write_count;
extern FFIO pack_write_routine;
extern i32 pack_write_size;
/* a work buffer and a hook for pack_data; never set */
extern void (*pack_special_buffer)(void);
extern void (*pack_special_function)(void);
u16 pack_read_memory(void *data, u16 *n);
u16 pack_write_memory(void *data, u16 *n);

#endif
