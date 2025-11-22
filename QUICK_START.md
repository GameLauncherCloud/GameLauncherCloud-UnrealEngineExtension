# Game Launcher Cloud - Manager for Unreal Engine
## Quick Start Guide

### 📋 Prerequisites

Before you start, make sure you have:
- ✅ Unreal Engine 5.0 or newer installed
- ✅ A Game Launcher Cloud account ([Sign up here](https://app.gamelauncher.cloud))
- ✅ At least one app created in your Game Launcher Cloud dashboard

### 🔑 Step 1: Get Your API Key

1. Log in to [Game Launcher Cloud API Keys](https://app.gamelauncher.cloud/user/api-keys)
2. Click **Create New API Key**
3. Give it a name (e.g., "Unreal Engine Plugin")
4. **Copy the API key** (you won't be able to see it again!)

### 📥 Step 2: Install the Plugin

#### Option A: Install to Project (Recommended)

1. Download the plugin from [Releases](https://github.com/GameLauncherCloud/GameLauncherCloud-UnrealEngineExtension/releases)
2. Extract the `GameLauncherCloud` folder to:
   ```
   YourProject/Plugins/GameLauncherCloud/
   ```
3. Restart Unreal Engine
4. Open your project
5. Go to **Edit** → **Plugins**
6. Search for "Game Launcher Cloud"
7. Check the box to enable it
8. Click **Restart Now**

#### Option B: Install to Engine

1. Download the plugin
2. Extract to Unreal Engine's plugin folder:
   ```
   C:/Program Files/Epic Games/UE_5.X/Engine/Plugins/Marketplace/GameLauncherCloud/
   ```
3. Restart Unreal Engine

### 🚀 Step 3: Open the Manager

1. In Unreal Engine, open the **Tools** menu from the top menu bar
2. Click **Game Launcher Cloud**
3. The Game Launcher Cloud Manager window will open

### 🔐 Step 4: Login

1. In the Manager window, you'll see the login screen
2. Paste your API Key in the text field
3. Click **Login with API Key**
4. You should see "Login successful!" message
5. The window will refresh to show the Build & Upload interface

### 🎮 Step 5: Load Your Apps

1. In the Build & Upload section, click **Load My Apps**
2. Wait a moment while it fetches your apps from the server
3. You'll see a message showing how many apps were loaded
4. Your apps will appear in the dropdown menu

### 📦 Step 6: Build and Upload Your First Patch

1. Select your app from the **Select App** dropdown
2. (Optional) Write build notes describing what changed:
   ```
   Example:
   v1.0.1
   - Fixed player movement bug
   - Added new level "Forest Temple"
   - Performance improvements
   ```
3. Click **Build & Upload to Game Launcher Cloud**
4. Watch the progress bar as your build is packaged and uploaded
5. Once done, you'll see a success message!

### ✅ Step 7: Verify Your Build

1. Go to [Game Launcher Cloud Dashboard](https://app.gamelauncher.cloud)
2. Navigate to your app
3. You should see your new build listed
4. The build will be processed automatically
5. Once processing is complete, it's ready for distribution!

---

## 🆘 Troubleshooting

### "Login failed" Error
- Make sure you copied the entire API key
- Check that your API key hasn't expired or been deleted
- Verify you have an active Game Launcher Cloud subscription
- Try creating a new API key

### "No apps found" Message
- Make sure you've created at least one app in your dashboard
- Go to [Dashboard](https://app.gamelauncher.cloud) → **Apps** → **Create New App**
- Click **Load My Apps** again after creating an app
- Check that you're logged in with the correct account

### Plugin Not Showing in Tools Menu
- Check that the plugin is enabled in **Edit** → **Plugins**
- Search for "Game Launcher Cloud" in the plugins list
- Make sure the checkbox is enabled
- Restart Unreal Engine if you just enabled it
- Check the Output Log for any error messages

### Build Fails
- Make sure your project compiles successfully first
- Check **Window** → **Developer Tools** → **Output Log** for errors
- Verify you have enough disk space
- Try packaging manually first: **File** → **Package Project** → **Windows (64-bit)**

### Upload Fails
- Check your internet connection
- Verify the build size is within your plan limits
- Make sure you have enough storage quota in your account
- Check the Output Log for detailed error messages
- Try uploading again (temporary network issues)

### Plugin Won't Load
- Check Unreal Engine version (requires 5.0+, compatible with 4.27+)
- Make sure Visual Studio is installed (for compiling C++ plugins)
- Right-click your `.uproject` → **Generate Visual Studio project files**
- Try rebuilding the plugin from Visual Studio
- Check the Output Log for compilation errors

---

## 💡 Tips for Success

### 1. Test Locally First
Always test your packaged build on your local machine before uploading:
```
File → Package Project → Windows (64-bit)
Navigate to your Builds folder
Run the .exe file
Verify everything works
```

### 2. Write Clear Build Notes
Good build notes help you and your team track changes:
```
✅ Good:
v1.2.0
- Fixed memory leak in level loading
- Added 5 new weapons
- Improved AI pathfinding
- Known issue: Audio crackling on level 3

❌ Bad:
"some fixes"
"new stuff"
"idk"
```

### 3. Use Version Control
Keep your project in Git or similar:
```bash
# Add to .gitignore
Plugins/GameLauncherCloud/Config/glc_config.json
Builds/
Saved/
```

### 4. Organize Your Builds
Create a naming convention for your builds:
```
MyGame_v1.0.0_Windows.zip
MyGame_v1.0.0_Linux.zip
MyGame_v1.0.1_Windows_Hotfix.zip
```

### 5. Monitor Your Storage
Keep an eye on your Game Launcher Cloud storage usage:
- Dashboard → **Settings** → **Storage**
- Delete old builds you no longer need
- Optimize assets to reduce build size

---

## 📞 Need More Help?

- 📧 Email: support@gamelauncher.cloud
- 💬 Discord: [Join our community](https://discord.com/invite/FpWvUQ2CJP)
- 📚 Full Documentation: [docs.gamelauncher.cloud](https://help.gamelauncher.cloud)
- 🐛 Report bugs: [GitHub Issues](https://github.com/GameLauncherCloud/GameLauncherCloud-UnrealEngineExtension/issues)

---

**Happy Building! 🎮✨**
