// Copyright Game Launcher Cloud. All Rights Reserved.

#include "GLCManagerWindow.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Images/SImage.h"
#include "EditorStyleSet.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"
#include "Editor.h"
#include "HAL/PlatformProcess.h"
#include "Async/Async.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "GLCManagerWindow"

void SGLCManagerWindow::Construct(const FArguments& InArgs)
{
	bIsAuthenticated = false;
	bIsLoggingIn = false;
	bIsLoadingApps = false;
	bIsBuilding = false;
	bIsUploading = false;
	bIsMonitoringBuild = false;
	UploadProgress = 0.0f;
	SelectedAppIndex = 0;
	CurrentBuildId = 0;
	CurrentEnvironment = TEXT("Production");
	
	// Initialize build detection
	bHasBuildReady = false;
	LastBuildSize = 0;
	UncompressedBuildSize = 0;
	TotalFileCount = 0;
	bIsCompressed = false;
	
	ApiUrl = TEXT("https://api.gamelauncher.cloud");
	
	LoadConfig();
	
	ApiClient = MakeShareable(new FGLCApiClient(ApiUrl, AuthToken));
	
	// Auto-load apps and check builds if already authenticated
	if (bIsAuthenticated && !AuthToken.IsEmpty())
	{
		CheckForExistingBuild();
		OnLoadAppsClicked();
	}
	
	ChildSlot
	[
		SNew(SVerticalBox)
		
		// Header with epic gradient background
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.15f, 0.25f, 0.45f, 0.95f))
			.Padding(20.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 15.0f, 0.0f)
				[
					SNew(SImage)
					.Image(FSlateIcon("GameLauncherCloudStyle", "GameLauncherCloud.Icon").GetIcon())
					.DesiredSizeOverride(FVector2D(52.0f, 52.0f))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Title", "Game Launcher Cloud"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 26))
						.ColorAndOpacity(FLinearColor(0.9f, 0.95f, 1.0f))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Subtitle", "Build and Upload Manager for Unreal Engine"))
						.Font(FCoreStyle::GetDefaultFontStyle("Italic", 12))
						.ColorAndOpacity(FLinearColor(0.7f, 0.85f, 1.0f, 0.9f))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() { 
							return bIsAuthenticated ? 
								FText::Format(LOCTEXT("Connected", "✓ {0}"), FText::FromString(UserEmail)) : 
								LOCTEXT("NotConnected", "Not connected");
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						.ColorAndOpacity_Lambda([this]() { 
							return bIsAuthenticated ? 
								FLinearColor(0.4f, 1.0f, 0.4f) : 
								FLinearColor(1.0f, 0.7f, 0.4f);
						})
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.Padding(0.0f, 2.0f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() {
							return bIsAuthenticated && !UserPlan.IsEmpty() ?
								FText::Format(LOCTEXT("PlanLabel", "Plan: {0}"), FText::FromString(UserPlan)) :
								FText::GetEmpty();
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.0f, 0.8f))
						.Visibility_Lambda([this]() {
							return (bIsAuthenticated && !UserPlan.IsEmpty()) ? EVisibility::Visible : EVisibility::Collapsed;
						})
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.Padding(0.0f, 5.0f, 0.0f, 0.0f)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "FlatButton.Danger")
						.ForegroundColor(FLinearColor::White)
						.ContentPadding(FMargin(12.0f, 6.0f))
						.OnClicked(this, &SGLCManagerWindow::OnLogoutClicked)
						.Visibility_Lambda([this]() { return bIsAuthenticated ? EVisibility::Visible : EVisibility::Collapsed; })
						[
							SNew(STextBlock)
							.Text(LOCTEXT("LogoutButton", "Logout"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						]
					]
				]
			]
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f, 0.0f)
		[
			SNew(SSeparator)
		]
		
		// Content
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(20.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(MainContentBox, SVerticalBox)
				
				// Show login or build/upload based on auth state
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBorder)
					.Padding(20.0f)
					[
						bIsAuthenticated ? ConstructBuildUploadTab() : ConstructLoginTab()
					]
				]
				
				// Tips section
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 20.0f, 0.0f, 0.0f)
				[
					ConstructTipsTab()
				]
			]
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f, 0.0f)
		[
			SNew(SSeparator)
		]
		
		// Environment label at bottom
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() {
				return FText::Format(LOCTEXT("EnvironmentLabel", "Environment: {0}"), FText::FromString(CurrentEnvironment));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
			.ColorAndOpacity(FLinearColor(0.5f, 0.6f, 0.7f, 0.7f))
		]
	];
}

void SGLCManagerWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

TSharedRef<SWidget> SGLCManagerWindow::ConstructLoginTab()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.2f, 0.9f))
		.Padding(30.0f)
		[
			SNew(SVerticalBox)
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LoginTitle", "🔐 Login to Game Launcher Cloud"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
				.ColorAndOpacity(FLinearColor(0.9f, 0.95f, 1.0f))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 20.0f, 0.0f, 10.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ApiKeyLabel", "API Key"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
				.ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.0f))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(FMargin(10.0f, 8.0f))
				[
					SAssignNew(ApiKeyTextBox, SEditableTextBox)
					.HintText(LOCTEXT("ApiKeyHint", "glc_xxxxxxxxxxxxx"))
					.IsPassword(true)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.OnTextChanged_Lambda([this](const FText& NewText) { ApiKeyInput = NewText.ToString(); })
				]
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 20.0f)
			.HAlign(HAlign_Center)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
				.ForegroundColor(FLinearColor::White)
				.ContentPadding(FMargin(40.0f, 12.0f))
				.OnClicked(this, &SGLCManagerWindow::OnLoginWithApiKeyClicked)
				.IsEnabled_Lambda([this]() { return !bIsLoggingIn && !ApiKeyInput.IsEmpty(); })
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { 
						return bIsLoggingIn ? 
							LOCTEXT("Logging", "⏳ Logging in...") : 
							LOCTEXT("LoginButton", "Login with API Key");
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				]
			]
		
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 15.0f)
			.HAlign(HAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor_Lambda([this]() {
					return StatusMessageType == TEXT("Error") ? 
						FLinearColor(0.8f, 0.2f, 0.2f, 0.8f) : 
						FLinearColor(0.2f, 0.6f, 0.3f, 0.8f);
				})
				.Padding(FMargin(15.0f, 10.0f))
				.Visibility_Lambda([this]() { return StatusMessage.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
				[
					SAssignNew(StatusMessageText, STextBlock)
					.Text(FText::FromString(StatusMessage))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.ColorAndOpacity(FLinearColor::White)
				]
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 20.0f, 0.0f, 0.0f)
			.HAlign(HAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GetApiKeyLabel", "Get your API Key from:"))
					.Font(FCoreStyle::GetDefaultFontStyle("Italic", 11))
					.ColorAndOpacity(FLinearColor(0.7f, 0.8f, 0.9f, 0.8f))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 5.0f)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.OnClicked_Lambda([]() {
						FPlatformProcess::LaunchURL(TEXT("https://api.gamelauncher.cloud/user/api-keys"), nullptr, nullptr);
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ApiKeyLink", "https://api.gamelauncher.cloud/user/api-keys"))
						.Font(FCoreStyle::GetDefaultFontStyle("Italic", 11))
						.ColorAndOpacity(FLinearColor(0.4f, 0.8f, 1.0f))
					]
				]
			]
		];
}

