// Copyright Game Launcher Cloud. All Rights Reserved.

#include "GLCApiClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/FileHelper.h"

FGLCApiClient::FGLCApiClient(const FString& InBaseUrl, const FString& InAuthToken)
	: BaseUrl(InBaseUrl)
	, AuthToken(InAuthToken)
{
}

void FGLCApiClient::SetAuthToken(const FString& Token)
{
	AuthToken = Token;
}

TSharedPtr<FJsonObject> FGLCApiClient::ParseJsonResponse(const FString& ResponseString)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		return JsonObject;
	}
	
	return nullptr;
}

bool FGLCApiClient::ExtractApiResult(TSharedPtr<FJsonObject> JsonObject, TSharedPtr<FJsonObject>& OutResult, FString& OutError)
{
	if (!JsonObject.IsValid())
	{
		OutError = TEXT("Invalid JSON response");
		return false;
	}
	
	// Check both camelCase and PascalCase for compatibility
	bool IsSuccess = false;
	JsonObject->TryGetBoolField(TEXT("isSuccess"), IsSuccess) || JsonObject->TryGetBoolField(TEXT("IsSuccess"), IsSuccess);
	
	if (IsSuccess)
	{
		const TSharedPtr<FJsonObject>* ResultObject = nullptr;
		if (JsonObject->TryGetObjectField(TEXT("result"), ResultObject) || JsonObject->TryGetObjectField(TEXT("Result"), ResultObject))
		{
			if (ResultObject && (*ResultObject).IsValid())
			{
				OutResult = *ResultObject;
				return true;
			}
		}
	}
	
	// Extract error messages (try both camelCase and PascalCase)
	const TArray<TSharedPtr<FJsonValue>>* ErrorMessages = nullptr;
	if ((JsonObject->TryGetArrayField(TEXT("errorMessages"), ErrorMessages) || JsonObject->TryGetArrayField(TEXT("ErrorMessages"), ErrorMessages)) 
		&& ErrorMessages && ErrorMessages->Num() > 0)
	{
		OutError = (*ErrorMessages)[0]->AsString();
	}
	else
	{
		OutError = TEXT("Request failed");
	}
	
	return false;
}

void FGLCApiClient::LoginWithApiKeyAsync(const FString& ApiKey, TFunction<void(bool, FString, FGLCLoginResponse)> Callback)
{
	UE_LOG(LogTemp, Warning, TEXT("[GLC] === LoginWithApiKey ASYNC Started ==="));
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	FString Url = BaseUrl + TEXT("/api/cli/build/login-interactive");
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] URL: %s"), *Url);
	
	// Create request body with camelCase to match Unity
	TSharedPtr<FJsonObject> RequestObject = MakeShareable(new FJsonObject);
	RequestObject->SetStringField(TEXT("apiKey"), ApiKey);
	
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObject.ToSharedRef(), Writer);
	Request->SetContentAsString(RequestBody);
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] Request body: %s"), *RequestBody);
	
	Request->OnProcessRequestComplete().BindLambda([this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		FGLCLoginResponse LoginResponse;
		
		if (!bSuccess || !Response.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[GLC] Login request failed: No response"));
			Callback(false, TEXT("Connection error"), LoginResponse);
			return;
		}
		
		int32 StatusCode = Response->GetResponseCode();
		FString ResponseString = Response->GetContentAsString();
		
		UE_LOG(LogTemp, Log, TEXT("[GLC] Response status: %d"), StatusCode);
		UE_LOG(LogTemp, Log, TEXT("[GLC] Response body: %s"), *ResponseString);
		
		if (StatusCode != 200)
		{
			TSharedPtr<FJsonObject> JsonObject = ParseJsonResponse(ResponseString);
			FString ErrorMsg = FString::Printf(TEXT("HTTP %d"), StatusCode);
			
			if (JsonObject.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* ErrorMessages = nullptr;
				if (JsonObject->TryGetArrayField(TEXT("errorMessages"), ErrorMessages) && ErrorMessages && ErrorMessages->Num() > 0)
				{
					ErrorMsg = (*ErrorMessages)[0]->AsString();
				}
			}
			
			UE_LOG(LogTemp, Error, TEXT("[GLC] Login failed: %s"), *ErrorMsg);
			Callback(false, ErrorMsg, LoginResponse);
			return;
		}
		
		TSharedPtr<FJsonObject> JsonObject = ParseJsonResponse(ResponseString);
		
		if (!JsonObject.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[GLC] Failed to parse JSON response"));
			Callback(false, TEXT("Invalid JSON response"), LoginResponse);
			return;
		}
		
		TSharedPtr<FJsonObject> ResultObject;
		FString ErrorMessage;
		
		if (ExtractApiResult(JsonObject, ResultObject, ErrorMessage))
		{
			// Parse login response (camelCase to match backend)
			ResultObject->TryGetStringField(TEXT("id"), LoginResponse.Id);
			ResultObject->TryGetStringField(TEXT("username"), LoginResponse.Username);
			ResultObject->TryGetStringField(TEXT("email"), LoginResponse.Email);
			ResultObject->TryGetStringField(TEXT("token"), LoginResponse.Token);
			
			if (LoginResponse.Token.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("[GLC] Login response missing token"));
				Callback(false, TEXT("Login response missing token"), LoginResponse);
				return;
			}
			
			// Parse roles
			const TArray<TSharedPtr<FJsonValue>>* RolesArray = nullptr;
			if (ResultObject->TryGetArrayField(TEXT("roles"), RolesArray) && RolesArray)
			{
				for (const TSharedPtr<FJsonValue>& RoleValue : *RolesArray)
				{
					LoginResponse.Roles.Add(RoleValue->AsString());
				}
			}
			
			// Parse subscription plan
			const TSharedPtr<FJsonObject>* SubscriptionObject = nullptr;
			if (ResultObject->TryGetObjectField(TEXT("subscription"), SubscriptionObject) && SubscriptionObject && (*SubscriptionObject).IsValid())
			{
				const TSharedPtr<FJsonObject>* PlanObject = nullptr;
				if ((*SubscriptionObject)->TryGetObjectField(TEXT("plan"), PlanObject) && PlanObject && (*PlanObject).IsValid())
				{
					(*PlanObject)->TryGetStringField(TEXT("name"), LoginResponse.PlanName);
				}
			}
			
			AuthToken = LoginResponse.Token;
			UE_LOG(LogTemp, Log, TEXT("[GLC] Login successful as %s"), *LoginResponse.Email);
			Callback(true, TEXT("Login successful"), LoginResponse);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GLC] Login failed: %s"), *ErrorMessage);
			Callback(false, ErrorMessage, LoginResponse);
		}
	});
	
	Request->ProcessRequest();
}

