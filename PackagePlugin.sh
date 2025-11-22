#!/bin/bash
# Game Launcher Cloud - Unreal Engine Plugin Packaging Script
# This script helps package the plugin for distribution (Linux/Mac)

PLUGIN_NAME="GameLauncherCloud"
VERSION="1.0.0"
OUTPUT_DIR="PackagedPlugin_${VERSION}"

echo "========================================"
echo "Game Launcher Cloud Plugin Packager"
echo "========================================"
echo ""

echo "Packaging plugin version ${VERSION}..."
echo ""

# Create output directory
mkdir -p "${OUTPUT_DIR}"

# Copy plugin files
echo "Copying plugin files..."
cp -r Source "${OUTPUT_DIR}/"
cp -r Resources "${OUTPUT_DIR}/"
cp -r Config "${OUTPUT_DIR}/"
cp GameLauncherCloud.uplugin "${OUTPUT_DIR}/"
cp README.md "${OUTPUT_DIR}/"
cp LICENSE "${OUTPUT_DIR}/"
cp CHANGELOG.md "${OUTPUT_DIR}/"
cp QUICK_START.md "${OUTPUT_DIR}/"
cp glc_config.example.json "${OUTPUT_DIR}/"

echo ""
echo "========================================"
echo "Plugin packaged successfully!"
echo "Output directory: ${OUTPUT_DIR}"
echo "========================================"
echo ""
echo "To use this plugin:"
echo "1. Copy the ${OUTPUT_DIR} folder to your project's Plugins directory"
echo "2. Rename it to GameLauncherCloud"
echo "3. Restart Unreal Engine"
echo "4. Enable the plugin in Edit > Plugins"
echo ""
