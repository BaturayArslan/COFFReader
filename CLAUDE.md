# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Early-stage COFF (Windows object/executable format) reader in C++, built with MSVC (`cl`). There are no tests, linter, or README yet.

## Build

- Run `shell.bat` first. It calls an external script (`C:\Users\batur\AppData\Roaming\handmadeHero\scripts\shell.bat`) that puts `cl` on PATH; the path is hardcoded.
- `build-sample.bat` creates `samples/build/` and compiles the sample programs: `cl -Zi -I ..\code ..\code\entry.cpp ..\code\addition.cpp` (emits `.obj`, `.exe`, `.ilk`, `.pdb`).
- No build script exists for `main.cpp` yet. Once it is real, something like `cl -Zi main.cpp` would work.

## Layout

- `main.cpp` — the reader itself. Currently a stub (`void main()` that returns a value, so it won't compile cleanly).
- `samples/code/` — small C++ programs (`entry.cpp`, `addition.cpp`, `addition.h`) compiled to produce real COFF `.obj`/`.exe` files to use as reader inputs.
- `samples/build/` — generated compiler output. It is tracked in git, so rebuilding shows up as modified/deleted/untracked files.

## Notes

- `.gitignore` only covers Emacs backup files (`*~`, `#*#`, `.#*`), which is why `main.cpp~` etc. are present but ignored.
