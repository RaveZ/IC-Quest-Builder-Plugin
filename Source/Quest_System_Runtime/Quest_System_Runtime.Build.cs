// Copyright 2024 Ivan Chandra. All Rights Reserved.
using UnrealBuildTool;

public class Quest_System_Runtime : ModuleRules
{
    public Quest_System_Runtime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;
        ShadowVariableWarningLevel = WarningLevel.Error;

        PublicIncludePaths.AddRange(
            new string[] {
			}
            );

        PrivateIncludePaths.AddRange(
            new string[] {
                "Quest_System_Runtime/Private",
			}
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
			}
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "GameplayTags",
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
			}
            );
    }
}