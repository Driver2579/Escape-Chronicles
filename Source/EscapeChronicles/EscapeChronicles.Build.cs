// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EscapeChronicles : ModuleRules
{
	public EscapeChronicles(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
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
			"OnlineSubsystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"SmartObjectsModule",
			"NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"OnlineSubsystemUtils",
			"GameplayBehaviorSmartObjectsModule"
		});
	}
}