# PowerShell script to build and run Bug #2 regression tests
# Usage: .\run_bug2_tests.ps1

param(
    [string]$QtPath = "C:\Qt\6.5.3\msvc2019_64\bin",
    [string]$Configuration = "debug",
    [string]$VSPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
)

Write-Host "=== Pencil2D Bug #2 Test Runner ===" -ForegroundColor Cyan
Write-Host ""

# Initialize MSVC environment
$vcvarsPath = Join-Path $VSPath "vcvars64.bat"
if (Test-Path $vcvarsPath) {
    Write-Host "Setting up MSVC environment from: $VSPath" -ForegroundColor Green

    # Run vcvars64.bat and capture environment variables
    $tempFile = [System.IO.Path]::GetTempFileName()
    cmd /c "`"$vcvarsPath`" && set" > $tempFile

    Get-Content $tempFile | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }
    Remove-Item $tempFile
} else {
    Write-Host "Warning: Visual Studio environment script not found at: $vcvarsPath" -ForegroundColor Yellow
    Write-Host "Please specify correct VS path with -VSPath parameter" -ForegroundColor Yellow
    Write-Host "Example: .\run_bug2_tests.ps1 -VSPath 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build'" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Common paths to try:" -ForegroundColor Yellow
    Write-Host "  VS 2022: C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build" -ForegroundColor Cyan
    Write-Host "  VS 2019: C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build" -ForegroundColor Cyan
    Write-Host "  VS 2017: C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build" -ForegroundColor Cyan
    exit 1
}

# Set Qt path
if (Test-Path $QtPath) {
    Write-Host "Setting Qt path: $QtPath" -ForegroundColor Green
    $env:PATH += ";$QtPath"
} else {
    Write-Host "Warning: Qt path not found: $QtPath" -ForegroundColor Yellow
    Write-Host "Please specify correct Qt path with -QtPath parameter" -ForegroundColor Yellow
    Write-Host "Example: .\run_bug2_tests.ps1 -QtPath C:\Qt\6.5.3\msvc2019_64\bin" -ForegroundColor Yellow
    exit 1
}

# Navigate to project root
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir
Set-Location $projectRoot

Write-Host ""
Write-Host "Project root: $projectRoot" -ForegroundColor Green
Write-Host ""

# Generate Visual Studio solution
Write-Host "Generating Visual Studio solution..." -ForegroundColor Cyan
qmake pencil2d.pro -tp vc -r

if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: qmake failed" -ForegroundColor Red
    exit 1
}

Write-Host "Done." -ForegroundColor Green
Write-Host ""

# Build tests
Write-Host "Building tests target..." -ForegroundColor Cyan
msbuild ./pencil2d.sln /t:tests /p:Configuration=$Configuration /v:minimal

if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Build failed" -ForegroundColor Red
    exit 1
}

Write-Host "Build complete." -ForegroundColor Green
Write-Host ""

# Find test executable
$testExe = ".\$Configuration\tests.exe"

if (-not (Test-Path $testExe)) {
    Write-Host "Error: Test executable not found at $testExe" -ForegroundColor Red
    exit 1
}

# Run Bug #2 tests
Write-Host "=== Running Bug #2 Regression Tests ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "Test 1: LayerBitmap::needSaveFrame - Bug #2 Regression Tests" -ForegroundColor Yellow
& $testExe "LayerBitmap::needSaveFrame - Bug #2 Regression Tests"
$test1Result = $LASTEXITCODE

Write-Host ""
Write-Host "Test 2: LayerBitmap::saveKeyFrameFile - Integration Test for Bug #2" -ForegroundColor Yellow
& $testExe "LayerBitmap::saveKeyFrameFile - Integration Test for Bug #2"
$test2Result = $LASTEXITCODE

Write-Host ""
Write-Host "=== Test Results ===" -ForegroundColor Cyan

if ($test1Result -eq 0 -and $test2Result -eq 0) {
    Write-Host "✓ All Bug #2 tests PASSED!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Bug #2 fix is working correctly:" -ForegroundColor Green
    Write-Host "  - Moved frames are properly detected" -ForegroundColor Green
    Write-Host "  - Frame data will not be lost during save/reload" -ForegroundColor Green
    Write-Host "  - Bitmap layers now match vector layer behavior" -ForegroundColor Green
    exit 0
} else {
    Write-Host "✗ Some tests FAILED!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Test 1 result: $(if ($test1Result -eq 0) { 'PASS' } else { 'FAIL' })" -ForegroundColor $(if ($test1Result -eq 0) { 'Green' } else { 'Red' })
    Write-Host "Test 2 result: $(if ($test2Result -eq 0) { 'PASS' } else { 'FAIL' })" -ForegroundColor $(if ($test2Result -eq 0) { 'Green' } else { 'Red' })
    Write-Host ""
    Write-Host "Please investigate test failures before proceeding." -ForegroundColor Yellow
    exit 1
}
