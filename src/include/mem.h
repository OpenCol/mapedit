/* mem.h -- memory: named blocks, the arenas carved from them, the free-memory figures.
 *
 * Defined in sys/mem.c. Every allocation is a block from mem_get_name, zeroed and tagged
 * with up to eight characters, and freed with mem_free. An arena (Arena,
 * port/port_records.h) is a block handed out front to back by heap_get and released all at
 * once: heap_create allocates its block, heap_declare lays one over memory the caller owns.
 * Running out of an arena is a fatal error. The menus, the popups and the string store keep
 * their records in arenas.
 *
 * The original managed DOS memory itself (the memory-control-block chain, upper memory).
 * The port allocates from the C heap and reports a fixed 576 KB of conventional memory free
 * and no upper memory; the original's bookkeeping variables stay, most of them unused.
 */
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
extern char mem_last_alloc_failed;      /* the last allocation returned null */
extern i16 mem_last_alloc_loader;       /* set by the loaders; not read */
extern i32 mem_last_alloc_size;         /* shown by the error report; never set */
extern i32 mem_last_alloc_umb;
extern i16 mem_manager_active;          /* -1 while that hook runs */
extern void (*mem_manager_update)(void); /* run after each allocation and release */
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
