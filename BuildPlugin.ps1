# Build Game Launcher Cloud Plugin for Unreal Engine 5.7
# Copyright Game Launcher Cloud. All Rights Reserved.

$ErrorActionPreference = "Stop"

# Configuration
$UE_VERSION = "5.7"
$PLUGIN_VERSION = "1.0.0"
$UE_PATH = "E:\Epic Games\UE_5.7"
$PLUGIN_NAME = "GameLauncherCloud"

# Paths
$ProjectRoot = $PSScriptRoot
$PluginPath = Join-Path $ProjectRoot "Plugins\$PLUGIN_NAME\$PLUGIN_NAME.uplugin"
$PackageDir = Join-Path $ProjectRoot "Package_UE5.7"
$OutputZip = Join-Path $ProjectRoot "$PLUGIN_NAME-v$PLUGIN_VERSION-UE$UE_VERSION.zip"

# Validate paths
if (-not (Test-Path $PluginPath)) {
    Write-Host "ERROR: Plugin file not found: $PluginPath" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path "$UE_PATH\Engine\Build\BatchFiles\RunUAT.bat")) {
    Write-Host "ERROR: Unreal Engine not found at: $UE_PATH" -ForegroundColor Red
    exit 1
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Building $PLUGIN_NAME Plugin" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Plugin:  $PluginPath" -ForegroundColor White
Write-Host "UE:      $UE_VERSION" -ForegroundColor White
Write-Host "Output:  $PackageDir`n" -ForegroundColor White

# Clean previous build
if (Test-Path $PackageDir) {
    Write-Host "[1/4] Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item $PackageDir -Recurse -Force
    Write-Host "      Done`n" -ForegroundColor Green
}

# Build plugin
Write-Host "[2/4] Compiling plugin..." -ForegroundColor Yellow
& "$UE_PATH\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="$PluginPath" -Package="$PackageDir" -Rocket

if ($LASTEXITCODE -ne 0) {
    Write-Host "`nERROR: Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "`n      Build successful!`n" -ForegroundColor Green

# Verify binaries
Write-Host "[3/4] Verifying binaries..." -ForegroundColor Yellow
$Binaries = Get-ChildItem "$PackageDir\Binaries" -Recurse -File -Filter "*.dll" 2>$null
if ($Binaries.Count -eq 0) {
    Write-Host "      WARNING: No DLL files found!" -ForegroundColor Red
} else {
    foreach ($dll in $Binaries) {
        $sizeKB = [math]::Round($dll.Length / 1KB, 2)
        Write-Host "      [OK] $($dll.Name) ($sizeKB KB)" -ForegroundColor Green
    }
}
Write-Host ""

# Create ZIP
Write-Host "[4/4] Creating ZIP package..." -ForegroundColor Yellow

# Remove old ZIP
if (Test-Path $OutputZip) {
    Remove-Item $OutputZip -Force
}

# Create clean temp directory
$TempDir = Join-Path $ProjectRoot "Package_Temp"
if (Test-Path $TempDir) {
    Remove-Item $TempDir -Recurse -Force
}
New-Item -ItemType Directory -Path "$TempDir\$PLUGIN_NAME" | Out-Null

# Copy only necessary files (no Intermediate)
Copy-Item "$PackageDir\Config" "$TempDir\$PLUGIN_NAME\Config" -Recurse -ErrorAction SilentlyContinue
Copy-Item "$PackageDir\Resources" "$TempDir\$PLUGIN_NAME\Resources" -Recurse
Copy-Item "$PackageDir\Source" "$TempDir\$PLUGIN_NAME\Source" -Recurse
Copy-Item "$PackageDir\$PLUGIN_NAME.uplugin" "$TempDir\$PLUGIN_NAME\"

# Ensure Config directory exists
if (-not (Test-Path "$TempDir\$PLUGIN_NAME\Config")) {
    New-Item -ItemType Directory -Path "$TempDir\$PLUGIN_NAME\Config" | Out-Null
}

# Copy FilterPlugin.ini from source (RunUAT may not include it)
$SourceFilterPlugin = Join-Path $ProjectRoot "Plugins\$PLUGIN_NAME\Config\FilterPlugin.ini"
if (Test-Path $SourceFilterPlugin) {
    Copy-Item $SourceFilterPlugin "$TempDir\$PLUGIN_NAME\Config\" -Force
    Write-Host "      [OK] FilterPlugin.ini included" -ForegroundColor Green
}

# Copy example config file
$SourceExampleConfig = Join-Path $ProjectRoot "Plugins\$PLUGIN_NAME\Config\glc_config_example.json"
if (Test-Path $SourceExampleConfig) {
    Copy-Item $SourceExampleConfig "$TempDir\$PLUGIN_NAME\Config\" -Force
    Write-Host "      [OK] glc_config_example.json included" -ForegroundColor Green
}

# Remove user configuration file (contains API keys)
Remove-Item "$TempDir\$PLUGIN_NAME\Config\glc_config.json" -Force -ErrorAction SilentlyContinue

# Remove .pdb files (debug symbols not needed for distribution)
Get-ChildItem "$TempDir\$PLUGIN_NAME\Binaries\Win64" -Filter "*.pdb" -ErrorAction SilentlyContinue | Remove-Item -Force

# Create ZIP
Compress-Archive -Path "$TempDir\$PLUGIN_NAME" -DestinationPath $OutputZip -Force

# Cleanup temp
Remove-Item $TempDir -Recurse -Force

# Cleanup Package_UE5.7 directory
Write-Host "      Cleaning build artifacts..." -ForegroundColor Yellow
Remove-Item $PackageDir -Recurse -Force -ErrorAction SilentlyContinue

$ZipSizeMB = [math]::Round((Get-Item $OutputZip).Length / 1MB, 2)
Write-Host "      Done`n" -ForegroundColor Green

# Summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Build Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Package:  $OutputZip" -ForegroundColor White
Write-Host "Size:     $ZipSizeMB MB" -ForegroundColor White
Write-Host "`nReady for Fab submission!`n" -ForegroundColor Green