TSharedRef<SWidget> SGLCManagerWindow::ConstructBuildUploadTab()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.2f, 0.9f))
		.Padding(30.0f)
		[
			SNew(SVerticalBox)
			
			// Welcome header with logout
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.2f, 0.6f, 0.9f, 0.3f))
				.Padding(FMargin(20.0f, 15.0f))
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("WelcomeMessage", "👋 Welcome, {0}!"), FText::FromString(UserEmail)))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
					.ColorAndOpacity(FLinearColor(0.9f, 0.95f, 1.0f))
				]
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 25.0f, 0.0f, 15.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BuildUploadTitle", "🚀 Build & Deploy"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
				.ColorAndOpacity(FLinearColor(0.9f, 0.95f, 1.0f))
			]
		
			
			// App selection section
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(FMargin(20.0f, 15.0f))
				[
					SNew(SVerticalBox)
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("SelectAppLabel", "📱 Select Application"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
							.ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.0f))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(5.0f, 0.0f, 0.0f, 0.0f)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "FlatButton.Info")
							.ForegroundColor(FLinearColor::White)
							.ContentPadding(FMargin(15.0f, 8.0f))
							.OnClicked(this, &SGLCManagerWindow::OnLoadAppsClicked)
							.IsEnabled_Lambda([this]() { return !bIsLoadingApps; })
							[
								SNew(STextBlock)
								.Text_Lambda([this]() {
									return bIsLoadingApps ? 
										LOCTEXT("LoadingApps", "⏳ Loading...") : 
										LOCTEXT("LoadAppsButton", "🔄 Reload Apps");
								})
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(5.0f, 0.0f, 0.0f, 0.0f)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
							.ForegroundColor(FLinearColor(0.9f, 0.95f, 1.0f))
							.ContentPadding(FMargin(15.0f, 8.0f))
							.OnClicked(this, &SGLCManagerWindow::OnManageAppClicked)
							.IsEnabled_Lambda([this]() { return AvailableApps.Num() > 0 && SelectedAppIndex >= 0; })
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ManageAppButton", "⚙️ Manage App"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
							]
						]
					]
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 10.0f, 0.0f, 0.0f)
					[
						SAssignNew(AppComboBox, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&AppNames)
						.OnSelectionChanged(this, &SGLCManagerWindow::OnAppSelected)
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
						{
							return SNew(STextBlock)
								.Text(FText::FromString(*Item))
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12));
						})
						[
							SNew(STextBlock)
							.Text_Lambda([this]()
							{
								return SelectedApp.IsValid() ? FText::FromString(*SelectedApp) : LOCTEXT("SelectApp", "Select an app...");
							})
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
						]
					]
				]
			]
		
			
			// Open Dashboard button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Info")
				.ForegroundColor(FLinearColor::White)
				.ContentPadding(FMargin(20.0f, 10.0f))
				.HAlign(HAlign_Center)
				.OnClicked(this, &SGLCManagerWindow::OnDashboardClicked)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DashboardButton", "📊 Open Dashboard"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
				]
			]
			
			// Build detection card
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 15.0f, 0.0f, 0.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.15f, 0.3f, 0.2f, 0.85f))
				.Padding(FMargin(20.0f, 15.0f))
				.Visibility_Lambda([this]() { return bHasBuildReady ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(SVerticalBox)
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("BuildReadyTitle", "✓ Build Ready"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor(0.2f, 0.9f, 0.3f))
					]
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 8.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() {
							return FText::Format(LOCTEXT("LastBuildDate", "Last build: {0}"),
								FText::FromString(LastBuildDate.ToString(TEXT("%Y-%m-%d %H:%M:%S"))));
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.0f))
					]
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 5.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() {
							if (bIsCompressed && UncompressedBuildSize > 0)
							{
								float compressionRatio = (1.0f - (LastBuildSize / (float)UncompressedBuildSize)) * 100.0f;
								return FText::Format(LOCTEXT("BuildInfoCompressed", "Files: {0} | Uncompressed: {1} MB | Compressed: {2} MB ({3}% saved)"),
									FText::AsNumber(TotalFileCount),
									FText::AsNumber((int32)(UncompressedBuildSize / (1024.0 * 1024.0))),
									FText::AsNumber((int32)(LastBuildSize / (1024.0 * 1024.0))),
									FText::AsNumber((int32)compressionRatio));
							}
							else if (bIsCompressed)
							{
								return FText::Format(LOCTEXT("BuildInfoCompressedOnly", "Compressed size: {0} MB"),
									FText::AsNumber((int32)(LastBuildSize / (1024.0 * 1024.0))));
							}
							else
							{
								return FText::Format(LOCTEXT("BuildInfoUncompressed", "Files: {0} | Size: {1} MB (Not compressed)"),
									FText::AsNumber(TotalFileCount),
									FText::AsNumber((int32)(LastBuildSize / (1024.0 * 1024.0))));
							}
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.0f))
					]
					
					// Show in Explorer button (only if compressed)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 10.0f, 0.0f, 0.0f)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
						.ForegroundColor(FLinearColor(0.9f, 0.95f, 1.0f))
						.ContentPadding(FMargin(12.0f, 6.0f))
						.HAlign(HAlign_Left)
						.OnClicked_Lambda([this]() -> FReply {
							FString ZipPath = GetZipPath();
							if (FPaths::FileExists(ZipPath))
							{
								FPlatformProcess::ExploreFolder(*FPaths::GetPath(ZipPath));
							}
							return FReply::Handled();
						})
						.Visibility_Lambda([this]() { 
							return (bIsCompressed && FPaths::FileExists(GetZipPath())) ? EVisibility::Visible : EVisibility::Collapsed; 
						})
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ShowInExplorer", "📁 Show in Explorer"))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						]
					]
				]
			]
			
			// Build notes section
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 15.0f, 0.0f, 0.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(FMargin(20.0f, 15.0f))
				[
					SNew(SVerticalBox)
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("BuildNotesLabel", "📝 Build Notes (optional)"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
						.ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.0f))
					]
					
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(0.0f, 8.0f, 0.0f, 0.0f)
					[
						SNew(SBox)
						.HeightOverride(100.0f)
						[
							SAssignNew(BuildNotesTextBox, SEditableTextBox)
							.HintText(LOCTEXT("BuildNotesHint", "What's new in this build?"))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
							.OnTextChanged_Lambda([this](const FText& NewText) { BuildNotesInput = NewText.ToString(); })
						]
					]
				]
			]
			
			// Build & Upload buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 20.0f)
			.HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)
				
				// Build button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5.0f, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
					.ForegroundColor(FLinearColor::White)
					.ContentPadding(FMargin(40.0f, 15.0f))
					.OnClicked(this, &SGLCManagerWindow::OnBuildOnlyClicked)
					.IsEnabled_Lambda([this]() { return !bIsBuilding && !bIsUploading && AvailableApps.Num() > 0 && SelectedAppIndex >= 0; })
					[
						SNew(STextBlock)
						.Text_Lambda([this]() {
							return bIsBuilding ? LOCTEXT("Building", "🔨 Building...") : LOCTEXT("BuildButton", "🔨 Build");
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
					]
				]
				
				// Upload button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5.0f, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Info")
					.ForegroundColor(FLinearColor::White)
					.ContentPadding(FMargin(40.0f, 15.0f))
					.OnClicked(this, &SGLCManagerWindow::OnUploadOnlyClicked)
					.IsEnabled_Lambda([this]() { return !bIsBuilding && !bIsUploading && bHasBuildReady && AvailableApps.Num() > 0 && SelectedAppIndex >= 0; })
					[
						SNew(STextBlock)
						.Text_Lambda([this]() {
							return bIsUploading ? LOCTEXT("Uploading", "☁️ Uploading...") : LOCTEXT("UploadButton", "☁️ Upload to Cloud");
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
					]
				]
			]
		
			
			// Progress bar
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 15.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(FMargin(5.0f))
				.Visibility_Lambda([this]() { return (bIsBuilding || bIsUploading) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SAssignNew(UploadProgressBar, SProgressBar)
					.Percent_Lambda([this]() { return UploadProgress; })
					.FillColorAndOpacity(FLinearColor(0.2f, 0.7f, 1.0f))
				]
			]
			
			// Loading apps message (only when loading apps)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f)
			.HAlign(HAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.3f, 0.6f, 0.9f, 0.8f))
				.Padding(FMargin(20.0f, 12.0f))
				.Visibility_Lambda([this]() { return bIsLoadingApps ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LoadingApps", "Loading apps..."))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.ColorAndOpacity(FLinearColor::White)
					.Justification(ETextJustify::Center)
				]
			]
			
			// Status message (for build/upload operations)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f)
			.HAlign(HAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor_Lambda([this]() {
					return StatusMessageType == TEXT("Error") ? 
						FLinearColor(0.8f, 0.2f, 0.2f, 0.8f) : 
						StatusMessageType == TEXT("Success") ?
						FLinearColor(0.2f, 0.7f, 0.3f, 0.8f) :
						FLinearColor(0.3f, 0.6f, 0.9f, 0.8f);
				})
				.Padding(FMargin(20.0f, 12.0f))
				.Visibility_Lambda([this]() { 
					return !StatusMessage.IsEmpty() && !bIsLoadingApps ? EVisibility::Visible : EVisibility::Collapsed; 
				})
				[
					SAssignNew(StatusMessageText, STextBlock)
					.Text(FText::FromString(StatusMessage))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.ColorAndOpacity(FLinearColor::White)
					.Justification(ETextJustify::Center)
				]
			]
		];
}

