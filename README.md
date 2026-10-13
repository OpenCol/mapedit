# mapedit-port

The **Colonization Map Editor** (`MAPEDIT.EXE`, MicroProse 1994) running natively on Windows,
macOS and Linux. It is built from [mapedit-decomp](../mapedit-decomp), the editor's source
recovered from the shipped program, compiled with modern compilers, and it runs on
[SDL3](https://libsdl.org) instead of DOS.

**Status: M0.** The repository and the platform host exist; the editor is not imported yet.
[docs/plan.md](docs/plan.md) has the milestones and the gate for each one.

## What you need

- your own copy of *Colonization* for DOS (GOG or Steam). The editor reads the game's art,
  fonts and palette from it, and none of that is in this repository;
- CMake 3.16 or later, a C99 compiler, and SDL3.

```sh
cmake -B build && cmake --build build
ctest --test-dir build                      # the host self-test
```

## The contract

The port must behave exactly like the 1994 program before it does anything new. Given the
same input, it shows the same screens and writes byte-identical `.MP` files. Gate M4
([docs/plan.md](docs/plan.md)) measures that against the original running under DOSBox-X.
Improvements come after it, each behind an option.

## License

See [NOTICE.md](NOTICE.md). The platform layer and tools are MIT licensed. The imported
source (`src/`) is a reconstruction of MicroProse's program and is not ours to license.
