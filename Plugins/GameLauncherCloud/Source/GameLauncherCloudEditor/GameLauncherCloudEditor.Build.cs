// Copyright Game Launcher Cloud. All Rights Reserved.

using UnrealBuildTool;

public class GameLauncherCloudEditor : ModuleRules
{
public GameLauncherCloudEditor(ReadOnlyTargetRules Target) : base(Target)
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
"GameLauncherCloud",
"HTTP",
"Json",
"JsonUtilities",
"Projects"
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
