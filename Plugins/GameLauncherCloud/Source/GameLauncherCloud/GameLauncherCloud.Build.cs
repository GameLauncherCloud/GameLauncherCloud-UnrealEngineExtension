// Copyright Game Launcher Cloud. All Rights Reserved.

using UnrealBuildTool;

public class GameLauncherCloud : ModuleRules
{
public GameLauncherCloud(ReadOnlyTargetRules Target) : base(Target)
{
PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
bWarningsAsErrors = false;

// Disable specific warnings for Epic's build servers (UE 5.6+)
CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
if (Target.Platform == UnrealTargetPlatform.Win64)
{
bEnableExceptions = true;
}

PublicDependencyModuleNames.AddRange(
new string[]
{
"Core",
"HTTP",
"Json",
"JsonUtilities"
}
);

PrivateDependencyModuleNames.AddRange(
new string[]
{
"CoreUObject",
"Engine",
"Slate",
"SlateCore"
}
);
}
}
