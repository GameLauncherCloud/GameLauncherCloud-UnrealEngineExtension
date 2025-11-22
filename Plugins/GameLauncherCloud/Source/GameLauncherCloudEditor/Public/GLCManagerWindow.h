// Copyright Game Launcher Cloud. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "GLCApiClient.h"

/// <summary>
/// Main editor window for Game Launcher Cloud
/// Provides UI for authentication and build & upload
/// </summary>
class GAMELAUNCHERCLOUDEDITOR_API SGLCManagerWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGLCManagerWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// ========== UI CONSTRUCTION ========== //
	TSharedRef<SWidget> ConstructLoginTab();
	TSharedRef<SWidget> ConstructBuildUploadTab();
	TSharedRef<SWidget> ConstructTipsTab();
	
	// ========== LOGIN HANDLERS ========== //
	FReply OnLoginWithApiKeyClicked();
	FReply OnLogoutClicked();
	
	// ========== BUILD & UPLOAD HANDLERS ========== //
	FReply OnLoadAppsClicked();
	FReply OnBuildAndUploadClicked();
	void OnAppSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	
	// ========== HELPER FUNCTIONS ========== //
	void SaveConfig();
	void LoadConfig();
	FString GetBuildDirectory() const;
	void PackageProject(const FString& OutputDirectory);
	void CompressBuild(const FString& BuildDirectory, const FString& ZipPath);
	int64 GetDirectorySize(const FString& DirectoryPath);
	void StartBuildStatusMonitoring(int64 BuildId);
	void StopBuildStatusMonitoring();
	
	// ========== STATE ========== //
	TSharedPtr<FGLCApiClient> ApiClient;
	FString ApiUrl;
	FString AuthToken;
	FString UserEmail;
	bool bIsAuthenticated;
	
	// ========== UI STATE ========== //
	FString ApiKeyInput;
	FString BuildNotesInput;
	TArray<FGLCAppInfo> AvailableApps;
	TArray<TSharedPtr<FString>> AppNames;
	TSharedPtr<FString> SelectedApp;
	int32 SelectedAppIndex;
	
	bool bIsLoggingIn;
	bool bIsLoadingApps;
	bool bIsBuilding;
	bool bIsUploading;
	bool bIsMonitoringBuild;
	
	FString StatusMessage;
	FString StatusMessageType; // "Info", "Warning", "Error", "Success"
	float UploadProgress;
	
	int64 CurrentBuildId;
	FTimerHandle BuildStatusTimerHandle;
	
	// ========== UI WIDGETS ========== //
	TSharedPtr<SEditableTextBox> ApiKeyTextBox;
	TSharedPtr<SEditableTextBox> BuildNotesTextBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> AppComboBox;
	TSharedPtr<SProgressBar> UploadProgressBar;
	TSharedPtr<STextBlock> StatusMessageText;
};
