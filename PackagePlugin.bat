@echo off
REM Game Launcher Cloud - Unreal Engine Plugin Packaging Script
REM This script helps package the plugin for distribution

echo ========================================
echo Game Launcher Cloud Plugin Packager
echo ========================================
echo.

REM Set plugin name
set PLUGIN_NAME=GameLauncherCloud

REM Set version (you can modify this)
set VERSION=1.0.0

REM Create output directory
set OUTPUT_DIR=PackagedPlugin_%VERSION%
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo Packaging plugin version %VERSION%...
echo.

REM Copy plugin files
echo Copying plugin files...
xcopy /E /I /Y "Source" "%OUTPUT_DIR%\Source\"
xcopy /E /I /Y "Resources" "%OUTPUT_DIR%\Resources\"
xcopy /E /I /Y "Config" "%OUTPUT_DIR%\Config\"
copy /Y "GameLauncherCloud.uplugin" "%OUTPUT_DIR%\"
copy /Y "README.md" "%OUTPUT_DIR%\"
copy /Y "LICENSE" "%OUTPUT_DIR%\"
copy /Y "CHANGELOG.md" "%OUTPUT_DIR%\"
copy /Y "QUICK_START.md" "%OUTPUT_DIR%\"
copy /Y "glc_config.example.json" "%OUTPUT_DIR%\"

echo.
echo ========================================
echo Plugin packaged successfully!
echo Output directory: %OUTPUT_DIR%
echo ========================================
echo.
echo To use this plugin:
echo 1. Copy the %OUTPUT_DIR% folder to your project's Plugins directory
echo 2. Rename it to GameLauncherCloud
echo 3. Restart Unreal Engine
echo 4. Enable the plugin in Edit ^> Plugins
echo.

pause
