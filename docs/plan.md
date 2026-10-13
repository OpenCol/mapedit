# Plan: the Colonization map editor on modern systems

`MAPEDIT.EXE`, the map editor MicroProse shipped with *Colonization* for DOS in 1994, rebuilt
from its recovered source ([mapedit-decomp](../../mapedit-decomp)) as a native program for
Windows, macOS and Linux, on SDL3.

Two stages:
- **Faithful.** The port behaves exactly like the DOS program. With the same input it shows the
  same screens and writes the same `.MP` files.
- **Improved.** Changes on top of that baseline, each one optional and each one listed.

## Starting point

mapedit-decomp holds the editor as C that Microsoft C 6.00A compiles to the shipped bytes, one
file per function, under the original names (the EXE kept its linker's symbol table). That is
the specification: the port changes how the C is compiled and what it runs on, never what it
does.

The program has three layers, and the port treats each one differently:

| Layer | Modules | In the port |
| --- | --- | --- |
| The editor | `mapedit`, `map*`, `popup`, `menu`, `text`, `write`, `tile`, `terrain`, `me_mini`, ... | imported, mechanically converted |
| MicroProse's library, C | `buffer_*`, `sprite_*`, `font_*`, `pal_*`, `pack_*`, `loader_*`, `fileio_*`, `hspot_*`, ... | imported, converted; inline `_asm` rewritten in C |
| The machine | the 13 asm modules (`mouse_1`, `keys_4`, `timer_3`, `video_1/2`, `sound_1/2`, `stack_1`, `fileio_c`, PKWARE's `pfab*`), the memory managers (`mem_*`, `ems_*`, `xms_*`, `himem_*`), and every DOS/BIOS call | replaced by `platform/`, under the same names |

## The conversion (tools/import.py)

The import takes mapedit-decomp at a pinned commit (`import/SOURCE`), never a working tree, and
writes `src/`. It is run once per source update. Edits made by hand are marked `PORT:` and
survive re-imports as a three-way merge.
- **16-bit semantics, explicitly.** `int` becomes `int16_t`, `unsigned` becomes `uint16_t`,
  `long` becomes `int32_t`, and `char` stays signed (`-fsigned-char`). Arithmetic that wrapped
  at 16 bits in 1994 must still wrap; modern C promotes to 32 bits and silently stops wrapping.
  This is the port's main risk. Every fix is a cast, logged.
- **Pointers.** `far`, `near`, `huge`, `_fastcall`, `pascal` and `cdecl` go. Segment arithmetic
  (`mem_normalize`, `FP_SEG`/`FP_OFF`, huge-pointer walks) becomes flat-pointer arithmetic.
  Records that are read from files keep their 1994 layout (`#pragma pack(1)`, 16-bit fields).
- **Inline assembly.** About 40 `_asm` blocks (pixel copies, palette ports, DOS calls) are
  rewritten as C by hand, each one checked against the original (gate 2).

## The platform layer (platform/)

The C implementations of what the asm modules and DOS did, keeping the original entry points:
- **video**: a 320×200 8-bit framebuffer standing in for VGA mode 13h, with the 6-bit DAC
  palette, shown in a window scaled by an integer factor (`video_init`, `video_update`,
  `VIDEO_ADDRESS`, the `mcga_*` palette functions);
- **mouse**: the INT 33h driver and MicroProse's software cursor (`mouse_*`: position, buttons,
  the cursor drawn into the work buffer and wiped, view ports, freeze/thaw);
- **keyboard**: the INT 9 handler and the BIOS queue (`KEYS_*`, `keys_*`): SDL key events
  become the scan code / ASCII pairs the editor reads;
- **timer**: the INT 8 handler (`timer_install`, `timer_read`, `timer_read_600`), from a
  monotonic clock, or from a virtual clock that counts reads (for the gates);
- **files**: DOS file names in the game directory, found whatever their case;
- **memory**: `mem_get` and friends on `malloc`; EMS, XMS and upper memory report "absent";
- **decompression**: PKWARE's explode for the game's packed art (zlib's `blast.c`);
- **sound**: none (the editor makes none).

## Milestones and gates

| | Milestone | Gate (a command that exits 0) |
| --- | --- | --- |
| M0 | this repository, the SDL3 host, CI | `ctest`: the host self-test |
| M1 | every imported module compiles | warning-free on clang, gcc and MSVC |
| M2 | it links | no unresolved symbol |
| M3 | it starts, shows the editor and edits a map | a scripted session runs to the end |
| M4 | **faithful** | the same input script, run in the original (DOSBox-X) and in the port, writes byte-identical `.MP` files, and the screens match at each checkpoint |
| M5 | improvements | each one behind an option, M4 still green with all of them off |

Gate M4 is the contract. An input script is a list of key presses and mouse events, with the
time each one happens. The original runs under DOSBox-X, with the editor's timer reads
counted rather than clocked, so both sides see the same time.

## Improvements (M5), candidates

- window scaling and fullscreen; aspect-correct 4:3 presentation;
- the mouse wheel for zoom (`Z`/`X`), dragging the view;
- more undo levels (the original has one);
- native open/save dialogs beside the original's file list;
- maps larger than the original's 12,000-square limit, flagged as unreadable by the 1994 game;
- an export of the map as a PNG.
