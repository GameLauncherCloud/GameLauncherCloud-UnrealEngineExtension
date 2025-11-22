# Developer Documentation

## Overview

This document provides technical information for developers who want to understand, modify, or contribute to the Game Launcher Cloud Unreal Engine plugin.

## Architecture

### Module Structure

The plugin consists of two modules:

1. **GameLauncherCloud** (Runtime Module)
   - Location: `Source/GameLauncherCloud/`
   - Type: Runtime
   - Purpose: Base module, can be loaded at runtime (currently minimal functionality)

2. **GameLauncherCloudEditor** (Editor Module)
   - Location: `Source/GameLauncherCloudEditor/`
   - Type: Editor
   - Purpose: Contains all editor UI and functionality
   - Dependencies: HTTP, Json, JsonUtilities, Slate, UnrealEd, etc.

### Key Components

#### 1. FGLCApiClient (`GLCApiClient.h/cpp`)
HTTP API client that handles all communication with Game Launcher Cloud backend.

**Key Methods:**
- `LoginWithApiKeyAsync()` - Authenticates using API key
- `GetAppListAsync()` - Fetches user's apps
- `CanUploadAsync()` - Validates upload eligibility
- `StartUploadAsync()` - Initiates upload process
- `UploadFileAsync()` - Uploads build file to presigned URL
- `NotifyFileReadyAsync()` - Notifies backend that upload is complete
- `GetBuildStatusAsync()` - Polls build processing status

**Response Structures:**
- `FGLCLoginResponse` - Login result with token
- `FGLCAppInfo` - App information
- `FGLCCanUploadResponse` - Upload validation
- `FGLCStartUploadResponse` - Upload URLs and build ID
- `FGLCBuildStatusResponse` - Build processing status

#### 2. SGLCManagerWindow (`GLCManagerWindow.h/cpp`)
Slate-based editor window providing the user interface.

**UI Components:**
- Login tab (when not authenticated)
- Build & Upload tab (when authenticated)
- Tips tab (always visible)

**Key Features:**
- Persistent configuration (saves to JSON)
- Real-time progress tracking
- Async API calls with callbacks
- Dynamic UI updates based on state

#### 3. FGLCCommands (`GLCCommands.h/cpp`)
UI command registration for menu integration.

#### 4. FGameLauncherCloudEditorModule (`GameLauncherCloudEditorModule.h/cpp`)
Main editor module that registers the tab spawner and menu items.

## Building from Source

### Prerequisites
- Unreal Engine 5.0+ source or binary
- Visual Studio 2019 or newer
- Windows SDK 10.0.18362.0 or newer

### Steps

1. **Clone the repository:**
```bash
git clone https://github.com/GameLauncherCloud/GameLauncherCloud-UnrealExtension.git
```

2. **Copy to your project:**
```bash
cp -r GameLauncherCloud-UnrealExtension YourProject/Plugins/GameLauncherCloud
```

3. **Generate project files:**
```bash
# Right-click your .uproject file
# Select "Generate Visual Studio project files"
```

4. **Open in Visual Studio:**
```bash
# Open YourProject.sln
```

5. **Build:**
```bash
# Build → Build Solution (Ctrl+Shift+B)
# Configuration: Development Editor
# Platform: Win64
```

## Configuration

### Config File Location
```
YourProject/Plugins/GameLauncherCloud/Config/glc_config.json
```

### Config Structure
```json
{
  "authToken": "your-jwt-token-here",
  "userEmail": "user@example.com",
  "apiUrl": "https://app.gamelauncher.cloud"
}
```

**Security Note:** Always add this file to `.gitignore`!

## API Integration

### Base URL
```
Production: https://app.gamelauncher.cloud
```

### Endpoints Used

#### Login
```
POST /api/cli/build/login-interactive
Body: { "apiKey": "..." }
Response: { "result": { "token": "...", "email": "...", ... } }
```

#### List Apps
```
GET /api/cli/build/list-apps
Headers: Authorization: Bearer {token}
Response: { "result": { "apps": [...] } }
```

