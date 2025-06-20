// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/AIActionTasks/UnoccupyCurrentActivitySpotStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

FUnoccupyCurrentActivitySpotStateTreeTask::FUnoccupyCurrentActivitySpotStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FUnoccupyCurrentActivitySpotStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Character);
#endif

	const AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		InstanceData.Character->GetPlayerState(), ECastCheckedType::NullAllowed);

	// Return Failed if the PlayerState isn't set yet
	if (!IsValid(PlayerState))
	{
		return EStateTreeRunStatus::Failed;
	}

	AActivitySpot* ActivitySpot = PlayerState->GetOccupyingActivitySpot();

	// Return Failed if the character doesn't occupy any activity spot
	if (!IsValid(ActivitySpot))
	{
		return EStateTreeRunStatus::Failed;
	}

	// Finally, unoccupy the activity spot
	ActivitySpot->SetOccupyingCharacter(nullptr);

	return EStateTreeRunStatus::Succeeded;
}