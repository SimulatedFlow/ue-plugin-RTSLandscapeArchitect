// Copyright 2026 Silvan Teufel. All Rights Reserved.

using UnrealBuildTool;

public class RTSLandscapeArchitectEditor : ModuleRules
{
	public RTSLandscapeArchitectEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"RTSLandscapeArchitect",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UnrealEd",
			"PropertyEditor",
			"InputCore",
		});
	}
}
