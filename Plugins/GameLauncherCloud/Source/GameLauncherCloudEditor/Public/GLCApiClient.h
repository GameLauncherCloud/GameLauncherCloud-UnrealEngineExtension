// Copyright Game Launcher Cloud. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "Dom/JsonObject.h"

/// <summary>
/// Response structure for login operations
/// </summary>
struct FGLCLoginResponse
{
	FString Id;
	FString Username;
	FString Email;
	FString Token;
	TArray<FString> Roles;
	FString PlanName;
};

/// <summary>
/// App information structure
/// </summary>
struct FGLCAppInfo
{
	int64 Id;
	FString Name;
	FString Description;
	int32 BuildCount;
	bool IsOwnedByUser;
};

/// <summary>
/// Upload validation response
/// </summary>
struct FGLCCanUploadResponse
{
	bool CanUpload;
	int64 FileSizeBytes;
	int64 UncompressedSizeBytes;
	FString PlanName;
	int32 MaxCompressedSizeGB;
	int32 MaxUncompressedSizeGB;
};

/// <summary>
/// Start upload response with presigned URL
/// </summary>
struct FGLCStartUploadResponse
{
	int64 AppBuildId;
	FString UploadUrl;
	FString Key;
	FString FinalUrl;
};

/// <summary>
/// Build status response
/// </summary>
struct FGLCBuildStatusResponse
{
	int64 AppBuildId;
	int64 AppId;
	FString Status;
	FString FileName;
	FString BuildNotes;
	FString ErrorMessage;
	int64 FileSize;
	int64 CompressedFileSize;
	int32 StageProgress;
};

/// <summary>
/// HTTP API Client for Game Launcher Cloud backend communication
/// Handles authentication, app listing, and build upload operations
/// </summary>
class GAMELAUNCHERCLOUDEDITOR_API FGLCApiClient
{
public:
	FGLCApiClient(const FString& InBaseUrl, const FString& InAuthToken = TEXT(""));
	
	void SetAuthToken(const FString& Token);
	
	// Authentication
	void LoginWithApiKeyAsync(const FString& ApiKey, TFunction<void(bool, FString, FGLCLoginResponse)> Callback);
	
	// App management
	void GetAppListAsync(TFunction<void(bool, FString, TArray<FGLCAppInfo>)> Callback);
	
	// Build upload
	void CanUploadAsync(int64 FileSizeBytes, int64 UncompressedSizeBytes, int64 AppId, TFunction<void(bool, FString, FGLCCanUploadResponse)> Callback);
	void StartUploadAsync(int64 AppId, const FString& FileName, int64 FileSize, int64 UncompressedFileSize, const FString& BuildNotes, TFunction<void(bool, FString, FGLCStartUploadResponse)> Callback);
	void UploadFileAsync(const FString& PresignedUrl, const FString& FilePath, TFunction<void(bool, FString, float)> ProgressCallback);
	void NotifyFileReadyAsync(int64 AppBuildId, const FString& Key, TFunction<void(bool, FString)> Callback);
	
	// Build status
	void GetBuildStatusAsync(int64 AppBuildId, TFunction<void(bool, FString, FGLCBuildStatusResponse)> Callback);
	
	// Build cancellation
	void CancelBuildAsync(int64 AppBuildId, TFunction<void(bool, FString)> Callback);
	
	// Cancel active upload
	void CancelActiveUpload();

private:
	FString BaseUrl;
	FString AuthToken;
	
	// Active upload request tracking
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ActiveUploadRequest;
	
	// Helper functions
	TSharedPtr<FJsonObject> ParseJsonResponse(const FString& ResponseString);
	bool ExtractApiResult(TSharedPtr<FJsonObject> JsonObject, TSharedPtr<FJsonObject>& OutResult, FString& OutError);
};
