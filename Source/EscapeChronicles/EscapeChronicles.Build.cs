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
			"PhysicsCore",
			"InputCore",
			"EnhancedInput",
			"Mover",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"InteractionSystem",
			"InventorySystem",
			"UMG",
			"CommonUI",
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