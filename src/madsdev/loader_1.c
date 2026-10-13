/* From mapedit-decomp src/madsdev/loader_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE loader_1: MAPEDIT.EXE segment 121a (loader_1.c), file 0x137a0-0x13b82, 3 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn loader_open         0x137a0
 * fn loader_set_priority 0x13adc
 * fn loader_close        0x13aee
 *
 * WHAT THE EDITOR'S loader_open DOES DIFFERENTLY: err is -1, open is -1. For a
 * read with a non-zero mode and loader_ems_search_disabled clear it first asks
 * himem_resident(name) whether the file is preloaded; when that answers >= 0 the FileIO is
 * filled from himem_directory_entry instead of the disk (kind 1 for EMS, 2 for XMS, the
 * handle, the page count 0x4000, the total and the entry count, and each entry's size
 * copied from the directory's size table into both size fields), and loader_found_in_ems or
 * loader_found_in_xms counts it. Otherwise it is VICEROY's open (loader_found_on_disk
 * counts it; no opens counter). The failed-open fclose also needs the himem answer to be 0.
 * The module's data (DS:0x44c2-0x44f5, all zero) is defined before loader_open, after its
 * two "MADSPACK 2.0" literals as shipped.
 *
 *  What got it there: mode is read
 * through a register COPY `m = mode` taken right after the strncpy (shipped `mov si,[bp-0x1e]`
 * after the call; the later `(char)mode` reads the home) -- a folded third use of the
 * parameter itself puts the si web at the entry and spills it; the loop is `if (count > 0)
 * do {...} while (++i < count)` with the count read through a volatile view (otherwise "l"
 * hoists it into a `loop` instruction, as it does for the disk path's sum). What is left:
 * shipped materialises p and q as `lea ax,[di+0x2a]; mov cx,[bp+0xc]` / `lea ax,[si+0x1a];
 * mov cx,[seg]`, keeps only their offsets in bx/di and reloads es from the segment slots
 * for every statement (`mov es,[bp-6]` even between two stores), with a stray
 * `mov ax,[bp-0xe]` before `inc cx`; ours builds the pointers in ax:dx and holds q's segment
 * in ds (restoring DGROUP after the loop). No effect: &fh->ent[0], a char-pointer walk
 * with casts, p[i]/q[i] indexing (for/do), a long temp for *q, volatile views on either
 * side, (void)&p/&q, volatile p/q, declaration order, folded dead defs of p/q after or in
 * the loop, pragmas e/el/eg, "w" off. The shipped frame has p at -8 and q at -0x10 (ours
 * p -0xc), so the allocator weighed them differently.
 *
 * MADSPACK FILES (overlay 28, para 0). DOS-only engine code. A FileIO wraps a stdio FILE
 * holding a "MADSPACK 2.0\x1a" archive: a 0xb0-byte header (12-byte magic, an entry count
 * at +0x28, and 16 ten-byte entries from +0x2a) followed by the data.
 *   - ff_open(how, mode, fh, name) (_fastcall, thunk 181f:2e9e) copies the name to
 *     g_ff_name (DS:0x244c) and opens the file through file_open.
 *       - For reading, it reads the 16-byte head, checks the magic, reads count entries,
 *         seeks past the header and sums the entry sizes into fh->total.
 *       - For writing, it stamps the magic, puts `mode` into entry 0's type and writes a
 *         blank header.
 *     It counts opens in g_ff_opens (DS:0x242e) and returns 0 on success. On failure it
 *     closes the FILE and returns 1.
 *   - loader_set_priority(flag, fh) sets entry 0's flag byte.
 *   - loader_close(fh) (thunk 181f:2eac) resets an in-memory file (kind 1 or 2). Otherwise it
 *     rewinds a file opened for writing, rewrites its header, and fcloses it. It returns 1
 *     when that header write failed.
 *
 * `#pragma optimize("elg")`: loader_set_priority and loader_close match under any pragma with "e".
 *
 *  The size (522) and the overall
 * shape are right with "elg" plus a `volatile` unused local (shipped keeps its dead `= -1`
 * store at bp-4). But register allocation differs: shipped keeps name's offset in si and
 * fh's in di, with `reading` in memory (bp-2, shared later with the loop's i), while ours
 * gives si to fh and di to `reading`. These had no effect: `register` on the parameters,
 * three local declaration orders, testing the field instead of the variable, pads 0-16,
 * and the pragmas "", e, el, eg, l, g, lg.
 * In ff_open fh (16 uses) takes si and reading di; name (2 uses) is not a candidate. A third
 * name use that folds (`|| (unsigned)name < 0`, `fh->open = (unsigned)name < 0`) makes name a
 * candidate -- in di, with fh still in si, and either keeps name live across file_open (so fh
 * spills a 2-word temp) or builds a far-pointer CSE temp of name. Nothing moved fh out of si:
 * `volatile int reading`, `fh->reading = reading; if (fh->reading)`, `char reading`, the
 * file_open test split or `!`-spelled, K&R declarations in either order, `register` on name/fh/
 * both (ignored under elg), `const`/`huge` name, fh read through a cast, a by-value wrapper, or
 * `*(FileIO far **)&fh` (the last three drop fh from the candidates: `les` per use), deleting
 * the loop/else/read branch/tail/kind+x8/x18 statements one at a time, and the pragmas aelg,
 * w-off, t-off, elgs/elgp/elgz, c-off. One observation for the next attempt: shipped frees si at file_open (name
 * dies at its `push si`) and uses si to carry es across that call and the first far_fread
 * (`mov si,es ... mov es,si`), where ours spills es:si to a 2-word temp; so shipped's name
 * must be a register candidate whose range ends there, i.e. a THIRD counted use of name
 * before file_open, not after it. The third use of name is a folded `if ((unsigned)name < 0) goto out;` placed between
 *      `fh->open = 0` and `reading = ...` (after strncpy: 184; after kind/x8 or inside the
 *      file_open call: name stays live across the call and es:si spills).
 *   2. The si/di PAIRING was the `volatile int unused`: with it name always takes di and fh
 *      si, whatever the fold; without it (name's three uses) name takes si and fh di as
 *      shipped. The dead `mov word [bp-4],-1` is kept instead by an aliased local: plain
 *      `int unused; unused = -1; (void)&unused;` or `int unused[1]; unused[0] = -1;` (both
 *      5 bytes). Aliased locals sort LAST among the memory slots, which is the 5 bytes left:
 *      shipped has reading(-2), unused(-4), err(-6); ours reading, err, unused. Volatile
 *      unused sorts above err but flips si/di again; a propagated read (`fh->x8 = unused`
 *      twice) keeps the store above err but makes unused a register candidate (119).
 *   3. `sub dx,dx` for the entry-size product: `(long)(unsigned)(fh->count * sizeof(FFEntry))`
 *      (`10u`, `(unsigned long)` and `unsigned count` all keep the cwd).
 *   4. The loop's segment roles (shipped `mov ds,[bp+0xc]` first, fh via ds:di, entries via
 *      es:si, and no nop after the exit jmp) come from a one-site volatile view on the
 *      reading store: `(*(volatile int far *)&fh->reading = reading) != 0`. A volatile
 *      FIELD does the same; two statements, an element view and a volatile local do not.
 *      No loop spelling (pointer walk, count-down, while, do, temp, huge view) moved it. The aliased `unused` is not what
 * shipped had: an aliased, array, struct or union local sorts LAST however many `&` refs it
 * gets, and aliasing err instead moves err ABOVE reading (67). Shipped's unused is a plain
 * local with a slot of its own. What the probes established:
 *   - Any counted read of a plain `unused` (a propagated `fh->x8 = unused`, a folded
 *     `(unsigned)unused < 0` in any condition, an optimizer-folded `if (unused != -1)`)
 *     gives one state, 328: name loses its register, reading takes si, unused sits at bp-2
 *     sharing with i. Doubling the name test, `if ((unsigned)name < 0 && (unsigned)name
 *     < 0) goto out;` (an `||` or a second statement drops the store), gives name si and fh
 *     di again: 17 bytes, every instruction right, frame 2 bytes short because unused,
 *     reading and i all share bp-2.
 *   - A memory local packs into an earlier slot when the lifetimes are disjoint, and the
 *     lifetime is computed AFTER constant propagation: a propagated read anywhere (x8, x18,
 *     count, total, return, loop body, array index, long argument), `if (unused != -1)`
 *     blocks at any site, dead later defs (`unused = 0` at out:, x18, before return), dead
 *     earlier defs of reading or i, declaration order, do/for wrappers and `>> 16` all leave
 *     the point lifetime (17). `& 0`, `* 0` and unreachable reads after return/goto are not
 *     references at all (store dropped, 428). A REAL read gives shipped's order at once
 *     (`unused = mode; fh->x18 = unused`: reading -2, unused -4, err -6, pairing intact).
 *   - The dead store survives only when a call separates the def from its propagated read
 *     (def after strchr: dropped); a def after strncpy sinks into the next push sequence.
 *   - `i = -1` at the top with `fh->x8 = i` (one variable, two webs): 6 bytes, the slot
 *     takes the union lifetime and sinks below err (bp-6). reading reused as the loop
 *     counter: 8 bytes (unused -2, err -4, reading -6). volatile unused: 181/523.
 *   Model that fits every state: slots are handed out by refs / lifetime length (code
 *   bytes, post-propagation), ties aside, with disjoint packing; shipped's unused is one
 *   store live for roughly 30-130 bytes from the top, i.e. past reading's def, with nothing
 *   reading it. No C spelling found that produces that lifetime without code. The lifetime comes from LATE-FOLDED BLOCKS: a statement whose
 * whole condition is an always-false unsigned test of a parameter, `if ((unsigned)mode < 0)
 * stmt;`, is deleted after slot lifetimes are computed, so a real read inside it extends the
 * variable's range with no code -- provided the read is not constant-propagated first, which a
 * second (also dead) def provides. Source: the doubled name test (R2) plus
 *     if ((unsigned)mode < 0) unused = 0;          before `reading = ...`
 *     if ((unsigned)mode < 0) fh->x8 = unused;     after it (before `fh->kind = 0`)
 * with plain `fh->x8 = -1` and no `(void)&unused`. unused now overlaps reading, so it gets its
 * own slot (bp-4), above err. Equivalent: the read as `fh->x18 = unused`, the read under
 * `(unsigned)how < 0`, or the def inside a `(unsigned)name < 0` block after the doubled test.
 * Needs the doubled name test (single test: 328). Not enough: the read alone (constant, 343,
 * code), the def alone (store dropped, 428), read-then-def in ONE dead block (17: the read sees
 * only the -1 def and is propagated), folded `|| (unsigned)unused < 0` terms inside existing
 * conditions (17: those fold early and extend nothing), the read placed after file_open
 * (`fh->x8 = unused` after or before `fh->x18 = 0`: 338/161, real code survives).
 */



