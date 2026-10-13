# mapedit-port

The **Colonization Map Editor** (`MAPEDIT.EXE`, MicroProse 1994) running natively on Windows,
macOS and Linux. It is built from [mapedit-decomp](../mapedit-decomp), the editor's source
recovered from the shipped program, compiled with modern compilers, and it runs on
[SDL3](https://libsdl.org) instead of DOS.

**Status: M2 done (it links), M3 under way.** The editor starts, loads the game's palette,
fonts and art, and shows its first screen; scripted sessions (`--script`, below) drive it
headless and take screenshots. Every function that was inline assembly has C in
`port/lib/`, each checked against the original machine code where it computes something
(`tests/oracle`, 21 tests, thousands of random cases). [docs/plan.md](docs/plan.md) has the
milestones and the gate for each one.

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
