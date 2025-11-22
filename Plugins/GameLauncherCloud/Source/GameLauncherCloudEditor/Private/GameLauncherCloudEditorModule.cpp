// Copyright Game Launcher Cloud. All Rights Reserved.

#include "GameLauncherCloudEditorModule.h"
#include "GLCManagerWindow.h"
#include "GLCCommands.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

static const FName GLCManagerTabName("GLCManager");

#define LOCTEXT_NAMESPACE "FGameLauncherCloudEditorModule"

void FGameLauncherCloudEditorModule::StartupModule()
{
	// Initialize style
	StyleSet = MakeShareable(new FSlateStyleSet("GameLauncherCloudStyle"));
	
	// Find plugin content directory
	FString ContentDir = IPluginManager::Get().FindPlugin("GameLauncherCloud")->GetBaseDir() / TEXT("Resources");
	StyleSet->SetContentRoot(ContentDir);
	
	// Register icon
	StyleSet->Set("GameLauncherCloud.Icon", new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("GameLauncherCloud_Icon.png")), FVector2D(48.0f, 48.0f)));
	
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	
	// Register commands
	FGLCCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FGLCCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FGameLauncherCloudEditorModule::OpenManagerWindow),
		FCanExecuteAction());

	// Register tab spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GLCManagerTabName, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SGLCManagerWindow)
			];
	}))
	.SetDisplayName(LOCTEXT("GLCManagerTabTitle", "Game Launcher Cloud Manager"))
	.SetMenuType(ETabSpawnerMenuType::Hidden)
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGameLauncherCloudEditorModule::RegisterMenus));

	UE_LOG(LogTemp, Log, TEXT("GameLauncherCloud Editor Module Started"));
}

void FGameLauncherCloudEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGLCCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GLCManagerTabName);
	
	// Unregister style
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		StyleSet.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("GameLauncherCloud Editor Module Shutdown"));
}

void FGameLauncherCloudEditorModule::OpenManagerWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(GLCManagerTabName);
}

void FGameLauncherCloudEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("GameLauncherCloud");
			Section.AddMenuEntryWithCommandList(FGLCCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("GameLauncherCloud");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FGLCCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGameLauncherCloudEditorModule, GameLauncherCloudEditor)
