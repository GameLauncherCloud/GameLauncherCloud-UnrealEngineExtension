# Changelog

All notable changes to the Game Launcher Cloud Unreal Engine plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2024-11-21

### Added
- Initial release of Game Launcher Cloud plugin for Unreal Engine
- API key authentication support
- App listing functionality
- Basic UI with login and build/upload tabs
- HTTP API client for Game Launcher Cloud backend communication
- Configuration management (saves auth token and settings)
- Tips and best practices section in UI
- Support for Unreal Engine 5.0+
- Slate-based editor UI
- Real-time upload progress tracking
- Build status monitoring (foundation)

### Features
- **Authentication**: Login with API key from Game Launcher Cloud dashboard
- **App Management**: Load and select from your available apps
- **User Interface**: Clean, intuitive Slate-based UI integrated into Unreal Engine editor
- **Configuration**: Persistent settings saved to project config
- **Tips**: Built-in tips for optimizing builds and uploads

### Known Limitations
- Full UAT (Unreal Automation Tool) integration is in development
- Build and packaging currently requires manual implementation
- Multipart upload for files > 500MB not yet implemented
- Build status monitoring uses polling (no real-time websocket updates)

### Technical Details
- Written in C++ using Unreal Engine's plugin system
- Uses HTTP module for API communication
- JSON serialization for API requests/responses
- Slate framework for UI components
- Editor-only module (doesn't ship with packaged games)

### Compatibility
- Unreal Engine 5.0 and newer
- Windows, Linux, and macOS development platforms
- Requires Visual Studio 2019+ for building from source

### Documentation
- README.md with full feature list and installation instructions
- QUICK_START.md with step-by-step guide for first-time users
- Inline code documentation with XML comments

---

## [Unreleased]

### Planned Features
- Full UAT integration for automated project packaging
- Multipart upload support for large builds (> 500MB)
- Real-time build status updates via websockets
- Build history viewer
- Advanced compression options
- Support for all Unreal Engine target platforms
- Build configuration presets
- Incremental patch support
- Blueprint API for custom workflows
- Command-line interface for CI/CD integration

### Future Improvements
- Better error handling and user feedback
- Retry logic for failed uploads
- Upload resumption for interrupted transfers
- Build validation before upload
- Automatic version detection from project settings
- Integration with Unreal Engine's build system
- Performance optimizations for large file handling

---

## How to Contribute

Found a bug or have a feature request? 
- 🐛 Report issues: [GitHub Issues](https://github.com/GameLauncherCloud/GameLauncherCloud-UnrealExtension/issues)
- 💡 Suggest features: [GitHub Discussions](https://github.com/GameLauncherCloud/GameLauncherCloud-UnrealExtension/discussions)
- 📧 Contact us: support@gamelauncher.cloud

---

Made with ❤️ by the Game Launcher Cloud team