void FGLCApiClient::GetAppListAsync(TFunction<void(bool, FString, TArray<FGLCAppInfo>)> Callback)
{
	if (AuthToken.IsEmpty())
	{
		Callback(false, TEXT("Not authenticated"), TArray<FGLCAppInfo>());
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] GetAppList started"));
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BaseUrl + TEXT("/api/cli/build/list-apps"));
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + AuthToken);
	
	Request->OnProcessRequestComplete().BindLambda([this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		TArray<FGLCAppInfo> Apps;
		
		if (!bSuccess || !Response.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[GLC] GetAppList request failed: No response"));
			Callback(false, TEXT("Connection error"), Apps);
			return;
		}
		
		FString ResponseString = Response->GetContentAsString();
		TSharedPtr<FJsonObject> JsonObject = ParseJsonResponse(ResponseString);
		TSharedPtr<FJsonObject> ResultObject;
		FString ErrorMessage;
		
		if (ExtractApiResult(JsonObject, ResultObject, ErrorMessage))
		{
			const TArray<TSharedPtr<FJsonValue>>* AppsArray;
			if (ResultObject->TryGetArrayField(TEXT("apps"), AppsArray))
			{
				for (const TSharedPtr<FJsonValue>& AppValue : *AppsArray)
				{
					TSharedPtr<FJsonObject> AppObject = AppValue->AsObject();
					if (AppObject.IsValid())
					{
						FGLCAppInfo AppInfo;
						AppObject->TryGetNumberField(TEXT("id"), AppInfo.Id);
						AppObject->TryGetStringField(TEXT("name"), AppInfo.Name);
						AppObject->TryGetStringField(TEXT("description"), AppInfo.Description);
						AppObject->TryGetNumberField(TEXT("buildCount"), AppInfo.BuildCount);
						AppObject->TryGetBoolField(TEXT("isOwnedByUser"), AppInfo.IsOwnedByUser);
						Apps.Add(AppInfo);
					}
				}
			}
			
			UE_LOG(LogTemp, Log, TEXT("[GLC] Retrieved %d apps successfully"), Apps.Num());
			Callback(true, TEXT("Apps retrieved successfully"), Apps);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GLC] GetAppList failed: %s"), *ErrorMessage);
			Callback(false, ErrorMessage, Apps);
		}
	});
	
	Request->ProcessRequest();
}

