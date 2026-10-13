/* pack.h -- the functions, data and records of fab.c, misc.c, pack_1.c, pack_2.c, pack_3.c, pack_4.c, pack_5.c, pack_6.c, pack_9.c. */
#ifndef MAPEDIT_PACK_H
#define MAPEDIT_PACK_H

#include "msc.h"

#pragma pack(push, 2)
typedef u16 (*FFIO)(void *buf, u16 *n);
typedef i16 (*FFPACK)(FFIO rd, FFIO wr, void *work, i16 *type,
                                 u16 *dsize);
typedef i16 (*FFUNPACK)(FFIO rd, FFIO wr, void *work);
typedef i16 (*FFUNPACK_MEM)(void *src, void *dst, void *work);
typedef i16 (*FFUNPACK_TO)(FFIO rd, void *dst, void *work);
#pragma pack(pop)

i16 pfabcomp(FFIO rd, FFIO wr, void *work, i16 *type, u16 *dsize);
i16 pfabexp0(FFIO rd, FFIO wr, void *work);
i16 pfabexp1(FFIO rd, void *dst, void *work);
i16 pfabexp2(void *src, void *dst, void *work);
u16 far_to_near(void *p);
void io_call_hook(void);
u16 pack_read_memory(void *data, u16 *n);
u16 pack_write_memory(void *data, u16 *n);
u16 pack_read_file(void *data, u16 *n);
u16 pack_write_file(void *data, u16 *n);
i16 pack_a_packet(i16 method, i16 kind);
i16 pack_raw_copy(void);
i32 pack_data(i16 method, i16 from_file, i16 to_file, i32 bytes, void *src, void *dst);
void pack_set_special_buffer(void (*a)(void), void (*b)(void));
void pack_enable_pfab(void);

extern char *pack_read_memory_ptr;
extern char *pack_write_memory_ptr;
extern MSC_FILE *pack_read_file_handle;
extern MSC_FILE *pack_write_file_handle;
extern void *pack_buffer;
extern u16 pack_buffer_size;
extern FFUNPACK pack_explode_routine;
extern FFPACK pack_implode_routine;
extern i16 pack_mode;
extern FFPACK pack_pFABcomp_routine;
extern FFUNPACK pack_pFABexp0_routine;
extern FFUNPACK_TO pack_pFABexp1_routine;
extern FFUNPACK_MEM pack_pFABexp2_routine;
extern char pack_pfab_enabled;
extern i32 pack_read_count;
extern FFIO pack_read_routine;
extern i32 pack_read_size;
extern i16 pack_strategy;
extern i32 pack_write_count;
extern FFIO pack_write_routine;
extern i32 pack_write_size;
extern char pack_zip_enabled;
extern void (*pack_special_buffer)(void);
extern void (*pack_special_function)(void);

#endif