TSharedRef<SWidget> SGLCManagerWindow::ConstructTipsTab()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.25f, 0.15f, 0.35f, 0.3f))
		.Padding(25.0f)
		[
			SNew(SVerticalBox)
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TipsTitle", "💡 Tips for Better Builds"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
				.ColorAndOpacity(FLinearColor(0.9f, 0.85f, 1.0f))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 15.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(FMargin(20.0f, 15.0f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Tips", 
						"✓ Test your build locally before uploading\n"
						"✓ Write descriptive build notes\n"
						"✓ Optimize textures and assets to reduce build size\n"
						"✓ Use version control (Git) for your project\n"
						"✓ Check the Game Launcher Cloud dashboard for build status"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.0f, 0.9f))
					.AutoWrapText(true)
				]
			]
		];
}

FReply SGLCManagerWindow::OnLoginWithApiKeyClicked()
{
	if (ApiKeyInput.IsEmpty())
	{
		StatusMessage = TEXT("Please enter an API Key");
		StatusMessageType = TEXT("Error");
		if (StatusMessageText.IsValid())
		{
			StatusMessageText->SetText(FText::FromString(StatusMessage));
		}
		return FReply::Handled();
	}
	
	bIsLoggingIn = true;
	StatusMessage = TEXT("Logging in...");
	StatusMessageType = TEXT("Info");
	
	if (StatusMessageText.IsValid())
	{
		StatusMessageText->SetText(FText::FromString(StatusMessage));
	}
	
	ApiClient->LoginWithApiKeyAsync(ApiKeyInput, [this](bool bSuccess, FString Message, FGLCLoginResponse Response)
	{
		bIsLoggingIn = false;
		
		if (bSuccess)
		{
			bIsAuthenticated = true;
			AuthToken = Response.Token;
			UserEmail = Response.Email;
			// Extract plan name
			if (!Response.PlanName.IsEmpty())
			{
				UserPlan = Response.PlanName;
			}
			else
			{
				UserPlan = TEXT("Free");
			}
			ApiClient->SetAuthToken(AuthToken);
			
			// Save API key to config (before SaveConfig to ensure it's preserved)
			if (!ApiKeyInput.IsEmpty())
			{
				// Load existing config
				FString ConfigPath = FPaths::ProjectPluginsDir() / TEXT("GameLauncherCloud/Config/glc_config.json");
				FString ExistingContent;
				TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
				
				if (FFileHelper::LoadFileToString(ExistingContent, *ConfigPath))
				{
					TSharedPtr<FJsonObject> ExistingJson;
					TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ExistingContent);
					if (FJsonSerializer::Deserialize(Reader, ExistingJson) && ExistingJson.IsValid())
					{
						JsonObject = ExistingJson;
					}
				}
				
				// Save API key for Production environment
				JsonObject->SetStringField(TEXT("apiKeyProduction"), ApiKeyInput);
				
				FString OutputString;
				TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
				FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
				FFileHelper::SaveStringToFile(OutputString, *ConfigPath);
			}
			
			SaveConfig();
			
			StatusMessage = TEXT("Login successful!");
			StatusMessageType = TEXT("Success");
			
			// Refresh UI to show build/upload interface
			RefreshUI();
			
			// Auto-load apps after successful login
			OnLoadAppsClicked();
		}
		else
		{
			StatusMessage = Message;
			StatusMessageType = TEXT("Error");
			
			if (StatusMessageText.IsValid())
			{
				StatusMessageText->SetText(FText::FromString(StatusMessage));
			}
		}
	});
	
	return FReply::Handled();
}

FReply SGLCManagerWindow::OnLogoutClicked()
{
	bIsAuthenticated = false;
	AuthToken.Empty();
	UserEmail.Empty();
	UserPlan.Empty();
	// Keep ApiKeyInput - don't clear it so it shows in login screen
	AvailableApps.Empty();
	AppNames.Empty();
	SaveConfig();
	
	StatusMessage.Empty();
	
	// Refresh UI to show login interface
	RefreshUI();
	
	return FReply::Handled();
}

