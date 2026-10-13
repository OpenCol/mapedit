/* From mapedit-decomp src/editor/text (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE text: MAPEDIT.EXE segment 0842 (text.obj), file 0x9a20-0x9c5e, 10 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn text_close       0x09a20
 * fn text_open        0x09a3a
 * fn text_get         0x09b26
 * fn text_item_get    0x09b7e
 * fn text_item_number 0x09bb8
 * fn text_next_line   0x09bc4
 * fn text_string      0x09bd6
 * fn text_item_string 0x09be8
 * fn text_item_binary 0x09bfa
 * fn text_search      0x09c28
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl24_6f8e0).
 *
 *  The module's four `jmp far` stubs
 * at 0x6fb1e (text_get, text_open, text_close, text_item_get) are the `push cs; call near`
 * targets.
 *
 * THIS MODULE WAS COMPILED WITH MORE OPTIMISATION THAN THE GAME (NEW, not in the docs/msc6-codegen.md).
 * Nothing matches under the plain /Ot /Ow; `#pragma optimize("elg", on)` at the top -- global
 * register allocation, loop optimisation and global common subexpressions, probably a /Ox
 * command line -- matches all ten with the Windows C nearly verbatim and no `register`:
 *   - /Oe puts locals AND parameters in si/di by lifetime: text_open's file name, its `found`
 *     flag and its result all live in si; text_get's p and text_item_get's d are registers.
 *   - /Oe caches a GLOBAL in a register across a loop: text_item_get walks text_pointer in si
 *     and stores it back once, from the Windows source unchanged (a local copy loads in the
 *     wrong order).
 *   - /Ol strength-reduces text_search's `for (i = 0; i <= n; i++) text_get();` into a
 *     count-down of n + 1 in si; i keeps a frame slot it never uses.
 *   - With /Oe a lone `unsigned char` local sits at bp-1 (text_item_binary), not bp-2.
 *   - Without /Og, `*d = *s; s++; d++` becomes lodsb + `inc di; mov [di-1],al`.
 *
 * NAMES. Windows' text_close is empty.
 *
 * WHAT THE 1994 BUILD DOES DIFFERENTLY:
 *   - text_open(file, section): the file name (".TXT" added unless it has an extension) is
 *     opened "rt", and the section is found by reading lines until one equals "@SECTION"
 *     (upper-cased). A NULL file name keeps searching the file already open; then the first
 *     end of file is retried once with an empty line (there is no rewind, so the retry reads
 *     end of file again and fails). On success the cursor is left at the END of the header
 *     line. Any failure closes the file. Windows only loads the resource; text_search seeks.
 *   - text_get reads the next line with fgets (80 chars) and returns NULL at end of file,
 *     closing the file; Windows walks the in-memory copy and always returns the buffer.
 *   - text_search(file, section, n) = text_open + n+1 text_get + text_close, as in Windows.
 *
 * PLUMBING: fileio_add_ext and fileio_fix_lf_input/fileio_purge_all_spaces are `pascal` (far
 * pointers, callee pops); env_open (181f:0e86) is _fastcall (bx the mode, the far name on the
 * stack); stoi (1a1f:0b3a) is _fastcall (bx). fclose/fgets/strcmp/strchr are the MSC runtime
 * (0d1d:03f4, 09ca, 0816, 0c56). text_handle (DS:0x2014) is DEFINED here: it opens the
 * module's _DATA, followed by its literals "TXT" (0x2016) and "rt" (0x201a).
 */

#include "msc.h"
#include "msc.h"

