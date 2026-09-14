param(
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

# ------------------------------------------------------------
# Project configuration
# ------------------------------------------------------------

$ProjectName = "TestGame"

$ProjectRoot = $PSScriptRoot
$UProject = Join-Path $ProjectRoot "$ProjectName.uproject"
$Solution = Join-Path $ProjectRoot "$ProjectName.sln"

# Change this if your UE installation is somewhere else.
$EngineRoot = ""

$UBT = Join-Path `
    $EngineRoot `
    "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"

# ------------------------------------------------------------
# Validation
# ------------------------------------------------------------

if (-not (Test-Path $UProject)) {
    Write-Error "Could not find project: $UProject"
}

if (-not (Test-Path $UBT)) {
    Write-Error "Could not find UnrealBuildTool: $UBT"
}

Write-Host ""
Write-Host "======================================"
Write-Host " Unreal Project Generator"
Write-Host "======================================"
Write-Host ""
Write-Host "Project: $UProject"
Write-Host "Engine:  $EngineRoot"
Write-Host ""

# ------------------------------------------------------------
# Generate Visual Studio project files
# ------------------------------------------------------------

Write-Host "[1/3] Generating Visual Studio project files..."

& $UBT `
    -projectfiles `
    -project="$UProject" `
    -game `
    -engine `
    -progress

if ($LASTEXITCODE -ne 0) {
    Write-Error "Project file generation failed."
}

Write-Host "Project files generated successfully."
Write-Host ""

# ------------------------------------------------------------
# Build project
# ------------------------------------------------------------

if (-not $SkipBuild) {

    Write-Host "[2/3] Building $ProjectName Editor..."

    & $UBT `
        "${ProjectName}Editor" `
        Win64 `
        Development `
        "-Project=$UProject" `
        -WaitMutex `
        -FromMsBuild

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed."
    }

    Write-Host ""
    Write-Host "Build completed successfully."
}
else {
    Write-Host "[2/3] Build skipped."
}

# ------------------------------------------------------------
# Open Visual Studio
# ------------------------------------------------------------

Write-Host ""
Write-Host "[3/3] Opening solution..."

if (-not (Test-Path $Solution)) {
    Write-Error "Solution was not generated: $Solution"
}

Start-Process $Solution

Write-Host ""
Write-Host "Done."