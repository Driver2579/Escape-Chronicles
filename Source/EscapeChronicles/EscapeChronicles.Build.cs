// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EscapeChronicles : ModuleRules
{
	public EscapeChronicles(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Mover",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"InteractionSystem",
			"InventorySystem",
			"FragmentationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			
		});
	}
}
