# Developer scripts

Cross-platform helper scripts for formatting, linting, and packaging `gflow`.
Scripts are split by OS; pick the folder for your platform:

| Task | Windows | Linux | macOS |
| --- | --- | --- | --- |
| Format non-compliant files | `windows/format-check.ps1` | `linux/format-check.sh` | `macos/format-check.sh` |
| clang-tidy compliance (`src/`) | `windows/clang-tidy.ps1` | `linux/clang-tidy.sh` | `macos/clang-tidy.sh` |
| Release build + package | `windows/deploy.ps1` | `linux/deploy.sh` | `macos/deploy.sh` |

All scripts resolve the repository root from their own location, so they can be
run from any working directory.

## Prerequisites

- **`clang-format` / `clang-tidy` on `PATH`** — required by the format and tidy
  scripts. If they are not found the script aborts with an error (no fallback to
  LLVM/Visual Studio install paths). Match the CI toolchain version (clang 22).
- **`VCPKG_ROOT`** — required by `clang-tidy` (when it has to configure a build)
  and by `deploy` (the `release` CMake preset references the vcpkg toolchain).
- **CMake ≥ 3.25 + Ninja** — required by `clang-tidy` and `deploy`.
- On Windows, run from a Developer PowerShell (MSVC environment) so Ninja can
  find the compiler.

## `format-check`

Scans `src/` for `*.cpp/*.h/*.hpp`, reports which files violate the repository
`.clang-format`, and re-formats exactly those files in place.

```powershell
# Windows
scripts\windows\format-check.ps1              # report + fix
scripts\windows\format-check.ps1 -CheckOnly   # report only, non-zero exit if dirty
```

```bash
# Linux / macOS
scripts/linux/format-check.sh                 # report + fix
scripts/linux/format-check.sh --check-only    # report only, non-zero exit if dirty
```

You can also pass an explicit list of files instead of scanning `src/`:
`format-check.sh path/to/a.cpp path/to/b.hpp`.

## `clang-tidy`

Runs `clang-tidy` over `src/` using the repository `.clang-tidy`, treating any
diagnostic as an error. If `build/compile_commands.json` is missing the script
configures a Ninja build into `build/` first (needs `VCPKG_ROOT`). It then runs
an incremental build so generated sources (e.g. the echo protobuf headers,
produced at build time) exist before linting — so a working compiler
environment is required (on Windows, run from a Developer shell).

```powershell
scripts\windows\clang-tidy.ps1
```

```bash
scripts/linux/clang-tidy.sh
```

## `deploy`

Builds **only** the `gflow` target in Release via the `release` CMake preset
(`build-release/`), then stages and archives a distributable bundle under
`dist/`:

- the `gflow` executable,
- on Windows, the runtime `*.dll`s vcpkg placed next to the executable (Linux and
  macOS default triplets are static, so no bundled libraries are needed),
- `LICENSE`, `THIRD-PARTY-NOTICES.md`, and `README.md`.

The archive is `dist/gflow-<version>-<os>-<arch>.zip` (Windows) or `.tar.gz`
(Linux/macOS); `<version>` is read from `vcpkg.json`.

```powershell
scripts\windows\deploy.ps1
```

```bash
scripts/linux/deploy.sh
```
