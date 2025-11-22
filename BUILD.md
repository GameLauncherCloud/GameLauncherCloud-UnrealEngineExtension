# Building the Game Launcher Cloud Plugin

## Prerequisites

Before building the plugin, ensure you have:

- ✅ **Unreal Engine 5.0+** installed (or 4.27+)
- ✅ **Visual Studio 2019** or newer with C++ development tools
- ✅ **Windows SDK 10.0.18362.0** or newer
- ✅ **.NET Framework 4.7.2** or newer

## Build Methods

### Method 1: Build with Existing Project (Recommended)

This is the easiest way to build and test the plugin.

1. **Copy plugin to your project:**
   ```
   YourProject/
   └── Plugins/
       └── GameLauncherCloud/
           └── [copy all plugin files here]
   ```

2. **Generate Visual Studio files:**
   - Close Unreal Engine if it's running
   - Right-click your `.uproject` file
   - Select **"Generate Visual Studio project files"**
   - Wait for generation to complete

3. **Open solution:**
   - Open the generated `.sln` file in Visual Studio

4. **Build the plugin:**
   - Select **Development Editor** configuration
   - Select **Win64** platform
   - Press `Ctrl + Shift + B` or **Build → Build Solution**

5. **Launch Unreal Engine:**
   - Press `F5` to launch with debugger, or
   - Open the `.uproject` file normally

6. **Enable plugin:**
   - In Unreal Editor: **Edit → Plugins**
   - Search "Game Launcher Cloud"
   - Enable it
   - Restart when prompted

### Method 2: Build as Standalone Plugin

Build the plugin independently without a project.

1. **Create a minimal test project:**
   - Use the provided `ExampleProject.uproject`
   - Or create a blank C++ project in Unreal Engine

2. **Copy plugin:**
   ```
   TestProject/
   └── Plugins/
       └── GameLauncherCloud/
   ```

3. **Follow steps 2-6 from Method 1**

### Method 3: Install to Engine (Advanced)

Build and install the plugin globally for all projects.

1. **Copy to engine plugins:**
   ```
   C:/Program Files/Epic Games/UE_5.X/Engine/Plugins/Marketplace/GameLauncherCloud/
   ```

2. **Rebuild Unreal Engine:**
   - If you have engine source code:
     ```
     RunUAT BuildPlugin -Plugin="Path/To/GameLauncherCloud.uplugin" -Package="Output/Path"
     ```

3. **Restart Unreal Engine**

## Build Configurations

### Development Editor (Recommended for Testing)
- Faster compilation
- Includes debug symbols
- Editor optimizations enabled
- Use this for daily development

```
Configuration: Development Editor
Platform: Win64
```

### Shipping (For Distribution)
- Full optimizations
- No debug symbols
- Smallest binary size
- Use this for final release

```
Configuration: Shipping
Platform: Win64
```

## Build Output

After building, you'll find:

```
Plugins/GameLauncherCloud/
├── Binaries/
│   └── Win64/
│       ├── UnrealEditor-GameLauncherCloud.dll
│       └── UnrealEditor-GameLauncherCloudEditor.dll
├── Intermediate/
│   └── Build/
└── Source/
```

## Build Troubleshooting

### "Cannot open include file" errors

**Problem:** Missing header files
**Solution:**
```cpp
// Check that all headers exist:
// - GameLauncherCloudModule.h
// - GLCApiClient.h
// - GLCManagerWindow.h
// - GLCCommands.h
```

### "Unresolved external symbol" errors

**Problem:** Missing dependency modules
**Solution:** Check `.Build.cs` files have all required modules:
```csharp
PrivateDependencyModuleNames.AddRange(
    new string[]
    {
        "HTTP",
        "Json",
        "JsonUtilities",
        // ... etc
    }
);
```

### Plugin doesn't appear after build

**Problem:** Plugin not recognized by editor
**Solution:**
1. Check `.uplugin` file is valid JSON
2. Verify engine version matches: `"EngineVersion": "5.0"`
3. Regenerate project files
4. Clean and rebuild: `Build → Clean Solution`

### Build succeeds but crashes on load

**Problem:** Runtime errors
**Solution:**
1. Check Output Log in Unreal Editor
2. Look for module loading errors
3. Verify all dependencies are built
4. Try disabling other plugins to check for conflicts

### Slow build times

**Optimization tips:**
1. Use **Development Editor** instead of Debug
2. Enable incremental builds in VS
3. Close unnecessary programs
4. Use SSD for project files
5. Increase RAM allocation for VS

## Clean Build

To perform a clean build:

1. **Close Unreal Engine**

2. **Delete generated files:**
   ```
   Plugins/GameLauncherCloud/Binaries/
   Plugins/GameLauncherCloud/Intermediate/
   ```

3. **Regenerate project files:**
   - Right-click `.uproject`
   - Generate Visual Studio project files

4. **Rebuild in Visual Studio:**
   - `Build → Rebuild Solution`

## Building from Command Line

### Using MSBuild

```powershell
# Set Visual Studio environment
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

# Build solution
msbuild YourProject.sln /p:Configuration="Development Editor" /p:Platform=Win64 /t:GameLauncherCloudEditor
```

### Using Unreal Build Tool (UBT)

```powershell
# Direct UBT call
"C:\Program Files\Epic Games\UE_5.0\Engine\Binaries\DotNET\UnrealBuildTool.exe" ^
  GameLauncherCloudEditor Win64 Development ^
  -Project="C:\Path\To\YourProject.uproject" ^
  -WaitMutex
```

### Using RunUAT (Package Plugin)

```powershell
"C:\Program Files\Epic Games\UE_5.0\Engine\Build\BatchFiles\RunUAT.bat" ^
  BuildPlugin ^
  -Plugin="C:\Path\To\GameLauncherCloud.uplugin" ^
  -Package="C:\Output\GameLauncherCloud" ^
  -TargetPlatforms=Win64
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build Plugin

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Setup Unreal Engine
        # Setup UE here
        
      - name: Build Plugin
        run: |
          RunUAT BuildPlugin -Plugin="GameLauncherCloud.uplugin" -Package="Output"
```

## Verify Build

After building, verify the plugin works:

1. **Load test:**
   - Launch Unreal Engine
   - Check for errors in Output Log
   - Look for "GameLauncherCloud Module Started"

2. **UI test:**
   - Go to **Tools → Game Launcher Cloud**
   - Window should open without errors

3. **Functionality test:**
   - Try logging in with test API key
   - Verify all buttons respond
   - Check error handling

## Build Performance

Typical build times on different hardware:

| Hardware | Clean Build | Incremental |
|----------|-------------|-------------|
| High-end | 30-60s | 5-10s |
| Mid-range | 1-2 min | 10-20s |
| Low-end | 2-5 min | 20-40s |

**High-end**: i9/Ryzen 9, 32GB RAM, NVMe SSD
**Mid-range**: i5/Ryzen 5, 16GB RAM, SATA SSD
**Low-end**: i3/Ryzen 3, 8GB RAM, HDD

## Next Steps

After successful build:

1. ✅ Read `QUICK_START.md` for usage guide
2. ✅ See `CONFIGURATION.md` for config options
3. ✅ Check `DEVELOPER.md` for technical details
4. ✅ Test all features thoroughly

## Support

Build issues? Get help:

- 📧 Email: support@gamelauncher.cloud
- 💬 Discord: https://discord.com/invite/FpWvUQ2CJP
- 🐛 GitHub: Report issues on repository

---

Made with ❤️ by the Game Launcher Cloud team
