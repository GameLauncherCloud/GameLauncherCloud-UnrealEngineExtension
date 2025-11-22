# Installing Game Launcher Cloud Plugin

## Method 1: Install to Existing Project

1. **Copy the plugin to your project:**
   ```
   YourProject/
   └── Plugins/
       └── GameLauncherCloud/
           ├── Source/
           ├── Resources/
           ├── Config/
           └── GameLauncherCloud.uplugin
   ```

2. **Add plugin to your .uproject file:**
   
   Open your project's `.uproject` file and add this to the `Plugins` array:
   ```json
   {
     "Plugins": [
       {
         "Name": "GameLauncherCloud",
         "Enabled": true
       }
     ]
   }
   ```

3. **Generate project files:**
   - Right-click your `.uproject` file
   - Select "Generate Visual Studio project files"

4. **Open and build:**
   - Open the `.sln` file in Visual Studio
   - Build the solution (Development Editor configuration)
   - Launch Unreal Engine

5. **Verify installation:**
   - In Unreal Engine, go to **Edit > Plugins**
   - Search for "Game Launcher Cloud"
   - Make sure it's enabled
   - Restart if needed

## Method 2: Test with Example Project

This repository includes an example project file for testing:

1. **Copy the entire plugin folder:**
   ```
   GameLauncherCloud-UnrealExtension/
   ```

2. **Create a test project structure:**
   ```
   TestProject/
   ├── TestProject.uproject (copy ExampleProject.uproject and rename)
   └── Plugins/
       └── GameLauncherCloud/
           └── [plugin files]
   ```

3. **Generate and build:**
   - Right-click `TestProject.uproject`
   - Generate Visual Studio project files
   - Build in Visual Studio
   - Launch

## Method 3: Install to Engine

Install the plugin globally for all projects:

1. **Copy to engine plugins folder:**
   ```
   C:/Program Files/Epic Games/UE_5.X/Engine/Plugins/Marketplace/GameLauncherCloud/
   ```

2. **Restart Unreal Engine**

3. **Enable in project:**
   - Open any project
   - Go to **Edit > Plugins**
   - Search for "Game Launcher Cloud"
   - Enable it

## Troubleshooting

### Plugin doesn't appear in Plugins menu
- Make sure the plugin folder structure is correct
- Check that `GameLauncherCloud.uplugin` is in the root of the plugin folder
- Verify the engine version in the `.uplugin` file matches your installation
- Try regenerating project files

### Build errors
- Check that all source files are present
- Verify Visual Studio is properly configured for Unreal Engine
- Make sure you're building in Development Editor configuration
- Check the Output Log for specific errors

### Plugin loads but crashes
- Check the Output Log for error messages
- Verify all dependencies are available
- Try disabling other plugins to check for conflicts
- Rebuild the plugin from scratch

## Verifying Installation

Once installed, you should see:

1. **Tools menu item:**
   - Open Unreal Engine
   - Look for **Tools > Game Launcher Cloud** in the menu bar

2. **Toolbar button:**
   - A toolbar button should appear (if configured)

3. **Plugin manager:**
   - **Edit > Plugins**
   - Search "Game Launcher Cloud"
   - Should show as enabled

## Next Steps

After successful installation, see `QUICK_START.md` for usage instructions.

---

Need help? Contact support@gamelauncher.cloud