void FGLCApiClient::CanUploadAsync(int64 FileSizeBytes, int64 UncompressedSizeBytes, int64 AppId, TFunction<void(bool, FString, FGLCCanUploadResponse)> Callback)
{
	if (AuthToken.IsEmpty())
	{
		Callback(false, TEXT("Not authenticated"), FGLCCanUploadResponse());
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] CanUpload started"));
	
	FString Url = FString::Printf(TEXT("%s/api/cli/build/can-upload?fileSizeBytes=%lld&uncompressedSizeBytes=%lld&appId=%lld"), 
		*BaseUrl, FileSizeBytes, UncompressedSizeBytes, AppId);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + AuthToken);
	
	Request->OnProcessRequestComplete().BindLambda([this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		FGLCCanUploadResponse UploadResponse;
		
		if (!bSuccess || !Response.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[GLC] CanUpload request failed: No response"));
			Callback(false, TEXT("Connection error"), UploadResponse);
			return;
		}
		
		FString ResponseString = Response->GetContentAsString();
		TSharedPtr<FJsonObject> JsonObject = ParseJsonResponse(ResponseString);
		TSharedPtr<FJsonObject> ResultObject;
		FString ErrorMessage;
		
		if (ExtractApiResult(JsonObject, ResultObject, ErrorMessage))
		{
			ResultObject->TryGetBoolField(TEXT("canUpload"), UploadResponse.CanUpload);
			ResultObject->TryGetNumberField(TEXT("fileSizeBytes"), UploadResponse.FileSizeBytes);
			ResultObject->TryGetNumberField(TEXT("uncompressedSizeBytes"), UploadResponse.UncompressedSizeBytes);
			ResultObject->TryGetStringField(TEXT("planName"), UploadResponse.PlanName);
			ResultObject->TryGetNumberField(TEXT("maxCompressedSizeGB"), UploadResponse.MaxCompressedSizeGB);
			ResultObject->TryGetNumberField(TEXT("maxUncompressedSizeGB"), UploadResponse.MaxUncompressedSizeGB);
			
			UE_LOG(LogTemp, Log, TEXT("[GLC] Upload check successful"));
			Callback(true, TEXT("Upload check successful"), UploadResponse);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GLC] CanUpload failed: %s"), *ErrorMessage);
			Callback(false, ErrorMessage, UploadResponse);
		}
	});
	
	Request->ProcessRequest();
}

void FGLCApiClient::StartUploadAsync(int64 AppId, const FString& FileName, int64 FileSize, int64 UncompressedFileSize, const FString& BuildNotes, TFunction<void(bool, FString, FGLCStartUploadResponse)> Callback)
{
	if (AuthToken.IsEmpty())
	{
		Callback(false, TEXT("Not authenticated"), FGLCStartUploadResponse());
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] StartUpload started for file: %s (%lld bytes)"), *FileName, FileSize);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BaseUrl + TEXT("/api/cli/build/start-upload"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + AuthToken);
	
	// Create request body
	TSharedPtr<FJsonObject> RequestObject = MakeShareable(new FJsonObject);
	RequestObject->SetNumberField(TEXT("appId"), AppId);
	RequestObject->SetStringField(TEXT("fileName"), FileName);
	RequestObject->SetNumberField(TEXT("fileSize"), FileSize);
	RequestObject->SetNumberField(TEXT("uncompressedFileSize"), UncompressedFileSize);
	RequestObject->SetStringField(TEXT("buildNotes"), BuildNotes);
	
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObject.ToSharedRef(), Writer);
	Request->SetContentAsString(RequestBody);
	
	Request->OnProcessRequestComplete().BindLambda([this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		FGLCStartUploadResponse UploadResponse;
		
		if (!bSuccess || !Response.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[GLC] StartUpload request failed: No response"));
			Callback(false, TEXT("Connection error"), UploadResponse);
			return;
		}
		
		FString ResponseString = Response->GetContentAsString();
		TSharedPtr<FJsonObject> JsonObject = ParseJsonResponse(ResponseString);
		TSharedPtr<FJsonObject> ResultObject;
		FString ErrorMessage;
		
		if (ExtractApiResult(JsonObject, ResultObject, ErrorMessage))
		{
			ResultObject->TryGetNumberField(TEXT("appBuildId"), UploadResponse.AppBuildId);
			ResultObject->TryGetStringField(TEXT("uploadUrl"), UploadResponse.UploadUrl);
			ResultObject->TryGetStringField(TEXT("key"), UploadResponse.Key);
			ResultObject->TryGetStringField(TEXT("finalUrl"), UploadResponse.FinalUrl);
			
			UE_LOG(LogTemp, Log, TEXT("[GLC] Upload started successfully. Build ID: %lld"), UploadResponse.AppBuildId);
			Callback(true, TEXT("Upload started successfully"), UploadResponse);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GLC] StartUpload failed: %s"), *ErrorMessage);
			Callback(false, ErrorMessage, UploadResponse);
		}
	});
	
	Request->ProcessRequest();
}

