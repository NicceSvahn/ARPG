$ErrorActionPreference = "Stop"

. "$PSScriptRoot\Common.ps1"

$Editor = Get-UnrealEditorPath

Write-Host "Starting TestGame..."
Write-Host "Engine: $Editor"
Write-Host "Project: $ProjectFile"

Start-Process `
    -FilePath $Editor `
    -ArgumentList "`"$ProjectFile`""