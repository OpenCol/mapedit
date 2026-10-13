# Where src/ came from

`src/` began as [mapedit-decomp](https://github.com/OpenCol/mapedit-decomp) at commit
`f55aff8`: the editor's source reconstructed from the shipped `MAPEDIT.EXE`, function by function
compiled back to the same bytes with Microsoft C 6.00A. A converter (`tools/import.py`, in this
repository's history up to 0.1.0) turned it into C for modern compilers, and patches marked
`PORT:` fixed what a 64-bit machine needs differently. Since then `src/` is this project's code:
it is changed here, and mapedit-decomp stays the byte-exact reference it is checked against
(`tools/m4.py`, the original under DOSBox-X; `tests/oracle`, the original's machine code).

## What the conversion did

- `int`, `unsigned`, `long` became `i16`, `u16`, `i32` (16-bit arithmetic kept where 1994 had it);
  `far`, `near`, `huge`, `_fastcall`, `pascal`, `cdecl`, `_based`, `_segment` went; `char` stays
  signed; every module packs its records on 2 bytes (`#pragma pack(2)`, MSC's `/Zp2`).
- The C run-time functions the editor calls became `msc_*` (`port/msc_runtime.c`, MSC 6's
  behaviour on SDL3): `_dos_findfirst`, `_dos_findnext`, `_dos_read`, `_dos_write`, `_fmemcmp`, `_fmemcpy`, `_fmemmove`, `_fmemset`, `_fstrcat`, `_fstrchr`, `_fstrcpy`, `_fstricmp`, `_fstrlen`, `_fstrncmp`, `_fstrncpy`, `_fstrrchr`, `_fstrupr`, `atoi`, `bda`, `fclose`, `fgetpos`, `fgets`, `filelength`, `fopen`, `fread`, `fseek`, `fsetpos`, `fwrite`, `itoa`, `ltoa`, `open`, `printf`, `remove`, `rewind`, `setbuf`, `sprintf`, `strcat`, `strchr`, `strcmp`, `strcpy`, `strlen`, `strlwr`, `strncat`, `strncmp`, `strncpy`, `strnicmp`, `strupr`, `vga`.
- Far pointers into the BIOS data area (`0040:xxxx`) and VGA memory (`A000:xxxx`) became
  `msc_bda` and `msc_vga`.
- The records several modules share (`DosPort`, `Arena`, `HotSpot`) are defined once,
  in `port/port_records.h`.
- `main` became `editor_main`; `port/main.c` is the program's entry.
- The byte-matching scaffolding (stand-in declarations that moved symbols, `#pragma optimize`
  settings) was removed after the fork, and so were EMS, XMS and the high-memory directory
  (`ems_*`, `xms_*`, `himem_*`, `pack_d`'s EMS writer, `buffer_s`'s EMS copies), which a
  machine without them never used: the port always took the conventional-memory and disk
  paths those modules fell back to.

## Functions written anew in port/lib

These were inline assembly in 1994 (or the assembly modules), and have C of their own in
`port/lib/`; `tests/oracle` checks the ones that compute something against the original's code.

| Module | Function |
| --- | --- |
| `editor/env_1` | `env_exist` |
| `editor/tile` | `tile_draw` |
| `editor/tile` | `tile_draw_mask` |
| `editor/tile` | `tile_draw_scale` |
| `editor/tile` | `tile_draw_scale_mask` |
| `editor/strings` | `strings` |
| `editor/map_2` | `show_map_gradually` |
| `madsdev/hspot_7` | `hspot_which` |
| `madsdev/keys_1` | `keys_any` |
| `madsdev/keys_1` | `keys_get` |
| `madsdev/buffer_4` | `buffer_rect_copy` |
| `madsdev/buffer_5` | `buffer_rect_fill` |
| `madsdev/buffer_6` | `buffer_rect_copy_2` |
| `madsdev/buffer_9` | `buffer_hline` |
| `madsdev/buffer_a` | `buffer_vline` |
| `madsdev/buffer_i` | `buffer_pointer` |
| `madsdev/mem_2` | `mem_alloc_paras` |
| `madsdev/mem_2` | `mem_check_chain` |
| `madsdev/mem_2` | `mem_set_block_name` |
| `madsdev/mem_2` | `mem_get_name` |
| `madsdev/mem_2` | `mem_get_block_name` |
| `madsdev/mem_2` | `mem_free` |
| `madsdev/mem_2` | `mem_adjust` |
| `madsdev/mem_4` | `mem_conv_largest` |
| `madsdev/mem_4` | `mem_conv_get_avail` |
| `madsdev/font_2` | `font_write` |
| `madsdev/font_4` | `font_string_width` |
| `madsdev/mcga_7` | `mcga_setpal` |
| `madsdev/mcga_c` | `mcga_time_palette_swap` |
| `madsdev/mcga_c` | `mcga_compute_retrace_parameters` |
| `madsdev/sprite_1` | `sprite_draw` |
| `madsdev/sprite_2` | `sprite_draw_scaled` |
| `madsdev/sprite_l` | `sprite_draw_black` |
| `madsdev/sprite_m` | `sprite_draw_scaled_black` |
| `madsdev/error_1` | `error_shutdown_report` |
| `madsdev/cycle_1` | `cycle_colors` |
| `madsdev/heap_1` | `heap_shrink` |
| `madsdev/mem_1` | `mem_normalize` |
| `madsdev/timer_2` | `TIMER_SET_RATE` |
| `madsdev/mcga_8` | `mcga_getpal` |
| `madsdev/mcga_b` | `mcga_retrace` |
| `madsdev/mcga_b` | `mcga_setpal_range` |
| `madsdev/sprite_h` | `sprite_color_translate` |
| `madsdev/pal_1` | `pal_get_hash` |
| `madsdev/echo_1` | `echo` |
| `madsdev/mcga_d` | `MCGA_SHAKE` |
| `madsdev/sort_3` | `sort_insertion_8` |
| `madsdev/pack_6` | `far_to_near` |
| `madsdev/pack_6` | `io_call_hook` |
| `madsdev/magic_e` | `magic_hash_color` |
| `madsdev/mem_3` | `mem_check_overflow` |
| `madsdev/pack_1` | `pack_read_memory` |
| `madsdev/pack_2` | `pack_write_memory` |
