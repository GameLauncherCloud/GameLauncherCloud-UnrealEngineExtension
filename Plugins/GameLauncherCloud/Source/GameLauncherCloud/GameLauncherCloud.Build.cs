// Copyright Game Launcher Cloud. All Rights Reserved.

using UnrealBuildTool;

public class GameLauncherCloud : ModuleRules
{
public GameLauncherCloud(ReadOnlyTargetRules Target) : base(Target)
{
PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
bWarningsAsErrors = false;

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
