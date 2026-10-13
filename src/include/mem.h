/* mem.h -- the functions, data and records of buffer.c, heap_1.c, mem.c, mem_2.c, mem_4.c. */
#ifndef MAPEDIT_MEM_H
#define MAPEDIT_MEM_H

#include "msc.h"

i32 heap_shrink(Arena *a);
i16 mem_adjust(i32 size, void *p);
u16 mem_alloc_paras(u16 paras);
void mem_check_chain(void);
void *mem_check_overflow(void *p);
i32 mem_conv_get_avail(void);
u16 mem_conv_largest(void);
i16 mem_free(void *p);
void mem_get_block_name(void *block, char *dest);
void *mem_get_name(i32 size, char *tag);
void mem_set_block_name(void *block, char *name);
i16 heap_create(i16 kind, Arena *a, i32 size, char *tag);
void heap_declare(i16 kind, Arena *a, char *base, i32 size);
void heap_destroy(Arena *a);
void *heap_get(i32 n, Arena *a);
void mem_call_hook(u16 off, u16 seg);
void *mem_get(i32 size);
void mem_restore_free(void);
void mem_save_free(void);
i32 mem_get_avail(void);
void mem_set_video_mode(void);

extern char mem_initialized;
extern i32 mem_last_alloc_avail;
extern char mem_last_alloc_failed;
extern i16 mem_last_alloc_loader;
extern i32 mem_last_alloc_size;
extern i32 mem_last_alloc_umb;
extern i16 mem_manager_active;
extern void (*mem_manager_update)(void);
extern i32 mem_max_free;
extern char mem_max_free_set;
extern i32 mem_min_free;
extern i32 mem_save_free_area;
extern i32 umb_max_free;
extern char umb_max_free_set;
extern i32 umb_min_free;
extern i32 umb_save_free_area;
char *mem_normalize(char *p);

#endif