FReply SGLCManagerWindow::OnLoadAppsClicked()
{
	bIsLoadingApps = true;
	StatusMessage = TEXT("Loading apps...");
	StatusMessageType = TEXT("Info");
	
	ApiClient->GetAppListAsync([this](bool bSuccess, FString Message, TArray<FGLCAppInfo> Apps)
	{
		bIsLoadingApps = false;
		
		if (bSuccess)
		{
			AvailableApps = Apps;
			AppNames.Empty();
			
			for (const FGLCAppInfo& App : Apps)
			{
				AppNames.Add(MakeShareable(new FString(App.Name)));
			}
			
			if (Apps.Num() > 0)
			{
				SelectedApp = AppNames[0];
				SelectedAppIndex = 0;
				StatusMessage = FString::Printf(TEXT("Loaded %d apps successfully"), Apps.Num());
				StatusMessageType = TEXT("Success");
			}
			else
			{
				StatusMessage = TEXT("No apps found. Create an app in the Game Launcher Cloud dashboard first.");
				StatusMessageType = TEXT("Warning");
			}
			
			if (AppComboBox.IsValid())
			{
				AppComboBox->RefreshOptions();
			}
		}
		else
		{
			StatusMessage = Message;
			StatusMessageType = TEXT("Error");
		}
	});
	
	return FReply::Handled();
}

FReply SGLCManagerWindow::OnBuildAndUploadClicked()
{
	if (AvailableApps.Num() == 0 || SelectedAppIndex < 0 || SelectedAppIndex >= AvailableApps.Num())
	{
		StatusMessage = TEXT("Please select an app first");
		StatusMessageType = TEXT("Error");
		return FReply::Handled();
	}
	
	// This is a simplified version - full implementation would:
	// 1. Package the Unreal project using UAT (Unreal Automation Tool)
	// 2. Compress the build into a ZIP
	// 3. Upload to Game Launcher Cloud
	
	StatusMessage = TEXT("Build & Upload feature requires full UAT integration.\n")
		TEXT("This is a demonstration version. Full implementation coming soon!");
	StatusMessageType = TEXT("Info");
	
	return FReply::Handled();
}

void SGLCManagerWindow::OnAppSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedApp = NewSelection;
	
	for (int32 i = 0; i < AppNames.Num(); i++)
	{
		if (AppNames[i] == NewSelection)
		{
			SelectedAppIndex = i;
			break;
		}
	}
}

void SGLCManagerWindow::SaveConfig()
{
	FString ConfigPath = FPaths::ProjectPluginsDir() / TEXT("GameLauncherCloud/Config/glc_config.json");
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	// Load existing config to preserve API keys
	FString ExistingContent;
	if (FFileHelper::LoadFileToString(ExistingContent, *ConfigPath))
	{
		TSharedPtr<FJsonObject> ExistingJson;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ExistingContent);
		if (FJsonSerializer::Deserialize(Reader, ExistingJson) && ExistingJson.IsValid())
		{
			// Preserve API keys from existing config
			FString ApiKeyProduction;
			ExistingJson->TryGetStringField(TEXT("apiKeyProduction"), ApiKeyProduction);
			if (!ApiKeyProduction.IsEmpty())
			{
				JsonObject->SetStringField(TEXT("apiKeyProduction"), ApiKeyProduction);
			}
		}
	}
	
	JsonObject->SetStringField(TEXT("authToken"), AuthToken);
	JsonObject->SetStringField(TEXT("userEmail"), UserEmail);
	JsonObject->SetStringField(TEXT("userPlan"), UserPlan);
	JsonObject->SetStringField(TEXT("apiUrl"), ApiUrl);
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	FFileHelper::SaveStringToFile(OutputString, *ConfigPath);
}

void SGLCManagerWindow::LoadConfig()
{
	FString ConfigPath = FPaths::ProjectPluginsDir() / TEXT("GameLauncherCloud/Config/glc_config.json");
	
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *ConfigPath))
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("authToken"), AuthToken);
			JsonObject->TryGetStringField(TEXT("userEmail"), UserEmail);
			JsonObject->TryGetStringField(TEXT("userPlan"), UserPlan);
			JsonObject->TryGetStringField(TEXT("apiUrl"), ApiUrl);
			
			// Load API Key for current environment (Production)
			FString ApiKeyProduction;
			JsonObject->TryGetStringField(TEXT("apiKeyProduction"), ApiKeyProduction);
			if (!ApiKeyProduction.IsEmpty())
			{
				ApiKeyInput = ApiKeyProduction;
			}
			
			if (!AuthToken.IsEmpty())
			{
				bIsAuthenticated = true;
			}
		}
	}
}

FString SGLCManagerWindow::GetBuildDirectory() const
{
	return FPaths::ProjectDir() / TEXT("Builds");
}

int64 SGLCManagerWindow::GetDirectorySize(const FString& DirectoryPath)
{
	int64 TotalSize = 0;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	PlatformFile.IterateDirectoryRecursively(*DirectoryPath, [&TotalSize](const TCHAR* FilenameOrDirectory, bool bIsDirectory) -> bool
	{
		if (!bIsDirectory)
		{
			TotalSize += IFileManager::Get().FileSize(FilenameOrDirectory);
		}
		return true;
	});
	
	return TotalSize;
}

void SGLCManagerWindow::RefreshUI()
{
	if (!MainContentBox.IsValid())
	{
		return;
	}
	
	// Clear existing content
	MainContentBox->ClearChildren();
	
	// Check for existing build before refreshing
	CheckForExistingBuild();
	
	// Add new content based on authentication state
	MainContentBox->AddSlot()
	.AutoHeight()
	[
		SNew(SBorder)
		.Padding(20.0f)
		[
			bIsAuthenticated ? ConstructBuildUploadTab() : ConstructLoginTab()
		]
	];
	
	// Add tips section
	MainContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 20.0f, 0.0f, 0.0f)
	[
		ConstructTipsTab()
	];
}

FString SGLCManagerWindow::GetBuildSourcePath() const
{
	FString BaseUploadPath = FPaths::ProjectDir() / TEXT("Builds/GLC_Upload");
	FString WindowsPath = BaseUploadPath / TEXT("Windows");
	FString MacPath = BaseUploadPath / TEXT("Mac");
	FString LinuxPath = BaseUploadPath / TEXT("Linux");
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	if (PlatformFile.DirectoryExists(*WindowsPath))
	{
		return WindowsPath;
	}
	else if (PlatformFile.DirectoryExists(*MacPath))
	{
		return MacPath;
	}
	else if (PlatformFile.DirectoryExists(*LinuxPath))
	{
		return LinuxPath;
	}
	
	return BaseUploadPath;
}

FString SGLCManagerWindow::GetZipPath() const
{
	return FPaths::ProjectDir() / TEXT("Builds") / FString::Printf(TEXT("%s_upload.zip"), FApp::GetProjectName());
}

