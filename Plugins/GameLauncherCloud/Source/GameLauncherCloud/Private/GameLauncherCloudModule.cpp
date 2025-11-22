// Copyright Game Launcher Cloud. All Rights Reserved.

#include "GameLauncherCloudModule.h"

#define LOCTEXT_NAMESPACE "FGameLauncherCloudModule"

void FGameLauncherCloudModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("GameLauncherCloud Runtime Module Started"));
}

void FGameLauncherCloudModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("GameLauncherCloud Runtime Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGameLauncherCloudModule, GameLauncherCloud)
