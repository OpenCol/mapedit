/* msc.h -- what Microsoft C 6.00A gave the 1994 source, for a modern compiler.
 *
 * Every imported module includes this first (tools/import.py puts it there, and replaces each
 * #include <...> with it). It provides:
 *   - the integer types the import rewrote the source's to, with their 1994 widths
 *     (int 16 bits, long 32); char is compiled signed (-fsigned-char), as MSC's was;
 *   - (each imported module sets structure packing on 2-byte boundaries, MSC's default /Zp2,
 *     so a record read from a file has the layout it had);
 *   - the run-time library, renamed msc_* by the import so the source's own declarations
 *     never meet the modern library's; port/msc_runtime.c implements them with MSC 6's
 *     behaviour (16-bit ints, DOS file names, its own rand and printf);
 *   - the macros of MSC's headers the source uses (ctype, FILE constants). The source defines
 *     its own max and min.
 */
#ifndef MAPEDIT_MSC_H
#define MAPEDIT_MSC_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;

#ifndef NULL
#define NULL ((void *)0)
#endif

/* ---- stdio ---- */
/* MSC's FILE record: the library reads and moves its buffer pointer and count itself, and
   calls DOS on its handle (fileio_fread_f), so the port keeps the record as it was */
typedef struct MSC_FILE {
    char *_ptr;         /* next byte in the buffer */
    i16 _cnt;           /* bytes left in it */
    char *_base;        /* the buffer */
    char _flag;         /* _IOREAD 1, _IOWRT 2, _IONBF 4, _IOMYBUF 8, _IOEOF 0x10, _IOERR 0x20, _IORW 0x80 */
    char _file;         /* the DOS handle (port: an index into msc_runtime.c's handle table) */
} MSC_FILE;
/* MSC's stdio.h macros over the record */
#define feof(f)   ((f)->_flag & 0x10)
#define ferror(f) ((f)->_flag & 0x20)
typedef i32 msc_fpos_t;
#ifndef MSC_RUNTIME_IMPL     /* port/msc_runtime.c uses the host's headers beside this one */
#define fpos_t msc_fpos_t
#define EOF (-1)
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/* ---- dos.h ---- */
struct find_t {
    char reserved[21];
    char attrib;
    u16 wr_time;
    u16 wr_date;
    i32 size;
    char name[13];
};
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20

/* ---- fcntl.h / io.h: DOS's values ---- */
#ifndef MSC_RUNTIME_IMPL
#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR   0x0002
#define O_APPEND 0x0008
#define O_CREAT  0x0100
#define O_TRUNC  0x0200
#define O_EXCL   0x0400
#define O_TEXT   0x4000
#define O_BINARY 0x8000
#endif

/* ---- ctype.h: MSC's own table, which the source also indexes itself ((_ctype + 1)[c]).
   _ctype[0] is EOF's; characters 128-255 are in no class. */
extern u8 _ctype[257];
/* (MSVC's and MinGW's headers define these too, with the same values) */
#undef _UPPER
#undef _LOWER
#undef _DIGIT
#undef _SPACE
#undef _PUNCT
#undef _CONTROL
#undef _BLANK
#undef _HEX
#define _UPPER   0x01
#define _LOWER   0x02
#define _DIGIT   0x04
#define _SPACE   0x08
#define _PUNCT   0x10
#define _CONTROL 0x20
#define _BLANK   0x40
#define _HEX     0x80
#ifndef MSC_RUNTIME_IMPL
#define MSC_CT(c) ((_ctype + 1)[(u8)(c)])
#define isalpha(c)  (MSC_CT(c) & (_UPPER | _LOWER))
#define isupper(c)  (MSC_CT(c) & _UPPER)
#define islower(c)  (MSC_CT(c) & _LOWER)
#define isdigit(c)  (MSC_CT(c) & _DIGIT)
#define isxdigit(c) (MSC_CT(c) & _HEX)
#define isspace(c)  (MSC_CT(c) & _SPACE)
#define ispunct(c)  (MSC_CT(c) & _PUNCT)
#define isalnum(c)  (MSC_CT(c) & (_UPPER | _LOWER | _DIGIT))
#define isprint(c)  (MSC_CT(c) & (_BLANK | _PUNCT | _UPPER | _LOWER | _DIGIT))
#define isgraph(c)  (MSC_CT(c) & (_PUNCT | _UPPER | _LOWER | _DIGIT))
#define iscntrl(c)  (MSC_CT(c) & _CONTROL)
#undef _toupper
#undef _tolower
#define _toupper(c) ((c) - 'a' + 'A')
#define _tolower(c) ((c) - 'A' + 'a')
#define toupper(c)  (islower(c) ? _toupper(c) : (c))
#define tolower(c)  (isupper(c) ? _tolower(c) : (c))
#endif

