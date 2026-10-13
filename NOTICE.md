# NOTICE

## What this repository is derived from

This is a port of the **Colonization Map Editor** (`MAPEDIT.EXE`), part of **Sid
Meier's Colonization for DOS** (1994), **© 1994 MicroProse Software, Inc.** The rights in that
program are held by its present owner, not by this project.
The work is preservation and interoperability research, carried out on a copy of the game the
author owns.

## Two kinds of material, two different answers

**The platform layer, the tools and the research are this project's own work** and are MIT
licensed (see [LICENSE](LICENSE)). That covers `platform/`, `tools/`, `tests/`, `docs/`,
the files under `.github/`, `CMakeLists.txt`, and the Markdown documents at the top level.

**The rest is a reconstruction of someone else's program**:
- `src/`, imported from mapedit-decomp and converted for modern compilers;
- `import/`, the record of that import.

Each function compiles to bytes identical to part of the shipped binary. That is the point of
it, and it is also why this project cannot grant a license over it. No license is offered for
these files, and nothing in [LICENSE](LICENSE) should be read as one. If you want to reuse
them, ask the rights holder.

## What is not distributed here

The game is not in this repository, and neither are its data, art, music, text or resources.
None of the editor's machine code is copied in either: the C in `src/` is a reconstruction
written by hand. Like any source, it states the initial values of the program's tables and the
strings its code uses. The symbol table in `link/symbols.tsv` holds names and addresses only.
The hand-written assembly modules of the editor, among them PKWARE's Data Compression Library,
which MicroProse licensed, are not reconstructed here; `link/asm.json` holds only their names
and addresses. The build compares its result with **your own copy** of `MAPEDIT.EXE`. Nothing
here produces a program without one.

Microsoft C 6.00A is not distributed here either. It is third-party software, still under
copyright, and must come from your own media. docs/toolchain.md lists the files the build uses.

## Trademarks

*Sid Meier's Colonization*, *MicroProse* and *Sid Meier* are trademarks of their respective
owners. This project is not affiliated with, endorsed by, or connected to MicroProse Software,
Inc., Take-Two Interactive, Firaxis Games, or Sid Meier.

## Takedown

If you hold rights in *Sid Meier's Colonization* and object to any material here, open an
issue or contact the author and it will be removed.
