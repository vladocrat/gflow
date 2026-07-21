#!/usr/bin/env bash
#
# Verify which src/ files violate clang-format and re-format the offenders in place.
#
# Scans src/ for *.cpp/*.h/*.hpp, checks each against the repo .clang-format
# (--style=file), prints the non-compliant files, then re-formats exactly those
# (unless --check-only is given).
#
# clang-format must be on PATH; the script aborts otherwise.
#
# Usage:
#   format-check.sh [--check-only] [file ...]
#
#   --check-only   Report offenders without modifying any file; exit non-zero if
#                  any file is non-compliant (use in CI / pre-commit).
#   file ...       Optional explicit files to check instead of scanning src/.

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"

check_only=0
files=()
for arg in "$@"; do
    case "$arg" in
        --check-only) check_only=1 ;;
        *) files+=("$arg") ;;
    esac
done

if ! command -v clang-format >/dev/null 2>&1; then
    echo "error: clang-format not found on PATH. Install LLVM/clang and ensure clang-format is on PATH." >&2
    exit 1
fi

if [ "${#files[@]}" -eq 0 ]; then
    while IFS= read -r -d '' f; do
        files+=("$f")
    done < <(find "$repo_root/src/gflow" "$repo_root/src/gflow-sdk" -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0)
fi

if [ "${#files[@]}" -eq 0 ]; then
    echo "No source files to check."
    exit 0
fi

clang-format --version

offenders=()
for f in "${files[@]}"; do
    if ! clang-format --dry-run -Werror --style=file "$f" >/dev/null 2>&1; then
        offenders+=("$f")
    fi
done

if [ "${#offenders[@]}" -eq 0 ]; then
    echo "All files are correctly formatted."
    exit 0
fi

echo ""
echo "Non-compliant files (${#offenders[@]}):"
for f in "${offenders[@]}"; do
    echo "  ${f#"$repo_root/"}"
done

if [ "$check_only" -eq 1 ]; then
    echo ""
    echo "Run without --check-only to format these files."
    exit 1
fi

echo ""
echo "Formatting offenders..."
clang-format -i --style=file "${offenders[@]}"
echo "Formatted ${#offenders[@]} file(s)."
