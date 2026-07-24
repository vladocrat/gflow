<#
.SYNOPSIS
    Build gflow in Release and package it into a distributable archive.

.DESCRIPTION
    Configures and builds only the gflow target via the 'release' CMake preset
    (Ninja, build-release\), stages the executable together with its runtime DLLs
    (placed next to the exe by vcpkg's applocal deploy) and the license/notice
    files, then produces dist\gflow-<version>-windows-<arch>.zip.

    Requires VCPKG_ROOT (referenced by the release preset toolchain).

.EXAMPLE
    scripts\windows\deploy.ps1
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

# Run a native tool without letting its stderr writes turn into terminating
# errors under $ErrorActionPreference = 'Stop' (Windows PowerShell 5.1 quirk).
# Returns the process exit code; output is streamed to the host.
function Invoke-Tool {
    param(
        [Parameter(Mandatory)][string]$FilePath,
        [string[]]$ToolArgs = @()
    )
    $prev = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $FilePath @ToolArgs 2>&1 | ForEach-Object { Write-Host $_ }
        return $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $prev
    }
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path

if (-not $env:VCPKG_ROOT) {
    Write-Error "VCPKG_ROOT is not set. It is required by the 'release' preset (vcpkg toolchain)."
    exit 1
}

# --- Version / platform identifiers -----------------------------------------
$version = '0.0.0'
try {
    $vcpkg = Get-Content (Join-Path $repoRoot 'vcpkg.json') -Raw | ConvertFrom-Json
    if ($vcpkg.'version-semver') { $version = $vcpkg.'version-semver' }
    elseif ($vcpkg.version) { $version = $vcpkg.version }
} catch {
    Write-Warning "Could not read version from vcpkg.json; using $version."
}

$arch = switch ($env:PROCESSOR_ARCHITECTURE) {
    'AMD64' { 'x64' }
    'ARM64' { 'arm64' }
    'x86'   { 'x86' }
    default { $env:PROCESSOR_ARCHITECTURE.ToLower() }
}
$pkgName = "gflow-$version-windows-$arch"

# --- Configure + build (gflow only) -----------------------------------------
Write-Host "Configuring (preset: release)..."
$code = Invoke-Tool 'cmake' @('--preset', 'release')
if ($code -ne 0) { Write-Error "CMake configure failed (exit $code)."; exit $code }

Write-Host "Building gflow (Release)..."
$code = Invoke-Tool 'cmake' @('--build', '--preset', 'release', '--target', 'gflow')
if ($code -ne 0) { Write-Error "Build failed (exit $code)."; exit $code }

$buildDir = Join-Path $repoRoot 'build-release'
$exe = Get-ChildItem -Path $buildDir -Recurse -File -Filter 'gflow.exe' | Select-Object -First 1
if (-not $exe) {
    Write-Error "Could not locate gflow.exe under $buildDir after build."
    exit 1
}
$exeDir = $exe.Directory.FullName

# --- Stage ------------------------------------------------------------------
$distRoot = Join-Path $repoRoot 'dist'
$stageDir = Join-Path $distRoot $pkgName
if (Test-Path $stageDir) { Remove-Item -Recurse -Force $stageDir }
New-Item -ItemType Directory -Path $stageDir -Force | Out-Null

Copy-Item $exe.FullName -Destination $stageDir
# Runtime DLLs vcpkg copied next to the executable.
Get-ChildItem -Path $exeDir -File -Filter '*.dll' | Copy-Item -Destination $stageDir

foreach ($extra in 'LICENSE', 'THIRD-PARTY-NOTICES.md', 'README.md') {
    $path = Join-Path $repoRoot $extra
    if (Test-Path $path) { Copy-Item $path -Destination $stageDir }
    else { Write-Warning "$extra not found; skipping." }
}

# --- Package ----------------------------------------------------------------
$archive = Join-Path $distRoot "$pkgName.zip"
if (Test-Path $archive) { Remove-Item -Force $archive }
Compress-Archive -Path (Join-Path $stageDir '*') -DestinationPath $archive

Write-Host ""
Write-Host "Staged:  $stageDir"
Write-Host "Archive: $archive"
