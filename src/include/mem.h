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
 * and no upper memory. The mem_last_alloc_* variables describe the last allocation for the
 * error report; the port sets only mem_last_alloc_failed and mem_last_alloc_loader.
 */
#ifndef MAPEDIT_MEM_H
#define MAPEDIT_MEM_H

#include "msc.h"

i32 mem_conv_get_avail(void);
i16 mem_free(void *p);
void *mem_get_name(i32 size, char *tag);
void mem_set_block_name(void *block, char *name);
i16 heap_create(i16 kind, Arena *a, i32 size, char *tag);
void heap_declare(i16 kind, Arena *a, char *base, i32 size);
void *heap_get(i32 n, Arena *a);
void *mem_get(i32 size);
i32 mem_get_avail(void);

extern i32 mem_last_alloc_avail;
extern char mem_last_alloc_failed;      /* the last allocation returned null */
extern i16 mem_last_alloc_loader;       /* set by the loaders; not read */
extern i32 mem_last_alloc_size;         /* shown by the error report; never set */
extern void (*mem_manager_update)(void); /* run after each allocation and release */
char *mem_normalize(char *p);

#endif
