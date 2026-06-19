# pre-pr.ps1 — Pre-PR checklist for esp32-cam-rtsp
#
# Run this script before every pull request to catch issues locally.
# The script DOES NOT commit. All commits are created by the repository owner.
#
# Usage:
#   .\scripts\pre-pr.ps1               # full check
#   .\scripts\pre-pr.ps1 -SkipBuild    # skip PlatformIO build (fast mode)

param(
    [switch]$SkipBuild
)

$RootDir = Split-Path -Parent $PSScriptRoot
Set-Location $RootDir

$PASS   = "[OK]  "
$FAIL   = "[FAIL]"
$WARN   = "[WARN]"
$failed = $false

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  CrestVital pre-PR checks                 " -ForegroundColor Cyan
Write-Host "  Repository: $(Split-Path $RootDir -Leaf)" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# ---------------------------------------------------------------------------
# Step 1 — Branch guard
# ---------------------------------------------------------------------------
Write-Host "Step 1: Branch check" -ForegroundColor Yellow

$branch = git rev-parse --abbrev-ref HEAD 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "$FAIL  Not inside a git repository." -ForegroundColor Red
    exit 1
}
if ($branch -eq "main") {
    Write-Host "$FAIL  You are on 'main'. Create a feature branch first." -ForegroundColor Red
    Write-Host "       Example: git checkout -b feature/MVP-XX-short-desc" -ForegroundColor Gray
    exit 1
}
Write-Host "$PASS  Branch: $branch" -ForegroundColor Green

# ---------------------------------------------------------------------------
# Step 2 — PlatformIO build
# ---------------------------------------------------------------------------
Write-Host ""
Write-Host "Step 2: PlatformIO build" -ForegroundColor Yellow

if ($SkipBuild) {
    Write-Host "$WARN  Skipping build (-SkipBuild flag set)." -ForegroundColor DarkYellow
} else {
    if ($null -eq (Get-Command "pio" -ErrorAction SilentlyContinue)) {
        Write-Host "$FAIL  'pio' not found. Install PlatformIO Core: https://docs.platformio.org/en/latest/core/installation/index.html" -ForegroundColor Red
        $failed = $true
    } else {
        Write-Host "       Running: pio run -e lilygo-t-display-s3 ..." -ForegroundColor Gray
        pio run -e lilygo-t-display-s3 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "$FAIL  Build failed. Fix all errors and warnings before opening PR." -ForegroundColor Red
            $failed = $true
        } else {
            Write-Host "$PASS  Build succeeded with zero errors." -ForegroundColor Green
        }
    }
}

# ---------------------------------------------------------------------------
# Step 3 — Working tree state
# ---------------------------------------------------------------------------
Write-Host ""
Write-Host "Step 3: Working tree" -ForegroundColor Yellow

$dirty = git status --porcelain 2>&1
if ($dirty) {
    Write-Host "$WARN  Uncommitted changes in working tree:" -ForegroundColor DarkYellow
    $dirty | ForEach-Object { Write-Host "       $_" -ForegroundColor Gray }
    Write-Host "       Review each changed file before committing." -ForegroundColor Gray
} else {
    Write-Host "$PASS  Working tree clean." -ForegroundColor Green
}

# ---------------------------------------------------------------------------
# Step 4 — CHANGELOG.md
# ---------------------------------------------------------------------------
Write-Host ""
Write-Host "Step 4: CHANGELOG.md" -ForegroundColor Yellow

$changedFiles = git diff --name-only HEAD 2>&1
if ($changedFiles -match "CHANGELOG\.md") {
    Write-Host "$PASS  CHANGELOG.md updated." -ForegroundColor Green
} else {
    Write-Host "$WARN  CHANGELOG.md not updated. Add an entry under [Unreleased] if user-visible." -ForegroundColor DarkYellow
}

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
if ($failed) {
    Write-Host "  RESULT: ISSUES FOUND - fix before PR   " -ForegroundColor Red
    Write-Host "============================================" -ForegroundColor Cyan
    Write-Host ""
    exit 1
} else {
    Write-Host "  RESULT: READY FOR OWNER REVIEW         " -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  Next steps (OWNER ONLY - no AI commits):" -ForegroundColor Gray
    Write-Host "  1. Review all changed files manually" -ForegroundColor Gray
    Write-Host "  2. git add <files>" -ForegroundColor Gray
    Write-Host "  3. git commit -m 'type: description #MVP-XX'" -ForegroundColor Gray
    Write-Host "  4. git push && open PR" -ForegroundColor Gray
    Write-Host ""
    exit 0
}
