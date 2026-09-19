$ErrorActionPreference = "Stop"

. "$PSScriptRoot\Common.ps1"

$BuildScript = Get-UnrealBuildScript

Write-Host ""
Write-Host "Building TestGameEditor..."
Write-Host ""

& $BuildScript `
    TestGameEditor `
    Win64 `
    Development `
    "-Project=$ProjectFile" `
    -WaitMutex

if ($LASTEXITCODE -ne 0) {
    throw "TestGameEditor build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "TestGameEditor build successful."