MSC_FILE *text_handle = NULL;               /* DS:0x2014 */
extern char text_buffer[];               /* DS:0x833c */
extern char *text_pointer;             /* DS:0xa608 */
extern char text_item[];                  /* DS:0xa5b8 */
extern void fileio_add_ext(char *path, char *ext);
extern MSC_FILE *env_open(char *name, char *mode);
extern void fileio_fix_lf_input(char *s);
extern void fileio_purge_all_spaces(char *s);
extern i16 stoi(char *s);
extern i16 strings_store(char *s);
/* text_close -- 0842:0000, 26 bytes */
void text_close(void)
{
    if (text_handle != NULL) {
        msc_fclose(text_handle);
        text_handle = NULL;
    }
}
/* text_open -- 0842:001a, 236 bytes */
i16 text_open(char *name, char *key)
{
    i16 flag;
    i16 wrap;
    i16 found;
    char tag[0x50];
    char path[0x50];

    flag = 1;
    wrap = 0;
    tag[0] = '@';
    tag[1] = 0;
    msc_strcat(tag, key);
    msc_strupr(tag);
    if (name != NULL) {
        text_close();
        msc_strcpy(path, name);
        fileio_add_ext(path, "TXT");
        text_handle = env_open(path, "rt");
        if (text_handle == NULL)
            goto out;
    } else
        wrap = 1;
    if (key != NULL) {
        found = 0;
        do {
            if (msc_fgets(text_buffer, 0x50, text_handle) == NULL) {
                if (wrap == 0)
                    goto out;
                wrap = 0;
                text_buffer[0] = 0;
            }
            fileio_fix_lf_input(text_buffer);
            fileio_purge_all_spaces(text_buffer);
            if (msc_strcmp(text_buffer, tag) == 0)
                found = 1;
        } while (found == 0);
        text_pointer = text_buffer + msc_strlen(text_buffer);
    }
    flag = 0;
out:
    if (flag)
        text_close();
    return flag;
}
/* text_get -- 0842:0106, 88 bytes */
char *text_get(void)
{
    char *p;

    p = NULL;
    if (msc_fgets(text_buffer, 0x50, text_handle) != NULL) {
        fileio_fix_lf_input(text_buffer);
        fileio_purge_all_spaces(text_buffer);
        do {
            p = msc_strchr(text_buffer, '_');
            if (p != NULL)
                *p = ' ';
        } while (p != NULL);
        p = text_pointer = text_buffer;
    }
    if (p == NULL)
        text_close();
    return p;
}
/* text_item_get -- 0842:015e, 58 bytes */
char *text_item_get(void)
{
    char *d = text_item;

    while (*text_pointer != 0 && *text_pointer != ',') {
        *d = *text_pointer;
        text_pointer++;
        d++;
    }
    if (*text_pointer != 0)
        text_pointer++;
    *d = 0;
    fileio_purge_all_spaces(text_item);
    return text_item;
}
/* text_item_number -- 0842:0198, 12 bytes */
i16 text_item_number(void)
{
    return stoi(text_item_get());
}
/* text_next_line -- 0842:01a4, 18 bytes */
void text_next_line(void)
{
    text_pointer = text_buffer + msc_strlen(text_buffer);
}
/* text_string -- 0842:01b6, 18 bytes */
i16 text_string(void)
{
    text_get();
    return strings_store(text_buffer);
}
/* text_item_string -- 0842:01c8, 18 bytes */
i16 text_item_string(void)
{
    text_item_get();
    return strings_store(text_item);
}
/* text_item_binary -- 0842:01da, 46 bytes */
u8 text_item_binary(void)
{
    u8 v;
    char *p;

    v = 0;
    text_item_get();
    p = text_item;
    while (*p == '0' || *p == '1') {
        v <<= 1;
        if (*p == '1')
            v++;
        p++;
    }
    return v;
}
/* text_search -- 0842:0208, 54 bytes */
i16 text_search(char *name, char *key, i16 n)
{
    i16 err;
    i16 i;

    err = 1;
    if (text_open(name, key) == 0) {
        for (i = 0; i <= n; i++)
            text_get();
        err = 0;
    }
    text_close();
    return err;
}

/* The module's uninitialised data (communals, BSS). */
char text_item[80];                       /* DS:0x51cc */
char *text_pointer;                       /* DS:0x5ad6 */
char text_buffer[80];                     /* DS:0x5ec6 */

