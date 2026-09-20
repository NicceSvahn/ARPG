param(
    [Parameter(Mandatory = $true)]
    [string]$EnginePath
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\Common.ps1"

$EnginePath = (Resolve-Path $EnginePath).Path

$EditorPath = Join-Path $EnginePath "Engine\Binaries\Win64\UnrealEditor.exe"

if (-not (Test-Path $EditorPath)) {
    throw @"
This does not appear to be a valid TestGame Unreal Engine installation.

Expected:
$EditorPath
"@
}

Write-Host ""
Write-Host "TestGame Developer Setup"
Write-Host "========================"
Write-Host ""
Write-Host "Engine: $EnginePath"
Write-Host "Association: $EngineAssociation"
Write-Host ""

$RegistryNativePath = "HKCU\SOFTWARE\Epic Games\Unreal Engine\Builds"

& reg.exe add $RegistryNativePath `
    /v $EngineAssociation `
    /t REG_SZ `
    /d $EnginePath `
    /f

if ($LASTEXITCODE -ne 0) {
    throw "Failed to register Unreal Engine."
}

Write-Host ""
Write-Host "Unreal Engine registered successfully."