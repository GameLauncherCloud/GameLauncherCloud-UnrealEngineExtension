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
	int64 GetDirectorySize(const FString& DirectoryPath);
	void StartBuildStatusMonitoring(int64 BuildId);
	void StopBuildStatusMonitoring();
	void CheckBuildStatus();
	FString GetStatusIcon(const FString& Status);
	
	// Path helpers
	FString GetBuildSourcePath() const;
	FString GetZipPath() const;
	
	// ========== STATE ========== //
	TSharedPtr<FGLCApiClient> ApiClient;
	FString ApiUrl;
	FString AuthToken;
	FString UserEmail;
	FString UserPlan;
	FString CurrentEnvironment;
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
	
	// ========== BUILD DETECTION ========== //
	bool bHasBuildReady;
	FDateTime LastBuildDate;
	FString LastBuildPath;
	int64 LastBuildSize;
	int64 UncompressedBuildSize;
	int32 TotalFileCount;
	bool bIsCompressed;
	
	int64 CurrentBuildId;
	FTimerHandle BuildStatusTimerHandle;
	
	// ========== UI WIDGETS ========== //
	TSharedPtr<SVerticalBox> MainContentBox;
	TSharedPtr<SEditableTextBox> ApiKeyTextBox;
	TSharedPtr<SEditableTextBox> BuildNotesTextBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> AppComboBox;
	TSharedPtr<SProgressBar> UploadProgressBar;
	TSharedPtr<STextBlock> StatusMessageText;
	
	// ========== UI REFRESH ========== //
	void RefreshUI();
	
	// ========== BUILD DETECTION ========== //
	void CheckForExistingBuild();
	
	// ========== ACTIONS ========== //
	FReply OnBuildOnlyClicked();
	FReply OnUploadOnlyClicked();
	FReply OnDashboardClicked();
	FReply OnManageAppClicked();
	
	// ========== BUILD METHODS ========== //
	void BuildGame(bool bCompressOnly = false);
	void CompressOnly(const FString& BuildPath);
	void CompressAndUpload(const FString& BuildPath);
	bool CompressBuild(const FString& SourcePath, const FString& ZipPath);
	
	// ========== UPLOAD METHODS ========== //
	void UploadBuildToCloud(const FString& ZipPath);
};
