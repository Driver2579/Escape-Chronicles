// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/FindSmartObjectByClassStateTreeTask.h"

#include "NavigationSystem.h"
#include "SmartObjectRequestTypes.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"

FFindSmartObjectByClassStateTreeTask::FFindSmartObjectByClassStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FFindSmartObjectByClassStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!ensureAlways(IsValid(InstanceData.ActorClass)))
	{
		return EStateTreeRunStatus::Failed;
	}

	const UWorld* World = Context.GetWorld();

	if (!ensureAlways(IsValid(World)))
	{
		return EStateTreeRunStatus::Failed;
	}

	// Get all actors of the specified class
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, InstanceData.ActorClass, FoundActors);

	const USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();

	// Ensure the SmartObjectSubsystem is valid
	if (!ensureAlways(IsValid(SmartObjectSubsystem)))
	{
		return EStateTreeRunStatus::Failed;
	}

	// Find smart objects in the list of found actors
	TArray<FSmartObjectRequestResult> FindSmartObjectsResults;
	SmartObjectSubsystem->FindSmartObjectsInList(InstanceData.SmartObjectRequestFilter, FoundActors,
		FindSmartObjectsResults,
		FConstStructView::Make(FSmartObjectActorUserData(InstanceData.UserActor)));

	// If no smart objects were found, return failed status
	if (FindSmartObjectsResults.IsEmpty())
	{
		return EStateTreeRunStatus::Failed;
	}

	// Return the nearest smart object if requested and if the UserActor is specified
	if (InstanceData.FindSmartObjectMethod != EFindSmartObjectMethod::First && ensureAlways(InstanceData.UserActor))
	{
		const FVector UserActorLocation = InstanceData.UserActor->GetActorLocation();

		double ClosestDistance = TNumericLimits<double>::Max();

		for (const FSmartObjectRequestResult& FindSmartObjectResult : FindSmartObjectsResults)
		{
			FTransform SlotTransform;
			SmartObjectSubsystem->GetSlotTransformFromRequestResult(FindSmartObjectResult, SlotTransform);

			const double Distance = FVector::Dist(UserActorLocation, SlotTransform.GetLocation());

			// We can't select the smart object if it's not in an acceptable radius it should be checked
			const bool bCanSelectSmartObject =
				InstanceData.FindSmartObjectMethod != EFindSmartObjectMethod::NearestInAcceptableRadius ||
					Distance <= InstanceData.AcceptableRadius;

			/**
			 * If we can select this smart object, then remember the smart object if it's closer to the UserActor than
			 * the previously found one.
			 */
			if (bCanSelectSmartObject && Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				InstanceData.OutSmartObjectRequestResult = FindSmartObjectResult;
			}
		}

		/**
		 * Return the succeeded status if we found the smart object in the acceptable radius (it will always be valid if
		 * we didn't have to check for the acceptable radius).
		 */
		if (InstanceData.OutSmartObjectRequestResult.IsValid())
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}

	/**
	 * Always fall back to the first found smart object if we didn't find it in the code above. This will only be
	 * triggered if the finding method is set to First, or if the UserActor is not specified, or if the finding method
	 * is set to NearestInAcceptableRadius and no smart object was found within the acceptable radius.
	 */
	InstanceData.OutSmartObjectRequestResult = FindSmartObjectsResults[0];

	return EStateTreeRunStatus::Succeeded;
}