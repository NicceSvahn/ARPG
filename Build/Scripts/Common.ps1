$ErrorActionPreference = "Stop"

$ScriptRoot = $PSScriptRoot
$ProjectRoot = Resolve-Path (Join-Path $ScriptRoot "..\..")

$ProjectFile = Join-Path $ProjectRoot "TestGame.uproject"
$EngineConfigFile = Join-Path $ProjectRoot "Engine.json"

if (-not (Test-Path $ProjectFile)) {
    throw "Could not find TestGame.uproject at: $ProjectFile"
}

if (-not (Test-Path $EngineConfigFile)) {
    throw "Could not find Engine.json at: $EngineConfigFile"
}

$EngineConfig = Get-Content $EngineConfigFile -Raw | ConvertFrom-Json

$EngineAssociation = $EngineConfig.engineAssociation

$RegistryPath = "HKCU:\SOFTWARE\Epic Games\Unreal Engine\Builds"

function Get-TestGameEnginePath {

    if (-not (Test-Path $RegistryPath)) {
        throw "Unreal Engine Builds registry key does not exist."
    }

    $Registry = Get-ItemProperty $RegistryPath

    $EnginePath = $Registry.$EngineAssociation

    if (-not $EnginePath) {
        throw @"
Required Unreal Engine installation is not registered.

Engine association:
$EngineAssociation

Run:

    .\Build\Scripts\Setup.ps1
"@
    }

    if (-not (Test-Path $EnginePath)) {
        throw "Registered Unreal Engine path does not exist: $EnginePath"
    }

    return $EnginePath
}

function Get-UnrealEditorPath {

    $EnginePath = Get-TestGameEnginePath

    $Editor = Join-Path $EnginePath "Engine\Binaries\Win64\UnrealEditor.exe"

    if (-not (Test-Path $Editor)) {
        throw "Could not find UnrealEditor.exe at: $Editor"
    }

    return $Editor
}

function Get-UnrealBuildScript {

    $EnginePath = Get-TestGameEnginePath

    $BuildScript = Join-Path $EnginePath "Engine\Build\BatchFiles\Build.bat"

    if (-not (Test-Path $BuildScript)) {
        throw "Could not find Build.bat at: $BuildScript"
    }

    return $BuildScript
}