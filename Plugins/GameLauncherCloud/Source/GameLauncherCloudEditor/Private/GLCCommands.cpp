// Copyright Game Launcher Cloud. All Rights Reserved.

#include "GLCCommands.h"

#define LOCTEXT_NAMESPACE "FGameLauncherCloudModule"

void FGLCCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Game Launcher Cloud", "Open Game Launcher Cloud Manager window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
