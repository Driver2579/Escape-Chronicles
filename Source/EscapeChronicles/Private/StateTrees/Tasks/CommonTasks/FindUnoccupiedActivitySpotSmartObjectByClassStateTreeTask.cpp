// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/FindUnoccupiedActivitySpotSmartObjectByClassStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Actors/ActivitySpot.h"

EStateTreeRunStatus FFindUnoccupiedActivitySpotSmartObjectByClassStateTreeTask::EnterState(
	FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	const bool bActorClassValid = ensureAlways(IsValid(InstanceData.ActorClass)) &&
		ensureAlways(InstanceData.ActorClass->IsChildOf(AActivitySpot::StaticClass()));

	if (!bActorClassValid)
	{
		return EStateTreeRunStatus::Failed;
	}

	return FFindSmartObjectByClassStateTreeTask::EnterState(Context, Transition);
}

void FFindUnoccupiedActivitySpotSmartObjectByClassStateTreeTask::FilterFoundActors(FStateTreeExecutionContext& Context,
	TArray<AActor*>& InOutFoundActors) const
{
	FFindSmartObjectByClassStateTreeTask::FilterFoundActors(Context, InOutFoundActors);

	// Remove all activity spots that are currently occupied
	for (int32 i = InOutFoundActors.Num() - 1; i >= 0; --i)
	{
#if DO_CHECK
		check(InOutFoundActors[i]);
		check(InOutFoundActors[i]->IsA(AActivitySpot::StaticClass()));
#endif

		const AActivitySpot* ActivitySpot = CastChecked<AActivitySpot>(InOutFoundActors[i]);

		if (ActivitySpot->IsOccupied())
		{
			InOutFoundActors.RemoveAt(i, EAllowShrinking::No);
		}
	}

	// Shrink the array to release unused memory
	InOutFoundActors.Shrink();
}