void SGLCManagerWindow::CheckForExistingBuild()
{
	FString ZipPath = GetZipPath();
	FString BuildPath = GetBuildSourcePath();
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	// Check if compressed build exists
	if (PlatformFile.FileExists(*ZipPath))
	{
		bHasBuildReady = true;
		LastBuildDate = PlatformFile.GetTimeStamp(*ZipPath);
		LastBuildPath = ZipPath;
		LastBuildSize = PlatformFile.FileSize(*ZipPath);
		bIsCompressed = true;
		
		// Get uncompressed size and file count from build directory if it exists
		if (!BuildPath.IsEmpty() && PlatformFile.DirectoryExists(*BuildPath))
		{
			TArray<FString> Files;
			PlatformFile.FindFilesRecursively(Files, *BuildPath, nullptr);
			TotalFileCount = Files.Num();
			
			UncompressedBuildSize = 0;
			for (const FString& File : Files)
			{
				UncompressedBuildSize += PlatformFile.FileSize(*File);
			}
		}
		else
		{
			TotalFileCount = 0;
			UncompressedBuildSize = 0;
		}
	}
	// Check if uncompressed build exists
	else if (!BuildPath.IsEmpty() && PlatformFile.DirectoryExists(*BuildPath))
	{
		bHasBuildReady = true;
		LastBuildDate = PlatformFile.GetTimeStamp(*BuildPath);
		LastBuildPath = BuildPath;
		bIsCompressed = false;
		
		// Calculate directory size and file count
		TArray<FString> Files;
		PlatformFile.FindFilesRecursively(Files, *BuildPath, nullptr);
		TotalFileCount = Files.Num();
		
		LastBuildSize = 0;
		for (const FString& File : Files)
		{
			LastBuildSize += PlatformFile.FileSize(*File);
		}
		UncompressedBuildSize = LastBuildSize;
	}
	else
	{
		bHasBuildReady = false;
	}
}

FReply SGLCManagerWindow::OnBuildOnlyClicked()
{
	if (AvailableApps.Num() == 0 || SelectedAppIndex < 0)
	{
		StatusMessage = TEXT("Please select a valid app");
		StatusMessageType = TEXT("Error");
		return FReply::Handled();
	}
	
	bIsBuilding = true;
	StatusMessage = TEXT("Starting build process...");
	StatusMessageType = TEXT("Info");
	UploadProgress = 0.0f;
	
	// Start build in a separate thread
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
	{
		BuildGame(true); // compressOnly = true
	});
	
	return FReply::Handled();
}

FReply SGLCManagerWindow::OnUploadOnlyClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[GLC] OnUploadOnlyClicked called"));
	
	if (AvailableApps.Num() == 0 || SelectedAppIndex < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] No apps available or invalid selection"));
		StatusMessage = TEXT("Please select a valid app");
		StatusMessageType = TEXT("Error");
		return FReply::Handled();
	}
	
	if (!bHasBuildReady)
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] No build ready"));
		StatusMessage = TEXT("No build found. Please package your project first (File > Package Project) and place it in Builds/GLC_Upload/ folder.");
		StatusMessageType = TEXT("Error");
		return FReply::Handled();
	}
	
	// Get paths using centralized helpers
	FString ZipPath = GetZipPath();
	FString BuildPath = GetBuildSourcePath();
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] Checking for compressed build at: %s"), *ZipPath);
	UE_LOG(LogTemp, Log, TEXT("[GLC] Build source path: %s"), *BuildPath);
	
	if (!FPaths::FileExists(ZipPath))
	{
		// Need to compress first
		StatusMessage = TEXT("Compressing build before upload...");
		StatusMessageType = TEXT("Info");
		bIsUploading = true;
		UploadProgress = 0.0f;
		
		UE_LOG(LogTemp, Log, TEXT("[GLC] Starting compression from %s to %s"), *BuildPath, *ZipPath);
		
		// Compress in background thread and WAIT for completion
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, BuildPath, ZipPath]()
		{
			bool bSuccess = CompressBuild(BuildPath, ZipPath);
			
			// Return to main thread after compression completes
			AsyncTask(ENamedThreads::GameThread, [this, bSuccess, ZipPath]()
			{
				if (bSuccess)
				{
					UE_LOG(LogTemp, Log, TEXT("[GLC] Compression successful, starting upload"));
					// Now the ZIP exists, start upload
					UploadBuildToCloud(ZipPath);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[GLC] Compression failed"));
					StatusMessage = TEXT("Failed to compress build");
					StatusMessageType = TEXT("Error");
					bIsUploading = false;
					UploadProgress = 0.0f;
				}
			});
		});
	}
	else
	{
		// Already compressed, start upload directly
		UE_LOG(LogTemp, Log, TEXT("[GLC] Build already compressed, starting upload"));
		bIsUploading = true;
		StatusMessage = TEXT("Starting upload...");
		StatusMessageType = TEXT("Info");
		UploadProgress = 0.0f;
		
		UploadBuildToCloud(ZipPath);
	}
	
	return FReply::Handled();
}

FReply SGLCManagerWindow::OnDashboardClicked()
{
	FString DashboardUrl = TEXT("https://app.gamelauncher.cloud/dashboard");
	FPlatformProcess::LaunchURL(*DashboardUrl, nullptr, nullptr);
	return FReply::Handled();
}

FReply SGLCManagerWindow::OnManageAppClicked()
{
	if (AvailableApps.Num() == 0 || SelectedAppIndex < 0 || SelectedAppIndex >= AvailableApps.Num())
	{
		StatusMessage = TEXT("Please select a valid app");
		StatusMessageType = TEXT("Error");
		return FReply::Handled();
	}
	
	int64 AppId = AvailableApps[SelectedAppIndex].Id;
	FString ManageUrl = FString::Printf(TEXT("https://app.gamelauncher.cloud/apps/id/%lld/overview"), AppId);
	FPlatformProcess::LaunchURL(*ManageUrl, nullptr, nullptr);
	return FReply::Handled();
}

void SGLCManagerWindow::BuildGame(bool bCompressOnly)
{
	FString BuildPath = FPaths::ProjectDir() / TEXT("Builds") / TEXT("GLC_Upload");
	FString ProjectFile = FPaths::GetProjectFilePath();
	
	// Get platform name
	FString Platform = TEXT("Win64");
	
#if PLATFORM_MAC
	Platform = TEXT("Mac");
#elif PLATFORM_LINUX
	Platform = TEXT("Linux");
#endif
	
	// Get Unreal Engine path
	FString EnginePath = FPaths::EngineDir();
	FString UATPath = EnginePath / TEXT("Build/BatchFiles/RunUAT.bat");
	
#if PLATFORM_MAC || PLATFORM_LINUX
	UATPath = EnginePath / TEXT("Build/BatchFiles/RunUAT.sh");
#endif
	
	// Update status on main thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		StatusMessage = TEXT("Building project... This may take several minutes.");
		StatusMessageType = TEXT("Info");
		UploadProgress = 0.1f;
	});
	
	// Build command arguments
	FString Arguments = FString::Printf(
		TEXT("BuildCookRun -project=\"%s\" -platform=%s -clientconfig=Development -cook -stage -archive -archivedirectory=\"%s\" -build -noP4"),
		*ProjectFile,
		*Platform,
		*BuildPath
	);
	
	// Execute UAT
	int32 ReturnCode = 0;
	FString StdOut;
	FString StdErr;
	
	FPlatformProcess::ExecProcess(*UATPath, *Arguments, &ReturnCode, &StdOut, &StdErr);
	
	// Update on main thread
	AsyncTask(ENamedThreads::GameThread, [this, ReturnCode, BuildPath, bCompressOnly, StdErr]()
	{
		if (ReturnCode == 0)
		{
			StatusMessage = TEXT("Build completed successfully!");
			StatusMessageType = TEXT("Success");
			UploadProgress = 0.5f;
			
			// Update build detection
			CheckForExistingBuild();
			
			if (bCompressOnly)
			{
				CompressOnly(BuildPath);
			}
			else
			{
				CompressAndUpload(BuildPath);
			}
		}
		else
		{
			StatusMessage = FString::Printf(TEXT("Build failed with code %d. Check the Output Log for details."), ReturnCode);
			StatusMessageType = TEXT("Error");
			bIsBuilding = false;
			UploadProgress = 0.0f;
			
			UE_LOG(LogTemp, Error, TEXT("[GLC] Build error: %s"), *StdErr);
		}
	});
}

