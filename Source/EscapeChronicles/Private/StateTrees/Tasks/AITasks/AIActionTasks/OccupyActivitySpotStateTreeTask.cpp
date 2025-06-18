// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/AIActionTasks/OccupyActivitySpotStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Actors/ActivitySpot.h"

FOccupyActivitySpotStateTreeTask::FOccupyActivitySpotStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FOccupyActivitySpotStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Character);
	check(InstanceData.ActivitySpot);
#endif

	// Try to occupy the spot and remember the result
	const bool bResult = InstanceData.ActivitySpot->SetOccupyingCharacter(InstanceData.Character);

	// Return the status based on whether the occupation was successful
	return bResult ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

void FOccupyActivitySpotStateTreeTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Unoccupy the activity spot on task exit if we should
	if (InstanceData.bUnoccupyOnExit)
	{
#if DO_CHECK
		check(InstanceData.ActivitySpot);
#endif

		InstanceData.ActivitySpot->SetOccupyingCharacter(nullptr);
	}
}