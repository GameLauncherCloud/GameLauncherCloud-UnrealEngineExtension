// Copyright Game Launcher Cloud. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/// <summary>
/// Main runtime module for Game Launcher Cloud plugin
/// </summary>
class FGameLauncherCloudModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
