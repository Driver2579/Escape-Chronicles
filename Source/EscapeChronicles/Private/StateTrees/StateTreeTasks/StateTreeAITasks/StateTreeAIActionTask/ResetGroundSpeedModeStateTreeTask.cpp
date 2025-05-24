// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/StateTreeTasks/StateTreeAITasks/StateTreeAIActionTasks/ResetGroundSpeedModeStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"

FResetGroundSpeedModeStateTreeTask::FResetGroundSpeedModeStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FResetGroundSpeedModeStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const EStateTreeRunStatus PositiveReturnStatus = InstanceData.bRunTaskForever ?
		EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;

#if DO_CHECK
	check(InstanceData.OwnerCharacter);
#endif

	// Force reset the ground speed mode if we have to
	if (InstanceData.bForceResetGroundSpeedMode)
	{
		InstanceData.OwnerCharacter->ForceResetGroundSpeedMode();
	}
	// Otherwise, reset the ground speed mode depending on the one it was overriden to
	else
	{
		InstanceData.OwnerCharacter->ResetGroundSpeedMode(InstanceData.GroundSpeedModeOverrideToReset);
	}

	return PositiveReturnStatus;
}