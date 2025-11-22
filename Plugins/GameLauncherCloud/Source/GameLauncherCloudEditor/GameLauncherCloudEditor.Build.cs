// Copyright Game Launcher Cloud. All Rights Reserved.

using UnrealBuildTool;

public class GameLauncherCloudEditor : ModuleRules
{
public GameLauncherCloudEditor(ReadOnlyTargetRules Target) : base(Target)
{
PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
bWarningsAsErrors = false;

PublicDependencyModuleNames.AddRange(
new string[]
{
"Core",
"GameLauncherCloud",
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
"SlateCore",
"UnrealEd",
"EditorStyle",
"DesktopPlatform",
"ToolMenus",
"WorkspaceMenuStructure",
"InputCore"
}
);
}
}
