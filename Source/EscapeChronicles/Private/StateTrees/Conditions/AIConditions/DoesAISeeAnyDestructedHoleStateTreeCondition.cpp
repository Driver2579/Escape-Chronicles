// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/AIConditions/DoesAISeeAnyDestructedHoleStateTreeCondition.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Common/Structs/FunctionLibriries/AIPerceptionFunctionLibrary.h"
#include "Components/ActorComponents/DestructibleComponent.h"
#include "Perception/AIPerceptionComponent.h"

bool FDoesAISeeAnyDestructedHoleStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.AIController);
#endif

	const UAIPerceptionComponent* AIPerceptionComponent = InstanceData.AIController->GetAIPerceptionComponent();

	// Return false if the AI perception component is invalid. If bInvert is true, then true will be returned.
	if (!ensureAlways(IsValid(AIPerceptionComponent)))
	{
		return bInvert;
	}

	for (const AActor* SeenActor : InstanceData.SeenActors)
	{
		const UDestructibleComponent* DestructibleComponent = SeenActor->FindComponentByClass<UDestructibleComponent>();

		// Skip the actor if it doesn't have a destructible component
		if (!IsValid(DestructibleComponent))
		{
			continue;
		}

		for (const FDynamicMeshHoleData& Hole : DestructibleComponent->GetHoles())
		{
			// Convert the relative location of the hole to a world location
			FVector HoleWorldLocation;
			DestructibleComponent->GetHoleWorldLocation(Hole.RelativeLocation, HoleWorldLocation);

			// Make the extent of the hole based on its radius
			FVector HoleExtent(Hole.Radius);

			/**
			 * Return true if the AI perception component can see the hole in the destructible component. If bInvert is
			 * true, then false will be returned.
			 */
			if (FAIPerceptionFunctionLibrary::IsBoxVisible(AIPerceptionComponent, HoleWorldLocation, HoleExtent))
			{
				return !bInvert;
			}
		}
	}

	/**
	 * Return false if no destructed holes were found in the seen actors. If bInvert is true, then true will be
	 * returned.
	 */
	return bInvert;
}