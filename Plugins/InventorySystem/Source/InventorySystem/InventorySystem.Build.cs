// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class InventorySystem : ModuleRules
{
	public InventorySystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"NetCore",
				"GameplayTags",
				"Slate",
				"SlateCore",
			});
		
		PrivateDependencyModuleNames.AddRange(new string[]
			{
				"CoreUObject",
				"Engine",
			});
	}
}
