// Copyright Game Launcher Cloud. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/// <summary>
/// UI Commands for Game Launcher Cloud plugin
/// </summary>
class FGLCCommands : public TCommands<FGLCCommands>
{
public:
	FGLCCommands()
		: TCommands<FGLCCommands>(TEXT("GameLauncherCloud"), NSLOCTEXT("Contexts", "GameLauncherCloud", "Game Launcher Cloud Plugin"), NAME_None, TEXT("GameLauncherCloudStyle"))
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