void FGLCApiClient::UploadFileAsync(const FString& PresignedUrl, const FString& FilePath, TFunction<void(bool, FString, float)> ProgressCallback)
{
	UE_LOG(LogTemp, Log, TEXT("[GLC] UploadFile started for: %s"), *FilePath);
	
	// Read file
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] Failed to read file: %s"), *FilePath);
		ProgressCallback(false, TEXT("Failed to read file"), 0.0f);
		return;
	}
	
	int64 FileSize = FileData.Num();
	UE_LOG(LogTemp, Log, TEXT("[GLC] File size: %lld bytes (%.2f MB)"), FileSize, FileSize / (1024.0f * 1024.0f));
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	
	// Store the request so it can be cancelled
	ActiveUploadRequest = Request;
	
	Request->SetURL(PresignedUrl);
	Request->SetVerb(TEXT("PUT"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/octet-stream"));
	Request->SetContent(FileData);
	
	// Progress callback
	Request->OnRequestProgress64().BindLambda([ProgressCallback, FileSize](FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived)
	{
		float Progress = (float)BytesSent / (float)FileSize;
		ProgressCallback(false, TEXT("Uploading..."), Progress);
	});
	
	Request->OnProcessRequestComplete().BindLambda([this, ProgressCallback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		// Clear the active request reference
		ActiveUploadRequest.Reset();
		
		// Check if request was cancelled
		EHttpRequestStatus::Type Status = Request->GetStatus();
		if (!bSuccess && (!Response.IsValid() || Status == EHttpRequestStatus::Failed))
		{
			UE_LOG(LogTemp, Warning, TEXT("[GLC] Upload was cancelled or connection failed"));
			ProgressCallback(false, TEXT("Upload cancelled"), -1.0f);
			return;
		}
		
		if (!bSuccess || !Response.IsValid() || Response->GetResponseCode() != 200)
		{
			FString Error = FString::Printf(TEXT("Upload failed: HTTP %d"), Response.IsValid() ? Response->GetResponseCode() : 0);
			UE_LOG(LogTemp, Error, TEXT("[GLC] %s"), *Error);
			ProgressCallback(false, Error, 0.0f);
			return;
		}
		
		UE_LOG(LogTemp, Log, TEXT("[GLC] Upload successful!"));
		ProgressCallback(true, TEXT("Upload completed"), 1.0f);
	});
	
	Request->ProcessRequest();
}

void FGLCApiClient::NotifyFileReadyAsync(int64 AppBuildId, const FString& Key, TFunction<void(bool, FString)> Callback)
{
	if (AuthToken.IsEmpty())
	{
		Callback(false, TEXT("Not authenticated"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] NotifyFileReady started for Build ID: %lld"), AppBuildId);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BaseUrl + TEXT("/api/cli/build/file-ready"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + AuthToken);
	
	// Create request body
	TSharedPtr<FJsonObject> RequestObject = MakeShareable(new FJsonObject);
	RequestObject->SetNumberField(TEXT("appBuildId"), AppBuildId);
	RequestObject->SetStringField(TEXT("key"), Key);
	
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObject.ToSharedRef(), Writer);
	Request->SetContentAsString(RequestBody);
	
	Request->OnProcessRequestComplete().BindLambda([this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		if (!bSuccess || !Response.IsValid() || Response->GetResponseCode() != 200)
		{
			FString Error = FString::Printf(TEXT("Request failed: HTTP %d"), Response.IsValid() ? Response->GetResponseCode() : 0);
			UE_LOG(LogTemp, Error, TEXT("[GLC] %s"), *Error);
			Callback(false, Error);
			return;
		}
		
		UE_LOG(LogTemp, Log, TEXT("[GLC] File ready notification sent successfully!"));
		Callback(true, TEXT("File ready notification sent"));
	});
	
	Request->ProcessRequest();
}

void FGLCApiClient::GetBuildStatusAsync(int64 AppBuildId, TFunction<void(bool, FString, FGLCBuildStatusResponse)> Callback)
{
	if (AuthToken.IsEmpty())
	{
		Callback(false, TEXT("Not authenticated"), FGLCBuildStatusResponse());
		return;
	}
	
	FString Url = FString::Printf(TEXT("%s/api/cli/build/status/%lld"), *BaseUrl, AppBuildId);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + AuthToken);
	
	Request->OnProcessRequestComplete().BindLambda([this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		FGLCBuildStatusResponse StatusResponse;
		
		if (!bSuccess || !Response.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[GLC] GetBuildStatus request failed: No response"));
			Callback(false, TEXT("Connection error"), StatusResponse);
			return;
		}
		
		FString ResponseString = Response->GetContentAsString();
		TSharedPtr<FJsonObject> JsonObject = ParseJsonResponse(ResponseString);
		TSharedPtr<FJsonObject> ResultObject;
		FString ErrorMessage;
		
		if (ExtractApiResult(JsonObject, ResultObject, ErrorMessage))
		{
			ResultObject->TryGetNumberField(TEXT("appBuildId"), StatusResponse.AppBuildId);
			ResultObject->TryGetNumberField(TEXT("appId"), StatusResponse.AppId);
			ResultObject->TryGetStringField(TEXT("status"), StatusResponse.Status);
			ResultObject->TryGetStringField(TEXT("fileName"), StatusResponse.FileName);
			ResultObject->TryGetStringField(TEXT("buildNotes"), StatusResponse.BuildNotes);
			ResultObject->TryGetStringField(TEXT("errorMessage"), StatusResponse.ErrorMessage);
			ResultObject->TryGetNumberField(TEXT("fileSize"), StatusResponse.FileSize);
			ResultObject->TryGetNumberField(TEXT("compressedFileSize"), StatusResponse.CompressedFileSize);
			ResultObject->TryGetNumberField(TEXT("stageProgress"), StatusResponse.StageProgress);
			
			Callback(true, TEXT("Success"), StatusResponse);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GLC] GetBuildStatus failed: %s"), *ErrorMessage);
			Callback(false, ErrorMessage, StatusResponse);
		}
	});
	
	Request->ProcessRequest();
}

