$ErrorActionPreference = "Stop"

. "$PSScriptRoot\Common.ps1"

$EnginePath = Get-TestGameEnginePath

$RunUAT = Join-Path $EnginePath "Engine\Build\BatchFiles\RunUAT.bat"

$OutputDirectory = Join-Path $ProjectRoot "ServerBuild"

$Map = "/Game/Blueprints/Maps/Crucible/L_Crucible01"

Write-Host ""
Write-Host "Building TestGame Dedicated Server..."
Write-Host "Map: $Map"
Write-Host "Output: $OutputDirectory"
Write-Host ""

& $RunUAT BuildCookRun `
    "-project=$ProjectFile" `
    -noP4 `
    -server `
    -noclient `
    -serverconfig=Development `
    -platform=Win64 `
    -build `
    -cook `
    "-map=$Map" `
    -stage `
    -pak `
    -archive `
    "-archivedirectory=$OutputDirectory"

if ($LASTEXITCODE -ne 0) {
    throw "Dedicated server build failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "Dedicated server build successful."
Write-Host "Output: $OutputDirectory"