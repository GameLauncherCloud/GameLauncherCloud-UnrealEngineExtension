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
	
	ApiUrl = TEXT("https://api.gamelauncher.cloud");
	
	LoadConfig();
	
	ApiClient = MakeShareable(new FGLCApiClient(ApiUrl, AuthToken));
	
	// Auto-load apps if already authenticated
	if (bIsAuthenticated && !AuthToken.IsEmpty())
	{
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
					.AutoHeight()
					.Padding(0.0f, 8.0f, 0.0f, 0.0f)
					[
						SAssignNew(BuildNotesTextBox, SEditableTextBox)
						.HintText(LOCTEXT("BuildNotesHint", "What's new in this build?"))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
						.OnTextChanged_Lambda([this](const FText& NewText) { BuildNotesInput = NewText.ToString(); })
					]
				]
			]
			
			// Build & Upload button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 20.0f)
			.HAlign(HAlign_Center)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
				.ForegroundColor(FLinearColor::White)
				.ContentPadding(FMargin(50.0f, 15.0f))
				.OnClicked(this, &SGLCManagerWindow::OnBuildAndUploadClicked)
				.IsEnabled_Lambda([this]() { return !bIsBuilding && !bIsUploading && AvailableApps.Num() > 0 && SelectedAppIndex >= 0; })
				[
					SNew(STextBlock)
					.Text_Lambda([this]() {
						if (bIsBuilding) return LOCTEXT("Building", "🔨 Building...");
						if (bIsUploading) return LOCTEXT("Uploading", "⬆️ Uploading...");
						return LOCTEXT("BuildUploadButton", "🚀 Build & Upload to Cloud");
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
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
			
			// Status message
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
				.Visibility_Lambda([this]() { return StatusMessage.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
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
				StatusMessage.Empty(); // Clear loading message
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

#undef LOCTEXT_NAMESPACE
