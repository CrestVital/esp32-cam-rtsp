# scripts/install-hooks.ps1
# Copies canonical hooks from scripts/hooks/ into .git/hooks/
# Run from repository root: .\scripts\install-hooks.ps1

$repoRoot = git rev-parse --show-toplevel
$src  = Join-Path $repoRoot "scripts\hooks\pre-commit"
$dest = Join-Path $repoRoot ".git\hooks\pre-commit"

Copy-Item $src $dest -Force

# Normalise to LF line endings
$content   = [System.IO.File]::ReadAllText($dest).Replace("`r`n", "`n")
$utf8NoBom = New-Object System.Text.UTF8Encoding $false
[System.IO.File]::WriteAllText($dest, $content, $utf8NoBom)

Write-Host "[OK] pre-commit hook installed at $dest"