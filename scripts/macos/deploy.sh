#!/usr/bin/env bash
#
# Build gflow in Release and package it into a distributable archive.
#
# Configures and builds only the gflow target via the 'release' CMake preset
# (Ninja, build-release/), stages the executable and the license/notice files,
# then produces dist/gflow-<version>-macos-<arch>.tar.gz.
#
# On macOS the default vcpkg triplet is static, so the gflow binary needs no
# bundled shared libraries beyond the system ones.
#
# Requires VCPKG_ROOT (referenced by the release preset toolchain).
#
# Usage:
#   deploy.sh

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"

if [ -z "${VCPKG_ROOT:-}" ]; then
    echo "error: VCPKG_ROOT is not set. It is required by the 'release' preset (vcpkg toolchain)." >&2
    exit 1
fi

# --- Version / platform identifiers -----------------------------------------
version="$(grep -oE '"version(-semver)?"[[:space:]]*:[[:space:]]*"[^"]+"' "$repo_root/vcpkg.json" \
    | head -n1 | sed -E 's/.*"([^"]+)"$/\1/')"
version="${version:-0.0.0}"

case "$(uname -m)" in
    x86_64|amd64) arch="x86_64" ;;
    aarch64|arm64) arch="arm64" ;;
    *) arch="$(uname -m)" ;;
esac
pkg_name="gflow-${version}-macos-${arch}"

# --- Configure + build (gflow only) -----------------------------------------
echo "Configuring (preset: release)..."
cmake --preset release

echo "Building gflow (Release)..."
cmake --build --preset release --target gflow

build_dir="$repo_root/build-release"
exe="$(find "$build_dir" -type f -name gflow -perm -u+x | head -n1)"
if [ -z "$exe" ]; then
    echo "error: could not locate the gflow binary under $build_dir after build." >&2
    exit 1
fi

# --- Stage ------------------------------------------------------------------
dist_root="$repo_root/dist"
stage_dir="$dist_root/$pkg_name"
rm -rf "$stage_dir"
mkdir -p "$stage_dir"

cp "$exe" "$stage_dir/"
for extra in LICENSE THIRD-PARTY-NOTICES.md README.md; do
    if [ -f "$repo_root/$extra" ]; then
        cp "$repo_root/$extra" "$stage_dir/"
    else
        echo "warning: $extra not found; skipping." >&2
    fi
done

# --- Package ----------------------------------------------------------------
archive="$dist_root/$pkg_name.tar.gz"
rm -f "$archive"
tar -czf "$archive" -C "$dist_root" "$pkg_name"

echo ""
echo "Staged:  $stage_dir"
echo "Archive: $archive"