void SGLCManagerWindow::CompressOnly(const FString& BuildPath)
{
	StatusMessage = TEXT("Compressing build...");
	StatusMessageType = TEXT("Info");
	UploadProgress = 0.6f;
	
	FString ZipPath = GetZipPath();
	// Use GetBuildSourcePath() to get the correct path (Windows/Mac/Linux subfolder)
	FString ActualBuildPath = GetBuildSourcePath();
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] CompressOnly: Compressing %s to %s"), *ActualBuildPath, *ZipPath);
	
	// Compress in background thread
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, ActualBuildPath, ZipPath]()
	{
		bool bSuccess = CompressBuild(ActualBuildPath, ZipPath);
		
		AsyncTask(ENamedThreads::GameThread, [this, bSuccess, ZipPath]()
		{
			if (bSuccess)
			{
				IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
				int64 ZipSize = PlatformFile.FileSize(*ZipPath);
				
				StatusMessage = FString::Printf(TEXT("Build compressed successfully! Size: %.2f MB"), ZipSize / (1024.0 * 1024.0));
				StatusMessageType = TEXT("Success");
				UploadProgress = 1.0f;
				
				// Update build detection
				CheckForExistingBuild();
			}
			else
			{
				StatusMessage = TEXT("Compression failed. Check the Output Log for details.");
				StatusMessageType = TEXT("Error");
			}
			
			bIsBuilding = false;
			UploadProgress = 0.0f;
		});
	});
}

void SGLCManagerWindow::CompressAndUpload(const FString& BuildPath)
{
	StatusMessage = TEXT("Compressing build...");
	StatusMessageType = TEXT("Info");
	UploadProgress = 0.6f;
	
	FString ZipPath = GetZipPath();
	// Use GetBuildSourcePath() to get the correct path (Windows/Mac/Linux subfolder)
	FString ActualBuildPath = GetBuildSourcePath();
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] CompressAndUpload: Compressing %s to %s"), *ActualBuildPath, *ZipPath);
	
	// Compress in background thread
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, ActualBuildPath, ZipPath]()
	{
		bool bSuccess = CompressBuild(ActualBuildPath, ZipPath);
		
		AsyncTask(ENamedThreads::GameThread, [this, bSuccess, ZipPath]()
		{
			if (bSuccess)
			{
				StatusMessage = TEXT("Compression complete. Upload functionality coming soon!");
				StatusMessageType = TEXT("Info");
				UploadProgress = 0.8f;
				
				// Update build detection
				CheckForExistingBuild();
				
				// TODO: Implement upload here
				// For now just finish
				bIsBuilding = false;
				UploadProgress = 0.0f;
			}
			else
			{
				StatusMessage = TEXT("Compression failed. Check the Output Log for details.");
				StatusMessageType = TEXT("Error");
				bIsBuilding = false;
				UploadProgress = 0.0f;
			}
		});
	});
}

bool SGLCManagerWindow::CompressBuild(const FString& SourcePath, const FString& ZipPath)
{
	UE_LOG(LogTemp, Log, TEXT("[GLC] CompressBuild - Source: %s"), *SourcePath);
	UE_LOG(LogTemp, Log, TEXT("[GLC] CompressBuild - Target: %s"), *ZipPath);
	
	// Verify source exists
	if (!FPaths::DirectoryExists(SourcePath))
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] Source directory does not exist: %s"), *SourcePath);
		return false;
	}
	
	// Ensure target directory exists
	FString ZipDirectory = FPaths::GetPath(ZipPath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ZipDirectory))
	{
		UE_LOG(LogTemp, Log, TEXT("[GLC] Creating directory: %s"), *ZipDirectory);
		PlatformFile.CreateDirectoryTree(*ZipDirectory);
	}
	
	// Use 7-Zip or platform-specific compression
	FString CompressCmd;
	FString Arguments;
	
#if PLATFORM_WINDOWS
	// Try to find 7-Zip
	FString SevenZipPath = TEXT("C:/Program Files/7-Zip/7z.exe");
	if (!FPaths::FileExists(SevenZipPath))
	{
		SevenZipPath = TEXT("C:/Program Files (x86)/7-Zip/7z.exe");
	}
	
	if (FPaths::FileExists(SevenZipPath))
	{
		CompressCmd = SevenZipPath;
		Arguments = FString::Printf(TEXT("a -tzip \"%s\" \"%s\\*\""), *ZipPath, *SourcePath);
		UE_LOG(LogTemp, Log, TEXT("[GLC] Using 7-Zip: %s %s"), *CompressCmd, *Arguments);
	}
	else
	{
		// Fallback to PowerShell - normalize paths to forward slashes
		FString NormalizedSourcePath = SourcePath.Replace(TEXT("\\"), TEXT("/"));
		FString NormalizedZipPath = ZipPath.Replace(TEXT("\\"), TEXT("/"));
		CompressCmd = TEXT("powershell.exe");
		Arguments = FString::Printf(TEXT("-Command \"Compress-Archive -Path '%s/*' -DestinationPath '%s' -Force\""), *NormalizedSourcePath, *NormalizedZipPath);
		UE_LOG(LogTemp, Log, TEXT("[GLC] Using PowerShell: %s %s"), *CompressCmd, *Arguments);
	}
#elif PLATFORM_MAC || PLATFORM_LINUX
	CompressCmd = TEXT("/usr/bin/zip");
	Arguments = FString::Printf(TEXT("-r \"%s\" \"%s\""), *ZipPath, *SourcePath);
	UE_LOG(LogTemp, Log, TEXT("[GLC] Using zip: %s %s"), *CompressCmd, *Arguments);
#endif
	
	int32 ReturnCode = 0;
	FString StdOut;
	FString StdErr;
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] Executing compression command..."));
	FPlatformProcess::ExecProcess(*CompressCmd, *Arguments, &ReturnCode, &StdOut, &StdErr);
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] Compression return code: %d"), ReturnCode);
	if (!StdOut.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[GLC] StdOut: %s"), *StdOut);
	}
	if (!StdErr.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GLC] StdErr: %s"), *StdErr);
	}
	
	if (ReturnCode != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] Compression failed with code %d"), ReturnCode);
		return false;
	}
	
	// Verify the ZIP was created
	if (!FPaths::FileExists(ZipPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] ZIP file was not created at: %s"), *ZipPath);
		return false;
	}
	
	int64 ZipSize = PlatformFile.FileSize(*ZipPath);
	UE_LOG(LogTemp, Log, TEXT("[GLC] Compression successful! ZIP size: %lld bytes"), ZipSize);
	
	return true;
}

