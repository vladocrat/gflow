#!/usr/bin/env bash
#
# Run clang-tidy compliance over src/.
#
# Runs clang-tidy against every src/*.cpp/*.h/*.hpp using the repo .clang-tidy,
# treating any diagnostic as an error (mirrors CI).
#
# Requires a compile database. If build/compile_commands.json is missing, the
# script configures a Ninja build into build/ with the vcpkg toolchain (all
# targets ON so the database covers every src/ file). This needs VCPKG_ROOT.
#
# The script then runs an incremental build so generated sources (e.g. the echo
# protobuf headers, produced at build time) exist before linting; a working
# compiler toolchain is therefore required.
#
# clang-tidy must be on PATH; the script aborts otherwise.
#
# Usage:
#   clang-tidy.sh

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"
build_dir="$repo_root/build"

if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "error: clang-tidy not found on PATH. Install LLVM/clang and ensure clang-tidy is on PATH." >&2
    exit 1
fi

if [ ! -f "$build_dir/compile_commands.json" ]; then
    if [ -z "${VCPKG_ROOT:-}" ]; then
        echo "error: VCPKG_ROOT is not set. It is required to configure the build (no compile_commands.json found)." >&2
        exit 1
    fi
    echo "compile_commands.json not found; configuring build/ (Ninja)..."
    cmake -S "$repo_root" -B "$build_dir" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Debug
fi

# Generated sources (e.g. the echo protobuf headers) are produced at build time,
# so build first (incremental; a no-op once up to date) to make sure they exist
# before clang-tidy parses the translation units that include them.
echo "Building (incremental) so generated sources exist..."
cmake --build "$build_dir"

files=()
while IFS= read -r -d '' f; do
    files+=("$f")
done < <(find "$repo_root/src/gflow" "$repo_root/src/gflow-sdk" -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0)

if [ "${#files[@]}" -eq 0 ]; then
    echo "No source files to check."
    exit 0
fi

clang-tidy --version

echo "Running clang-tidy over ${#files[@]} file(s) in src/ ..."
clang-tidy -p "$build_dir" --warnings-as-errors=* "${files[@]}"

echo "clang-tidy: no violations."