void FGLCApiClient::CancelBuildAsync(int64 AppBuildId, TFunction<void(bool, FString)> Callback)
{
	if (AuthToken.IsEmpty())
	{
		Callback(false, TEXT("Not authenticated"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] CancelBuild started for Build ID: %lld"), AppBuildId);
	
	FString Url = FString::Printf(TEXT("%s/api/AppBuild/cancelByBuildId/%lld"), *BaseUrl, AppBuildId);
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + AuthToken);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	Request->OnProcessRequestComplete().BindLambda([this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		if (!bSuccess || !Response.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[GLC] CancelBuild request failed: No response"));
			Callback(false, TEXT("Connection error"));
			return;
		}
		
		int32 ResponseCode = Response->GetResponseCode();
		FString ResponseString = Response->GetContentAsString();
		
		if (ResponseCode == 200)
		{
			UE_LOG(LogTemp, Log, TEXT("[GLC] Build cancelled successfully"));
			Callback(true, TEXT("Build cancelled successfully"));
		}
		else
		{
			FString ErrorMessage = FString::Printf(TEXT("Failed to cancel build: HTTP %d"), ResponseCode);
			
			// Try to extract error message from response
			TSharedPtr<FJsonObject> JsonObject = ParseJsonResponse(ResponseString);
			if (JsonObject.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* ErrorMessages;
				if (JsonObject->TryGetArrayField(TEXT("errorMessages"), ErrorMessages) && ErrorMessages->Num() > 0)
				{
					ErrorMessage = (*ErrorMessages)[0]->AsString();
				}
			}
			
			UE_LOG(LogTemp, Error, TEXT("[GLC] CancelBuild failed: %s"), *ErrorMessage);
			Callback(false, ErrorMessage);
		}
	});
	
	Request->ProcessRequest();
}

void FGLCApiClient::CancelActiveUpload()
{
	if (ActiveUploadRequest.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[GLC] Cancelling active upload request"));
		ActiveUploadRequest->CancelRequest();
		ActiveUploadRequest.Reset();
		UE_LOG(LogTemp, Log, TEXT("[GLC] Active upload request cancelled"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GLC] No active upload request to cancel"));
	}
}
