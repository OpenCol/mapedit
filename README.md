# mapedit-port

The **Colonization Map Editor** (`MAPEDIT.EXE`, MicroProse 1994) running natively on Windows,
macOS and Linux. It is built from [mapedit-decomp](https://github.com/OpenCol/mapedit-decomp), the editor's source
recovered from the shipped program, compiled with modern compilers, and it runs on
[SDL3](https://libsdl.org) instead of DOS.

**Status: 0.1.0, faithful.** The editor runs natively on macOS (Apple silicon and Intel)
and Windows (64-bit), and Linux builds from source. Given the same keys, it writes `.MP` files
byte-identical to the original's running under DOSBox-X (`tools/m4.py`, six sessions: open
and save, paint, undo, fill, Find Continents, a new map). Improvements come next, each
behind an option. [docs/plan.md](docs/plan.md) has the milestones.

## Download

[Releases](https://github.com/OpenCol/mapedit/releases): `mapedit-<version>-macos.zip`
(an application, "Colonization Map Editor.app") and `mapedit-<version>-windows-x64.zip`
(`mapedit.exe` and `SDL3.dll`). Each has a README.txt on starting it. You need your own
copy of *Colonization* for DOS (GOG or Steam): the editor asks for its folder the first
time and remembers it.

On macOS the app is not notarized: the first time, right-click it and choose Open.

## The code

Four layers -- `src/sys`, `src/core`, `src/ui`, `src/editor` -- each using only those below
it, with one header per subsystem in `src/include`; [docs/architecture.md](docs/architecture.md)
has the map and the checks that keep it so.

## What you need

- your own copy of *Colonization* for DOS (GOG or Steam). The editor reads the game's art,
  fonts and palette from it, and none of that is in this repository;
- CMake 3.16 or later, a C99 compiler, and SDL3.

```sh
cmake -B build && cmake --build build
ctest --test-dir build                      # self-tests; the oracle with MAPEDIT_EXE set
```

Run it on a copy of your game directory -- the editor writes its maps there, as it did:

```sh
build/mapedit --game /path/to/COLONIZE
build/mapedit --game /path/to/COLONIZE --headless --script session.txt
```

Without `--game` (or `$MAPEDIT_GAME`) it looks in the current directory, then its own (or,
for the macOS app, the folder the app is in), then the folder chosen last time, and else
asks with a folder dialog (`--choose-game` asks again). On macOS, `cmake --build build
--target mapedit_app` makes "Colonization Map Editor.app"; `tools/release.py` builds the
release zips (a static, universal SDL3 for the app; the Windows build cross-compiled).

### Windows

With MSYS2 (UCRT64: `pacman -S mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,sdl3,python}`):

```sh
cmake -B build -G Ninja && cmake --build build --target mapedit
```

`build/mapedit.exe` and `build/SDL3.dll` are the program. From macOS or Linux, cross-build
with MinGW-w64 and SDL3's MinGW development package (`cmake/mingw-w64-x86_64.cmake` says
how); `tools/m4.py --wine` checks the result under Wine. MSVC builds too (CI).

A script is one command a line: `wait MS`, `key NAME`, `type TEXT`, `move X Y`, `click X Y`,
`shot FILE.bmp`, `exit` (platform/host_sdl.c has them all).

The oracle tests run the player's own `MAPEDIT.EXE` in an emulator beside the port: set
`MAPEDIT_EXE` to it and install Python's `unicorn` package.

## The contract

The port must behave exactly like the 1994 program before it does anything new. Given the
same input, it shows the same screens and writes byte-identical `.MP` files. Gate M4
([docs/plan.md](docs/plan.md)) measures that against the original running under DOSBox-X.
Improvements come after it, each behind an option.

## License

See [NOTICE.md](NOTICE.md). The platform layer and tools are MIT licensed. The imported
source (`src/`) is a reconstruction of MicroProse's program and is not ours to license.
