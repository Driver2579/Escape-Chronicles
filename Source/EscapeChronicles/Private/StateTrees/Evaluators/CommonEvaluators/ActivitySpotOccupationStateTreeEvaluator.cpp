// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Evaluators/CommonEvaluators/ActivitySpotOccupationStateTreeEvaluator.h"

#include "StateTreeExecutionContext.h"
#include "Actors/ActivitySpot.h"

void FActivitySpotOccupationStateTreeEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.ActivitySpot);
#endif

	// Initialize the occupation state
	InstanceData.bOccupied = InstanceData.ActivitySpot->IsOccupied();
}

void FActivitySpotOccupationStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.ActivitySpot);
#endif

	const bool bOccupied = InstanceData.ActivitySpot->IsOccupied();

	// Return if the occupation state hasn't changed
	if (bOccupied == InstanceData.bOccupied)
	{
		return;
	}

	// Remember the new occupation state
	InstanceData.bOccupied = bOccupied;

	// If the activity spot is now occupied, notify that the spot is now occupied
	if (bOccupied)
	{
		Context.BroadcastDelegate(InstanceData.OnActivitySpotOccupied);
	}
	// Otherwise, notify that the spot is now unoccupied
	else
	{
		Context.BroadcastDelegate(InstanceData.OnActivitySpotUnoccupied);
	}
}