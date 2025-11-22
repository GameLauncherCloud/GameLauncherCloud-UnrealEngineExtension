// Copyright Game Launcher Cloud. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

/// <summary>
/// Editor module for Game Launcher Cloud plugin
/// Provides UI for authentication, building, and uploading game builds
/// </summary>
class FGameLauncherCloudEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** Opens the Game Launcher Cloud Manager window */
	void OpenManagerWindow();

private:
	void RegisterMenus();

	TSharedPtr<class FUICommandList> PluginCommands;
};