typedef struct {                /* one directory entry, 10 bytes */
    char type;
    char flag;
    i32 size;
    i32 packed;
} FFEntry;

typedef struct {
    i16 msc_open;
    i16 reading;
    char kind;                  /* 1: in EMS, 2: in XMS */
    char pad5;
    MSC_FILE *fp;
    i16 ems_handle;             /* +8: the himem_resident answer */
    i16 xms_handle;             /* +0xa */
    i32 xms_pos;               /* +0xc */
    i16 ems_page;               /* +0x10 */
    i16 ems_offset;             /* +0x12 */
    i32 total;                 /* the entries' sizes summed */
    i16 next;
    char magic[12];             /* the 0xb0-byte header starts here */
    i16 x26;
    i16 count;
    FFEntry ent[16];
} FileIO;

extern MSC_FILE *env_open(char *name, char *mode);                     /* 0297:0104 */
extern i32 fileio_fread_f(void *p, i32 size, i32 n, MSC_FILE *fp);   /* 0bca:0000 */
extern i32 fileio_fwrite_f(void *p, i32 size, i32 n, MSC_FILE *fp);  /* 0bea:0008 */

/* Defined before loader_open: emitted after its literals, as shipped (DS:0x44c2-0x44f5). */
i32 loader_found_in_ems = 0;           /* DS:0x44c2 */
i32 loader_found_in_xms = 0;           /* DS:0x44c6 */
i32 loader_found_on_disk = 0;          /* DS:0x44ca */
i32 loader_timing_ems = 0;             /* DS:0x44ce */
i32 loader_timing_xms = 0;             /* DS:0x44d2 */
i32 loader_timing_disk = 0;            /* DS:0x44d6 */
i32 loader_size_ems = 0;               /* DS:0x44da */
i32 loader_size_xms = 0;               /* DS:0x44de */
i32 loader_size_disk = 0;              /* DS:0x44e2 */
i16 loader_ems_search_disabled = 0;     /* DS:0x44e6 */
char loader_last[14] = "";              /* DS:0x44e8 */
/* loader_open -- 121a:0000, 828 bytes */
i16 loader_open(char *how, i16 mode, FileIO *fh, char *name)
{
    i16 reading;
    i16 err;
    i16 i;
    fpos_t pos;

    err = -1;
    msc__fstrncpy(loader_last, name, 13);
    fh->msc_open = 0;
    reading = msc_strchr(msc_strlwr(how), 'r') != 0;
    fh->kind = 0;
    fh->ems_handle = -1;
    if ((fh->fp = env_open(name, how)) == 0)
        goto out;
    fh->reading = reading;
    fh->next = 0;
    if (reading) {
        msc_fgetpos(fh->fp, &pos);
        if (fileio_fread_f(fh->magic, 16L, 1L, fh->fp) == 0)
            goto out;
        if (msc__fstrncmp(fh->magic, "MADSPACK 2.0\x1a", 12) != 0)
            goto out;
        if (fileio_fread_f(fh->ent, (i32)(u16)(fh->count * sizeof(FFEntry)), 1L, fh->fp) == 0)
            goto out;
        pos += 0xb0;
        msc_fsetpos(fh->fp, &pos);
        fh->total = 0;
        for (i = 0; i < fh->count; i++)
            fh->total += fh->ent[i].size;
    } else {
        fh->count = 0;
        fh->ent[0].type = (char)mode;
        msc__fstrcpy(fh->magic, "MADSPACK 2.0\x1a");
        if (fileio_fwrite_f(fh->magic, 0xb0L, 1L, fh->fp) == 0)
            goto out;
        fh->total = 0;
    }
    loader_found_on_disk++;
    fh->msc_open = -1;
    err = 0;
out:
    /* (1994 closed the file on an error only for a resident file, which is never opened
       here: a file that fails its header check stays open, as it did) */
    return err;
}
/* loader_set_priority -- 121a:033c, 18 bytes */
void loader_set_priority(i16 flag, FileIO *fh)
{
    fh->ent[0].flag = (char)flag;
}
/* loader_close -- 121a:034e, 148 bytes */
i16 loader_close(FileIO *fh)
{
    i16 err;

    err = 0;
    if (fh->msc_open) {
        if (fh->reading == 0) {
            msc_rewind(fh->fp);
            err = fileio_fwrite_f(fh->magic, 0xb0L, 1L, fh->fp) == 0;
        }
        msc_fclose(fh->fp);
    }
    fh->msc_open = 0;
    return err;
}