#### Check Upload Eligibility
```
GET /api/cli/build/can-upload?fileSizeBytes={size}&appId={id}
Headers: Authorization: Bearer {token}
Response: { "result": { "canUpload": true, ... } }
```

#### Start Upload
```
POST /api/cli/build/start-upload
Headers: Authorization: Bearer {token}
Body: { "appId": 123, "fileName": "...", "fileSize": 123, ... }
Response: { "result": { "uploadUrl": "...", "appBuildId": 123, ... } }
```

#### Upload File
```
PUT {presigned_url}
Headers: Content-Type: application/octet-stream
Body: [binary file data]
```

#### Notify File Ready
```
POST /api/cli/build/file-ready
Headers: Authorization: Bearer {token}
Body: { "appBuildId": 123, "key": "..." }
```

#### Get Build Status
```
GET /api/cli/build/status/{buildId}
Headers: Authorization: Bearer {token}
Response: { "result": { "status": "Processing", ... } }
```

## Extending the Plugin

### Adding New Features

#### 1. Add API Method
Edit `GLCApiClient.h` and `GLCApiClient.cpp`:

```cpp
// In GLCApiClient.h
void YourNewMethodAsync(TFunction<void(bool, FString, FYourResponse)> Callback);

// In GLCApiClient.cpp
void FGLCApiClient::YourNewMethodAsync(TFunction<void(bool, FString, FYourResponse)> Callback)
{
    // Implementation
}
```

#### 2. Add UI Component
Edit `GLCManagerWindow.cpp`:

```cpp
TSharedRef<SWidget> SGLCManagerWindow::ConstructYourNewTab()
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [
            // Your UI here
        ];
}
```

#### 3. Add Menu Item
Edit `GameLauncherCloudEditorModule.cpp`:

```cpp
void FGameLauncherCloudEditorModule::RegisterMenus()
{
    // Add your menu item
}
```

## Testing

### Manual Testing Checklist

- [ ] Plugin loads without errors
- [ ] Manager window opens from Tools menu
- [ ] Login with valid API key succeeds
- [ ] Login with invalid API key shows error
- [ ] Load apps retrieves user's apps
- [ ] App selection works correctly
- [ ] Logout clears auth state
- [ ] Config saves and loads correctly
- [ ] UI updates reflect current state
- [ ] Error messages are clear and helpful

### Testing with Local Backend

To test against a local backend:

1. Update `ApiUrl` in `SGLCManagerWindow::Construct()`:
```cpp
ApiUrl = TEXT("https://localhost:7777");
```

2. The API client automatically bypasses SSL validation for localhost

## Common Issues

### Plugin Won't Load
- Check Output Log for compilation errors
- Verify all dependencies are available
- Try regenerating project files
- Clean and rebuild solution

### API Calls Failing
- Check Output Log for detailed error messages
- Verify API key is valid and not expired
- Check internet connection
- Confirm backend is accessible

### UI Not Updating
- Check if callbacks are being called
- Verify UI updates are on game thread
- Use `Construct()` to force full refresh

## Contributing

### Code Style
- Follow Unreal Engine coding standards
- Use `LOCTEXT_NAMESPACE` for all UI text
- Add XML documentation comments to public methods
- Keep files organized by functionality

### Pull Request Process
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit pull request with description

### Commit Messages
```
feat: Add new feature
fix: Fix bug description  
docs: Update documentation
refactor: Code restructure
test: Add tests
```

## Resources

- [Unreal Engine C++ API](https://docs.unrealengine.com/en-US/API/)
- [Slate UI Framework](https://docs.unrealengine.com/en-US/ProgrammingAndScripting/Slate/)
- [Plugin Development](https://docs.unrealengine.com/en-US/ProductionPipelines/Plugins/)
- [HTTP Module](https://docs.unrealengine.com/en-US/API/Runtime/HTTP/)

## Support

- 📧 Email: support@gamelauncher.cloud
- 💬 Discord: [Join our community](https://discord.com/invite/FpWvUQ2CJP)
- 🐛 GitHub Issues: [Report bugs](https://github.com/GameLauncherCloud/GameLauncherCloud-UnrealExtension/issues)

---

Made with ❤️ by the Game Launcher Cloud team