/* ---- the run-time library the source calls, MSC 6's signatures in the port's types ---- */
char *msc_strcat(char *d, const char *s);
char *msc_strncat(char *d, const char *s, u16 n);
i16   msc__fmemcmp(const void *a, const void *b, u16 n);
char *msc_strcpy(char *d, const char *s);
char *msc_strncpy(char *d, const char *s, u16 n);
u16   msc_strlen(const char *s);
i16   msc_strcmp(const char *a, const char *b);
i16   msc_strncmp(const char *a, const char *b, u16 n);
i16   msc_strnicmp(const char *a, const char *b, u16 n);
char *msc_strchr(const char *s, i16 c);
char *msc_strupr(char *s);
char *msc_strlwr(char *s);
char *msc__fstrcat(char *d, const char *s);
char *msc__fstrcpy(char *d, const char *s);
char *msc__fstrncpy(char *d, const char *s, u16 n);
u16   msc__fstrlen(const char *s);
i16   msc__fstrncmp(const char *a, const char *b, u16 n);
char *msc__fstrchr(const char *s, i16 c);
char *msc__fstrrchr(const char *s, i16 c);
char *msc__fstrupr(char *s);
void *msc__fmemcpy(void *d, const void *s, u16 n);
void *msc__fmemset(void *d, i16 c, u16 n);
void *msc__fmemmove(void *d, const void *s, u16 n);
i16   msc__fstricmp(const char *a, const char *b);
i16   msc_fseek(MSC_FILE *f, i32 offset, i16 whence);
i16   msc_atoi(const char *s);
char *msc_itoa(i16 v, char *s, i16 radix);
char *msc_ltoa(i32 v, char *s, i16 radix);
MSC_FILE *msc_fopen(const char *name, const char *mode);
i16   msc_fclose(MSC_FILE *f);
u16   msc_fread(void *p, u16 size, u16 n, MSC_FILE *f);
u16   msc_fwrite(const void *p, u16 size, u16 n, MSC_FILE *f);
char *msc_fgets(char *s, i16 n, MSC_FILE *f);
void  msc_rewind(MSC_FILE *f);
void  msc_setbuf(MSC_FILE *f, char *buf);
i16   msc_fgetpos(MSC_FILE *f, msc_fpos_t *pos);
i16   msc_fsetpos(MSC_FILE *f, const msc_fpos_t *pos);
i16   msc_remove(const char *name);
i16   msc_printf(const char *fmt, ...);
i16   msc_sprintf(char *buf, const char *fmt, ...);
i16   msc_open(const char *name, i16 flags, ...);
i16   msc_close(i16 handle);
i32   msc_filelength(i16 handle);
u16   msc__dos_findfirst(char *spec, u16 attrib, struct find_t *f);
u16   msc__dos_findnext(struct find_t *f);
u16   msc__dos_read(i16 handle, void *buf, u16 count, u16 *done);
u16   msc__dos_write(i16 handle, void *buf, u16 count, u16 *done);

/* MSC's <stdlib.h> min and max, which the source uses (some modules define their own) */
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

/* the BIOS data area (0040:0000-00FF): the shift flags at 17h, the tick count at 6Ch; kept
   current by port/lib/bios.c */
extern u8 msc_bda[256];

/* VGA memory, segment A000 (64 KB): the screen is its first 64000 bytes; the editor keeps its
   palette at FC00. The host's (platform/host.h). */
extern u8 host_vga[0x10000];
#define msc_vga host_vga

/* the records modules share, one definition each */
#include "port_records.h"

/* the editor's main, called by the port's */
void editor_main(i16 argc, char **argv);

#endif
