<#
.SYNOPSIS
    Verify which src/ files violate clang-format and re-format the offenders in place.

.DESCRIPTION
    Scans src/ for *.cpp/*.h/*.hpp, checks each against the repo .clang-format
    (--style=file), prints the non-compliant files, then re-formats exactly those
    (unless -CheckOnly is given).

    clang-format must be on PATH; the script aborts otherwise. The .clang-format in
    this repo requires a recent clang-format (see CI: clang-format ~22).

.PARAMETER CheckOnly
    Report offenders without modifying any file. Exits non-zero if any file is
    non-compliant (use in CI / pre-commit).

.PARAMETER Files
    Optional explicit list of files to check instead of scanning src/.

.EXAMPLE
    scripts\windows\format-check.ps1

.EXAMPLE
    scripts\windows\format-check.ps1 -CheckOnly
#>
[CmdletBinding()]
param(
    [switch]$CheckOnly,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Files
)

$ErrorActionPreference = 'Stop'

# Run a native tool without letting its stderr writes turn into terminating
# errors under $ErrorActionPreference = 'Stop' (Windows PowerShell 5.1 quirk).
# Returns the process exit code; captured output is written to the host unless -Quiet.
function Invoke-Tool {
    param(
        [Parameter(Mandatory)][string]$FilePath,
        [string[]]$ToolArgs = @(),
        [switch]$Quiet
    )
    $prev = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $output = & $FilePath @ToolArgs 2>&1
        if (-not $Quiet) { $output | ForEach-Object { Write-Host $_ } }
        return $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $prev
    }
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path

$clangFormatCmd = Get-Command clang-format -ErrorAction SilentlyContinue
if (-not $clangFormatCmd) {
    Write-Error "clang-format not found on PATH. Install LLVM/clang and ensure clang-format is on PATH."
    exit 1
}
$clangFormat = $clangFormatCmd.Source

if ($Files -and $Files.Count -gt 0) {
    $sources = $Files | ForEach-Object { (Resolve-Path $_).Path }
} else {
    $srcDir = Join-Path $repoRoot 'src'
    $sources = Get-ChildItem -Path $srcDir -Recurse -File -Include *.cpp, *.h, *.hpp |
        ForEach-Object { $_.FullName }
}

if (-not $sources -or @($sources).Count -eq 0) {
    Write-Host "No source files to check."
    exit 0
}

Invoke-Tool $clangFormat @('--version') | Out-Null

$offenders = @()
foreach ($file in $sources) {
    $code = Invoke-Tool $clangFormat @('--dry-run', '-Werror', '--style=file', $file) -Quiet
    if ($code -ne 0) {
        $offenders += $file
    }
}

if ($offenders.Count -eq 0) {
    Write-Host "All files are correctly formatted."
    exit 0
}

Write-Host ""
Write-Host "Non-compliant files ($($offenders.Count)):"
foreach ($file in $offenders) {
    $rel = if ($file.StartsWith($repoRoot)) { $file.Substring($repoRoot.Length).TrimStart('\', '/') } else { $file }
    Write-Host "  $rel"
}

if ($CheckOnly) {
    Write-Host ""
    Write-Host "Run without -CheckOnly to format these files."
    exit 1
}

Write-Host ""
Write-Host "Formatting offenders..."
$code = Invoke-Tool $clangFormat (@('-i', '--style=file') + $offenders)
if ($code -ne 0) {
    Write-Error "clang-format failed while rewriting files (exit $code)."
    exit $code
}
Write-Host "Formatted $($offenders.Count) file(s)."
