// Copyright 2024 Ivan Chandra. All Rights Reserved.
using UnrealBuildTool;

public class Quest_System_Editor : ModuleRules
{
	public Quest_System_Editor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] 
			{
			}
			);
		
		PrivateIncludePaths.AddRange(
			new string[] 
			{
                "Quest_System_Editor/Private",
                "Quest_System_Editor/Public",
			}
			);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Quest_System_Runtime",
                "Core",
                "CoreUObject",
                "Engine",
                "UnrealEd",
				"Kismet",
				"AIGraph",
				"EditorStyle",
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
   {
                "Quest_System_Runtime", "AssetTools", "GraphEditor", "PropertyEditor", "EditorStyle", "Kismet", "KismetWidgets", "ApplicationCore", "ToolMenus", "Projects", "InputCore", "UnrealEd", "ToolMenus", "CoreUObject", "Engine", "Slate", "SlateCore", "AIGraph",
   }
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
