<#
.SYNOPSIS
    Run clang-tidy compliance over src/.

.DESCRIPTION
    Runs clang-tidy against every src/*.cpp/*.h/*.hpp using the repo .clang-tidy,
    treating any diagnostic as an error (mirrors CI).

    Requires a compile database. If build\compile_commands.json is missing, the
    script configures a Ninja build into build\ with the vcpkg toolchain (all
    targets ON so the database covers every src/ file). This needs VCPKG_ROOT.

    The script then runs an incremental build so generated sources (e.g. the echo
    protobuf headers, produced at build time) exist before linting. This means a
    working compiler environment is required (on Windows, run from a Developer
    shell).

    clang-tidy must be on PATH; the script aborts otherwise.

.EXAMPLE
    scripts\windows\clang-tidy.ps1
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
$buildDir = Join-Path $repoRoot 'build'

$clangTidyCmd = Get-Command clang-tidy -ErrorAction SilentlyContinue
if (-not $clangTidyCmd) {
    Write-Error "clang-tidy not found on PATH. Install LLVM/clang and ensure clang-tidy is on PATH."
    exit 1
}
$clangTidy = $clangTidyCmd.Source

$compileDb = Join-Path $buildDir 'compile_commands.json'
if (-not (Test-Path $compileDb)) {
    if (-not $env:VCPKG_ROOT) {
        Write-Error "VCPKG_ROOT is not set. It is required to configure the build (no compile_commands.json found)."
        exit 1
    }
    Write-Host "compile_commands.json not found; configuring build\ (Ninja)..."
    $toolchain = Join-Path $env:VCPKG_ROOT 'scripts\buildsystems\vcpkg.cmake'
    $code = Invoke-Tool 'cmake' @(
        '-S', $repoRoot, '-B', $buildDir, '-G', 'Ninja',
        "-DCMAKE_TOOLCHAIN_FILE=$toolchain",
        '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
        '-DCMAKE_BUILD_TYPE=Debug'
    )
    if ($code -ne 0) {
        Write-Error "CMake configure failed (exit $code)."
        exit $code
    }
}

# Generated sources (e.g. the echo protobuf headers) are produced at build time,
# so build first (incremental; a no-op once up to date) to make sure they exist
# before clang-tidy parses the translation units that include them.
Write-Host "Building (incremental) so generated sources exist..."
$code = Invoke-Tool 'cmake' @('--build', $buildDir)
if ($code -ne 0) {
    Write-Error "Build failed (exit $code)."
    exit $code
}

$srcDirs = @(
    (Join-Path $repoRoot 'src\gflow'),
    (Join-Path $repoRoot 'src\gflow-sdk')
)
$sources = Get-ChildItem -Path $srcDirs -Recurse -File -Include *.cpp, *.h, *.hpp |
    ForEach-Object { $_.FullName }

if (-not $sources -or @($sources).Count -eq 0) {
    Write-Host "No source files to check."
    exit 0
}

Invoke-Tool $clangTidy @('--version') | Out-Null

Write-Host "Running clang-tidy over $(@($sources).Count) file(s) in src\gflow and src\gflow-sdk ..."
$code = Invoke-Tool $clangTidy (@('-p', $buildDir, '--warnings-as-errors=*') + $sources)

if ($code -ne 0) {
    Write-Error "clang-tidy reported violations (exit $code)."
    exit $code
}

Write-Host "clang-tidy: no violations."