void SGLCManagerWindow::StartBuildStatusMonitoring(int64 BuildId)
{
	UE_LOG(LogTemp, Log, TEXT("[GLC] === Starting Build Status Monitor for Build #%lld ==="), BuildId);
	
	CurrentBuildId = BuildId;
	bIsMonitoringBuild = true;
	
	// Start a timer to check status every 5 seconds
	if (GEditor)
	{
		GEditor->GetTimerManager()->SetTimer(
			BuildStatusTimerHandle,
			FTimerDelegate::CreateSP(this, &SGLCManagerWindow::CheckBuildStatus),
			5.0f,  // Check every 5 seconds
			true   // Loop
		);
		
		// Check immediately
		CheckBuildStatus();
	}
}

void SGLCManagerWindow::StopBuildStatusMonitoring()
{
	if (GEditor && BuildStatusTimerHandle.IsValid())
	{
		GEditor->GetTimerManager()->ClearTimer(BuildStatusTimerHandle);
		BuildStatusTimerHandle.Invalidate();
	}
	
	bIsMonitoringBuild = false;
	UE_LOG(LogTemp, Log, TEXT("[GLC] === Build Status Monitor Ended ==="));
}

void SGLCManagerWindow::CheckBuildStatus()
{
	if (!ApiClient.IsValid() || !bIsMonitoringBuild || CurrentBuildId == 0)
	{
		StopBuildStatusMonitoring();
		return;
	}
	
	ApiClient->GetBuildStatusAsync(CurrentBuildId,
		[this](bool bSuccess, FString Error, FGLCBuildStatusResponse Response)
		{
			if (!bSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("[GLC] Failed to get build status: %s"), *Error);
				return;
			}
			
			// Update UI based on status
			FString StatusIcon = GetStatusIcon(Response.Status);
			StatusMessage = FString::Printf(TEXT("%s Build #%lld: %s"), *StatusIcon, Response.AppBuildId, *Response.Status);
			
			// Add progress information if available
			if (Response.StageProgress > 0)
			{
				StatusMessage += FString::Printf(TEXT(" (%d%%)"), Response.StageProgress);
			}
			
			// Check if build is in final state
			if (Response.Status == TEXT("Completed"))
			{
				StatusMessage = FString::Printf(TEXT("✅ Build #%lld completed successfully!"), Response.AppBuildId);
				StatusMessageType = TEXT("Success");
				StopBuildStatusMonitoring();
				
				// Show notification
				FText DialogTitle = FText::FromString(TEXT("Build Completed"));
				FText DialogMessage = FText::FromString(FString::Printf(
					TEXT("Build #%lld processed successfully!\n\nDo you want to view it in Game Launcher Cloud?"),
					Response.AppBuildId
				));
				
				EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, DialogMessage, DialogTitle);
				if (Result == EAppReturnType::Yes)
				{
					FString FrontendUrl = ApiUrl.Replace(TEXT("api."), TEXT("app."));
					FString BuildUrl = FString::Printf(TEXT("%s/apps/id/%lld/builds"), *FrontendUrl, Response.AppId);
					FPlatformProcess::LaunchURL(*BuildUrl, nullptr, nullptr);
				}
			}
			else if (Response.Status == TEXT("Failed"))
			{
				FString ErrorMsg = Response.ErrorMessage.IsEmpty() ? TEXT("Unknown error") : Response.ErrorMessage;
				StatusMessage = FString::Printf(TEXT("❌ Build #%lld failed: %s"), Response.AppBuildId, *ErrorMsg);
				StatusMessageType = TEXT("Error");
				StopBuildStatusMonitoring();
				
				// Show error dialog
				FText DialogTitle = FText::FromString(TEXT("Build Failed"));
				FText DialogMessage = FText::FromString(FString::Printf(
					TEXT("Build #%lld processing failed:\n\n%s"),
					Response.AppBuildId,
					*ErrorMsg
				));
				FMessageDialog::Open(EAppMsgType::Ok, DialogMessage, DialogTitle);
			}
			else if (Response.Status == TEXT("Cancelled") || Response.Status == TEXT("Deleted"))
			{
				StatusMessage = FString::Printf(TEXT("⚠️ Build #%lld was %s"), Response.AppBuildId, *Response.Status.ToLower());
				StatusMessageType = TEXT("Warning");
				StopBuildStatusMonitoring();
			}
			else
			{
				// Still processing
				StatusMessageType = TEXT("Info");
			}
		});
}

FString SGLCManagerWindow::GetStatusIcon(const FString& Status)
{
	if (Status == TEXT("Pending")) return TEXT("⏳");
	if (Status == TEXT("GeneratingPresignedUrl")) return TEXT("🔗");
	if (Status == TEXT("UploadingBuild")) return TEXT("⬆️");
	if (Status == TEXT("Enqueued")) return TEXT("📋");
	if (Status == TEXT("DownloadingBuild")) return TEXT("⬇️");
	if (Status == TEXT("DownloadingPreviousBuild")) return TEXT("⬇️");
	if (Status == TEXT("UnzippingBuild")) return TEXT("📦");
	if (Status == TEXT("UnzippingPreviousBuild")) return TEXT("📦");
	if (Status == TEXT("CreatingPatch")) return TEXT("🔧");
	if (Status == TEXT("DeployingPatch")) return TEXT("🚀");
	if (Status == TEXT("Completed")) return TEXT("✅");
	if (Status == TEXT("Failed")) return TEXT("❌");
	if (Status == TEXT("Cancelled")) return TEXT("⚠️");
	if (Status == TEXT("Deleted")) return TEXT("🗑️");
	
	return TEXT("📊");
}

