// Copyright 2026 Simulated Flow. All Rights Reserved.

using UnrealBuildTool;

public class RTSLandscapeArchitect : ModuleRules
{
	public RTSLandscapeArchitect(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"ProceduralMeshComponent",
			"NavigationSystem",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"PCG",
		});
	}
}
