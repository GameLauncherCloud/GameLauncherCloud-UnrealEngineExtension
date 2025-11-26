<div align="center">
   
# Game Launcher Cloud - Manager for Unreal Engine
### **The Next-Generation Custom Game Launchers Creator Platform**
   
<img width="2800" height="720" alt="Game Launcher Cloud for Unreal Engine - Full Logo with Background" src="https://github.com/user-attachments/assets/0c1dde48-a813-4d00-9fe4-86e1067d4ad6" />

**Build and upload your game from Unreal Engine to Game Launcher Cloud!**

[![Website](https://img.shields.io/badge/Website-gamelauncher.cloud-blue?style=for-the-badge&logo=internet-explorer)](https://gamelauncher.cloud/)
[![Status](https://img.shields.io/badge/Status-Live-success?style=for-the-badge)](https://gamelauncher.cloud/)
[![Platform](https://img.shields.io/badge/Platform-Cross--Platform-orange?style=for-the-badge)](https://gamelauncher.cloud/)
[![Unreal Engine Extension](https://img.shields.io/badge/UnrealEngine-Marketplace-purple?style=for-the-badge&logo=unreal-engine)](https://www.fab.com/listings/af88972c-9d69-42cd-bcaa-109bb8dd31c6)

</div>

## 🌟 Features

A powerful Unreal Engine Editor plugin that allows you to build and upload your game patches directly to [Game Launcher Cloud](https://gamelauncher.cloud) platform from within Unreal Engine.

### ✓ **Connect to Your Account**
- Easy authentication using **API Key**
- Secure connection to Game Launcher Cloud backend
- Persistent login sessions

### ✓ **Build and Upload Patches**
- Build your Unreal Engine game directly from the editor
- Automatic compression and optimization
- Upload builds to Game Launcher Cloud with one click
- Real-time upload progress tracking
- Support for Windows, Linux, and macOS builds

### ✓ **Tips and Best Practices**
- Receive helpful tips to improve patch quality
- Learn optimization techniques
- Best practices for game distribution
- Build size recommendations

## 📦 Installation

### Method 1: Install from Fab (Recommended)

1. [Download the latest release from Fab](https://www.fab.com/listings/af88972c-9d69-42cd-bcaa-109bb8dd31c6)
2. Extract to your Unreal Engine's plugin folder:
   ```
   UE_5.X/Engine/Plugins/Marketplace/GameLauncherCloud/
   ```
3. Restart Unreal Engine
4. The plugin will be available for all projects

### Method 2: Install from GitHub

1. Download the latest release from [GitHub Releases](https://github.com/GameLauncherCloud/GameLauncherCloud-UnrealEngineExtension/releases)
2. Copy the `Plugin` folder to your project's `Plugins` folder:
   ```
   YourProject/Plugins/GameLauncherCloud/
   ```
3. Restart Unreal Engine
4. Open your project
5. Go to **Edit > Plugins**
6. Search for "Game Launcher Cloud"
7. Enable the plugin
8. Restart the editor when prompted

## 🚀 Quick Start

### Step 1: Get Your API Key

1. Go to [Game Launcher Cloud API Keys](https://app.gamelauncher.cloud/user/api-keys)
2. Click **Create New API Key**
3. Give it a name (e.g., "Unreal Engine Plugin")
4. Copy your API key

### Step 2: Open the Manager

1. In Unreal Engine, go to **Tools** menu
2. Click **Game Launcher Cloud**
3. The manager window will open

### Step 3: Login

1. In the Manager window, paste your API Key
2. Click **Login with API Key**
3. You should see "Login successful!" message

### Step 4: Build and Upload

1. Click **Load My Apps** to see your available apps
2. Select the app you want to upload to
3. (Optional) Write some build notes
4. Click **Build & Upload to Game Launcher Cloud**
5. Wait for the build and upload to complete

## 🔧 Requirements

- **Unreal Engine 5.0** or newer (compatible with UE4.27+)
- **Visual Studio 2019** or newer (for building from source)
- Active **Game Launcher Cloud** account
- **.NET Framework 4.7.2** or newer

## 📝 Configuration

The plugin saves your settings in:
```
YourProject/Plugins/GameLauncherCloud/Config/glc_config.json
```

**Note:** Add this file to `.gitignore` to avoid committing your API key!

Example `.gitignore` entry:
```
# Game Launcher Cloud Config (contains API keys)
Plugins/GameLauncherCloud/Config/glc_config.json
```

## 💡 Tips for Better Patches

### Optimize Build Size
- Compress textures appropriately
- Remove unused assets
- Use pak file compression
- Enable code stripping in Project Settings

### Use Descriptive Build Notes
Always include:
- Version number
- New features added
- Bugs fixed
- Known issues

### Test Before Uploading
- Run the packaged build locally first
- Check for crashes or errors
- Verify all features work
- Test performance on target hardware

## 🏗️ Building from Source

1. Clone this repository
2. Copy the plugin to your project's `Plugins` folder
3. Right-click your `.uproject` file
4. Select **Generate Visual Studio project files**
5. Open the `.sln` file in Visual Studio
6. Build the solution in Development Editor configuration

## 🤝 Support

Need help? We're here for you!

- 📧 Email: support@gamelauncher.cloud
- 💬 Discord: [Join our community](https://discord.com/invite/FpWvUQ2CJP)
- 📚 Documentation: [help.gamelauncher.cloud](https://help.gamelauncher.cloud)
- 🌐 Website: [gamelauncher.cloud](https://gamelauncher.cloud)

## 📝 License

This plugin is provided free of charge for use with Game Launcher Cloud platform.

## 🎮 About Game Launcher Cloud

Game Launcher Cloud is a comprehensive platform for game developers to:
- Create custom game launchers in minutes
- Distribute game patches efficiently
- Manage multiple games and versions
- Track downloads and analytics
- Provide seamless updates to players

Visit [gamelauncher.cloud](https://gamelauncher.cloud) to learn more!

---

Made with ❤️ by the Game Launcher Cloud team