void SGLCManagerWindow::UploadBuildToCloud(const FString& ZipPath)
{
	UE_LOG(LogTemp, Log, TEXT("[GLC] UploadBuildToCloud called with: %s"), *ZipPath);
	
	if (!ApiClient.IsValid() || !bIsAuthenticated)
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] Not authenticated or ApiClient invalid"));
		StatusMessage = TEXT("Not authenticated");
		StatusMessageType = TEXT("Error");
		bIsUploading = false;
		return;
	}
	
	if (SelectedAppIndex < 0 || !AvailableApps.IsValidIndex(SelectedAppIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] Invalid app selection. Index: %d, Apps count: %d"), SelectedAppIndex, AvailableApps.Num());
		StatusMessage = TEXT("Please select an app");
		StatusMessageType = TEXT("Error");
		bIsUploading = false;
		return;
	}
	
	// Verify file exists
	if (!FPaths::FileExists(ZipPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] Build file does not exist: %s"), *ZipPath);
		StatusMessage = TEXT("Build file not found");
		StatusMessageType = TEXT("Error");
		bIsUploading = false;
		return;
	}
	
	// Get file size
	int64 FileSize = IFileManager::Get().FileSize(*ZipPath);
	if (FileSize <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[GLC] Invalid build file size: %lld for file: %s"), FileSize, *ZipPath);
		
		// Try alternative method using IPlatformFile
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FileSize = PlatformFile.FileSize(*ZipPath);
		
		if (FileSize <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("[GLC] Alternative method also failed. File may be corrupted."));
			StatusMessage = TEXT("Invalid build file. Please rebuild.");
			StatusMessageType = TEXT("Error");
			bIsUploading = false;
			return;
		}
		
		UE_LOG(LogTemp, Log, TEXT("[GLC] Alternative method succeeded. File size: %lld"), FileSize);
	}
	
	FGLCAppInfo SelectedAppInfo = AvailableApps[SelectedAppIndex];
	FString FileName = FPaths::GetCleanFilename(ZipPath);
	
	UE_LOG(LogTemp, Log, TEXT("[GLC] Starting upload - App: %s, File: %s, Size: %lld bytes"), *SelectedAppInfo.Name, *FileName, FileSize);
	
	// Update UI on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		StatusMessage = TEXT("Checking upload limits...");
		StatusMessageType = TEXT("Info");
		UploadProgress = 0.1f;
		if (StatusMessageText.IsValid())
		{
			StatusMessageText->SetText(FText::FromString(StatusMessage));
		}
	});
	
	// Step 1: Check if upload is allowed
	ApiClient->CanUploadAsync(FileSize, UncompressedBuildSize, SelectedAppInfo.Id, 
		[this, SelectedAppInfo, FileName, FileSize, ZipPath](bool bSuccess, FString Error, FGLCCanUploadResponse Response)
		{
			if (!bSuccess)
			{
				AsyncTask(ENamedThreads::GameThread, [this, Error]()
				{
					StatusMessage = FString::Printf(TEXT("Upload check failed: %s"), *Error);
					StatusMessageType = TEXT("Error");
					bIsUploading = false;
					UploadProgress = 0.0f;
					if (StatusMessageText.IsValid())
					{
						StatusMessageText->SetText(FText::FromString(StatusMessage));
					}
				});
				return;
			}
			
			if (!Response.CanUpload)
			{
				AsyncTask(ENamedThreads::GameThread, [this]()
				{
					StatusMessage = TEXT("Cannot upload. Check your plan limits.");
					StatusMessageType = TEXT("Error");
					bIsUploading = false;
					UploadProgress = 0.0f;
					if (StatusMessageText.IsValid())
					{
						StatusMessageText->SetText(FText::FromString(StatusMessage));
					}
				});
				return;
			}
			
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				StatusMessage = TEXT("Starting upload...");
				UploadProgress = 0.2f;
				if (StatusMessageText.IsValid())
				{
					StatusMessageText->SetText(FText::FromString(StatusMessage));
				}
			});
			
			// Use default build notes if empty
			FString Notes = BuildNotesInput.IsEmpty() ? TEXT("Uploaded from Unreal Engine Extension") : BuildNotesInput;
			
			// Step 2: Start upload and get presigned URL
			ApiClient->StartUploadAsync(SelectedAppInfo.Id, FileName, FileSize, UncompressedBuildSize, Notes,
				[this, ZipPath, FileSize](bool bSuccess, FString Error, FGLCStartUploadResponse Response)
				{
				if (!bSuccess)
				{
					AsyncTask(ENamedThreads::GameThread, [this, Error]()
					{
						StatusMessage = FString::Printf(TEXT("Failed to start upload: %s"), *Error);
						StatusMessageType = TEXT("Error");
						bIsUploading = false;
						UploadProgress = 0.0f;
						if (StatusMessageText.IsValid())
						{
							StatusMessageText->SetText(FText::FromString(StatusMessage));
						}
					});
					return;
				}
				
				AsyncTask(ENamedThreads::GameThread, [this]()
				{
					StatusMessage = TEXT("Uploading file to cloud...");
					UploadProgress = 0.3f;
					if (StatusMessageText.IsValid())
					{
						StatusMessageText->SetText(FText::FromString(StatusMessage));
					}
				});
				CurrentBuildId = Response.AppBuildId;					// Track if we've already notified (since callback is called multiple times)
					static bool bHasNotified = false;
					bHasNotified = false;
					
					// Step 3: Upload file to cloud storage
					ApiClient->UploadFileAsync(Response.UploadUrl, ZipPath,
						[this, Response, FileSize](bool bSuccess, FString Error, float Progress)
						{
							// Check if this is a real error (not just a progress update)
							if (!bSuccess && Progress >= 1.0f)
							{
								// Only treat as error if upload is complete but failed
								AsyncTask(ENamedThreads::GameThread, [this, Error]()
								{
									StatusMessage = FString::Printf(TEXT("Upload failed: %s"), *Error);
									StatusMessageType = TEXT("Error");
									bIsUploading = false;
									UploadProgress = 0.0f;
									if (StatusMessageText.IsValid())
									{
										StatusMessageText->SetText(FText::FromString(StatusMessage));
									}
								});
								return;
							}
							
							// Update progress (30% to 90%)
							UploadProgress = 0.3f + (Progress * 0.6f);
							
							// Show progress with percentage and size
							int32 Percentage = FMath::RoundToInt(Progress * 100.0f);
							float SizeMB = FileSize / (1024.0f * 1024.0f);
							StatusMessage = FString::Printf(TEXT("Uploading to cloud storage (%d%% of %.2f MB)..."), Percentage, SizeMB);
							
							// Update UI on game thread
							AsyncTask(ENamedThreads::GameThread, [this]()
							{
								if (StatusMessageText.IsValid())
								{
									StatusMessageText->SetText(FText::FromString(StatusMessage));
								}
							});
							
							if (bSuccess && Progress >= 1.0f && !bHasNotified)
							{
								bHasNotified = true;
								AsyncTask(ENamedThreads::GameThread, [this]()
								{
									StatusMessage = TEXT("Finalizing upload...");
									UploadProgress = 0.95f;
									if (StatusMessageText.IsValid())
									{
										StatusMessageText->SetText(FText::FromString(StatusMessage));
									}
								});
								
								// Step 4: Notify backend that file is ready
								ApiClient->NotifyFileReadyAsync(Response.AppBuildId, Response.Key,
									[this](bool bSuccess, FString Error)
									{
										if (!bSuccess)
										{
											AsyncTask(ENamedThreads::GameThread, [this, Error]()
											{
												StatusMessage = FString::Printf(TEXT("Failed to finalize upload: %s"), *Error);
												StatusMessageType = TEXT("Error");
												bIsUploading = false;
												UploadProgress = 0.0f;
												if (StatusMessageText.IsValid())
												{
													StatusMessageText->SetText(FText::FromString(StatusMessage));
												}
											});
											return;
										}
										
										AsyncTask(ENamedThreads::GameThread, [this]()
										{
											StatusMessage = TEXT("Upload completed! Your build is now processing.");
											StatusMessageType = TEXT("Success");
											bIsUploading = false;
											UploadProgress = 1.0f;
											if (StatusMessageText.IsValid())
											{
												StatusMessageText->SetText(FText::FromString(StatusMessage));
											}
										});
										
										// Start monitoring build status
										StartBuildStatusMonitoring(CurrentBuildId);
									});
							}
						});
				});
		});
}

#undef LOCTEXT_NAMESPACE
