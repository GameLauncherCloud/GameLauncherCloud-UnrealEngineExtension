# Configuration Guide

## Overview

The Game Launcher Cloud plugin stores its configuration in a JSON file. This guide explains how to configure the plugin for different environments and use cases.

## Configuration File Location

The plugin saves configuration to:
```
YourProject/Plugins/GameLauncherCloud/Config/glc_config.json
```

**⚠️ Security Warning:** This file contains your authentication token. Never commit it to version control!

## Configuration Structure

```json
{
  "authToken": "your-jwt-token-here",
  "userEmail": "user@example.com",
  "apiUrl": "https://app.gamelauncher.cloud"
}
```

### Fields

| Field | Type | Description | Required |
|-------|------|-------------|----------|
| `authToken` | string | JWT authentication token from login | No (auto-generated on login) |
| `userEmail` | string | User's email address | No (auto-set on login) |
| `apiUrl` | string | Game Launcher Cloud API base URL | Yes |

## Environment Setup

### Production (Default)
```json
{
  "apiUrl": "https://app.gamelauncher.cloud"
}
```

### Local Development
```json
{
  "apiUrl": "https://localhost:7777"
}
```

### Staging
```json
{
  "apiUrl": "https://staging.gamelauncher.cloud"
}
```

## Security Best Practices

### 1. Add to .gitignore

Always exclude the config file from version control:

```gitignore
# Game Launcher Cloud Config (contains API keys)
Plugins/GameLauncherCloud/Config/glc_config.json
```

### 2. Use Example Config

The plugin includes `glc_config.example.json`:

```json
{
  "apiKey": "your-api-key-here",
  "apiUrl": "https://app.gamelauncher.cloud",
  "environment": "production"
}
```

**To use:**
1. Copy `glc_config.example.json` to `Config/glc_config.json`
2. Replace `your-api-key-here` with your actual API key
3. Never commit the real `glc_config.json`

### 3. Team Sharing

For team environments:

1. Each developer maintains their own `glc_config.json`
2. Share the example config file via version control
3. Use team documentation to explain setup
4. Never share actual tokens or API keys

## Advanced Configuration

### Custom API Endpoint

To use a custom API endpoint:

```json
{
  "apiUrl": "https://custom.yourdomain.com"
}
```

### Multiple Configurations

For CI/CD pipelines, you can use environment variables or build scripts to generate configs:

```bash
# Example: Generate config for CI/CD
echo '{
  "apiUrl": "https://app.gamelauncher.cloud",
  "authToken": "$GLC_API_TOKEN"
}' > Plugins/GameLauncherCloud/Config/glc_config.json
```

## Troubleshooting

### Config Not Loading

**Problem:** Plugin doesn't remember login
**Solution:** 
1. Check file exists at correct path
2. Verify JSON is valid (use JSON validator)
3. Check file permissions (must be readable/writable)

### Invalid Token Error

**Problem:** "Not authenticated" or "Invalid token"
**Solution:**
1. Delete `glc_config.json`
2. Restart Unreal Engine
3. Login again with your API key

### API URL Not Working

**Problem:** Connection errors to API
**Solution:**
1. Verify the API URL is correct
2. Check internet connection
3. Try default production URL: `https://app.gamelauncher.cloud`
4. Check firewall settings

## Migration from Unity Extension

If you're migrating from the Unity extension:

1. The Unreal plugin uses the **same API**
2. You can use the **same API key**
3. Just login in the Unreal plugin with your existing credentials
4. No need to create a new account

## Config File Lifecycle

### Creation
- Created automatically on first login
- Plugin creates the Config directory if it doesn't exist

### Updates
- Updated every time you login
- Saves auth token after successful authentication
- Updates user email on login

### Deletion
- Deleted when you logout
- Can be manually deleted to reset plugin state

## Example Configs

### Minimal Config (First Launch)
```json
{
  "apiUrl": "https://app.gamelauncher.cloud"
}
```

### After Login
```json
{
  "authToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "userEmail": "developer@example.com",
  "apiUrl": "https://app.gamelauncher.cloud"
}
```

### Development Environment
```json
{
  "authToken": "dev_token_here",
  "userEmail": "dev@localhost",
  "apiUrl": "https://localhost:7777"
}
```

## Support

Need help with configuration?

- 📧 Email: support@gamelauncher.cloud
- 💬 Discord: https://discord.com/invite/FpWvUQ2CJP
- 📚 Docs: https://help.gamelauncher.cloud

---

Made with ❤️ by the Game Launcher Cloud team
