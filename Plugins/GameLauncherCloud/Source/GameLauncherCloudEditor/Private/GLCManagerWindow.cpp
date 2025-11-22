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
	
	ApiUrl = TEXT("https://app.gamelauncher.cloud");
	
	LoadConfig();
	
	ApiClient = MakeShareable(new FGLCApiClient(ApiUrl, AuthToken));
	
	ChildSlot
	[
		SNew(SVerticalBox)
		
		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f, 20.0f, 20.0f, 10.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "Game Launcher Cloud"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Subtitle", "Build and Upload Manager for Unreal Engine"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
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
				SNew(SVerticalBox)
				
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
	];
}

void SGLCManagerWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

TSharedRef<SWidget> SGLCManagerWindow::ConstructLoginTab()
{
	return SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("LoginTitle", "Login to Game Launcher Cloud"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ApiKeyLabel", "API Key"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ApiKeyTextBox, SEditableTextBox)
			.HintText(LOCTEXT("ApiKeyHint", "Enter your Game Launcher Cloud API Key"))
			.IsPassword(true)
			.OnTextChanged_Lambda([this](const FText& NewText) { ApiKeyInput = NewText.ToString(); })
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 10.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("LoginButton", "Login with API Key"))
			.OnClicked(this, &SGLCManagerWindow::OnLoginWithApiKeyClicked)
			.IsEnabled_Lambda([this]() { return !bIsLoggingIn && !ApiKeyInput.IsEmpty(); })
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 10.0f)
		[
			SAssignNew(StatusMessageText, STextBlock)
			.Text(FText::FromString(StatusMessage))
			.Visibility_Lambda([this]() { return StatusMessage.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 20.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GetApiKeyInfo", "Get your API Key from:\nhttps://app.gamelauncher.cloud/dashboard/settings/api-keys"))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		];
}

TSharedRef<SWidget> SGLCManagerWindow::ConstructBuildUploadTab()
{
	return SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("WelcomeMessage", "Welcome, {0}"), FText::FromString(UserEmail)))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("LogoutButton", "Logout"))
				.OnClicked(this, &SGLCManagerWindow::OnLogoutClicked)
			]
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 20.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BuildUploadTitle", "Build & Upload"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SelectAppLabel", "Select App"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("LoadAppsButton", "Load My Apps"))
				.OnClicked(this, &SGLCManagerWindow::OnLoadAppsClicked)
				.IsEnabled_Lambda([this]() { return !bIsLoadingApps; })
			]
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(AppComboBox, SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&AppNames)
			.OnSelectionChanged(this, &SGLCManagerWindow::OnAppSelected)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
			{
				return SNew(STextBlock).Text(FText::FromString(*Item));
			})
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return SelectedApp.IsValid() ? FText::FromString(*SelectedApp) : LOCTEXT("SelectApp", "Select an app...");
				})
			]
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 10.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BuildNotesLabel", "Build Notes (optional)"))
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(BuildNotesTextBox, SEditableTextBox)
			.HintText(LOCTEXT("BuildNotesHint", "What's new in this build?"))
			.OnTextChanged_Lambda([this](const FText& NewText) { BuildNotesInput = NewText.ToString(); })
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 10.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("BuildUploadButton", "Build & Upload to Game Launcher Cloud"))
			.OnClicked(this, &SGLCManagerWindow::OnBuildAndUploadClicked)
			.IsEnabled_Lambda([this]() { return !bIsBuilding && !bIsUploading && AvailableApps.Num() > 0 && SelectedAppIndex >= 0; })
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 10.0f)
		[
			SAssignNew(UploadProgressBar, SProgressBar)
			.Percent_Lambda([this]() { return UploadProgress; })
			.Visibility_Lambda([this]() { return (bIsBuilding || bIsUploading) ? EVisibility::Visible : EVisibility::Collapsed; })
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(StatusMessageText, STextBlock)
			.Text(FText::FromString(StatusMessage))
			.Visibility_Lambda([this]() { return StatusMessage.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
		];
}

TSharedRef<SWidget> SGLCManagerWindow::ConstructTipsTab()
{
	return SNew(SBorder)
		.Padding(20.0f)
		[
			SNew(SVerticalBox)
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TipsTitle", "💡 Tips for Better Builds"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 10.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Tips", 
					"• Test your build locally before uploading\n"
					"• Write descriptive build notes\n"
					"• Optimize textures and assets to reduce build size\n"
					"• Use version control (Git) for your project\n"
					"• Check the Game Launcher Cloud dashboard for build status\n"))
				.AutoWrapText(true)
			]
		];
}

FReply SGLCManagerWindow::OnLoginWithApiKeyClicked()
{
	if (ApiKeyInput.IsEmpty())
	{
		StatusMessage = TEXT("Please enter an API Key");
		StatusMessageType = TEXT("Error");
		return FReply::Handled();
	}
	
	bIsLoggingIn = true;
	StatusMessage = TEXT("Logging in...");
	StatusMessageType = TEXT("Info");
	
	ApiClient->LoginWithApiKeyAsync(ApiKeyInput, [this](bool bSuccess, FString Message, FGLCLoginResponse Response)
	{
		bIsLoggingIn = false;
		
		if (bSuccess)
		{
			bIsAuthenticated = true;
			AuthToken = Response.Token;
			UserEmail = Response.Email;
			ApiClient->SetAuthToken(AuthToken);
			SaveConfig();
			
			StatusMessage = TEXT("Login successful!");
			StatusMessageType = TEXT("Success");
			
			// Reload UI
			Construct(FArguments());
		}
		else
		{
			StatusMessage = Message;
			StatusMessageType = TEXT("Error");
		}
	});
	
	return FReply::Handled();
}

FReply SGLCManagerWindow::OnLogoutClicked()
{
	bIsAuthenticated = false;
	AuthToken.Empty();
	UserEmail.Empty();
	ApiKeyInput.Empty();
	AvailableApps.Empty();
	AppNames.Empty();
	SaveConfig();
	
	StatusMessage.Empty();
	
	// Reload UI
	Construct(FArguments());
	
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
				StatusMessage = FString::Printf(TEXT("Loaded %d app(s)"), Apps.Num());
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
	JsonObject->SetStringField(TEXT("authToken"), AuthToken);
	JsonObject->SetStringField(TEXT("userEmail"), UserEmail);
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
			JsonObject->TryGetStringField(TEXT("apiUrl"), ApiUrl);
			
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

#undef LOCTEXT_NAMESPACE
