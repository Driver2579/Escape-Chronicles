// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/StateTreeTasks/StateTreeAITasks/StateTreeAIActionTasks/SetGroundSpeedModeStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"

FSetGroundSpeedModeStateTreeTask::FSetGroundSpeedModeStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FSetGroundSpeedModeStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const EStateTreeRunStatus PositiveReturnStatus = InstanceData.bRunTaskForever ?
		EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;

#if DO_CHECK
	check(InstanceData.OwnerCharacter);
#endif

	InstanceData.OwnerCharacter->OverrideGroundSpeedMode(InstanceData.GroundSpeedMode);

	return PositiveReturnStatus;
}

void FSetGroundSpeedModeStateTreeTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Reset the ground speed mode only if we have to
	if (InstanceData.bResetGroundSpeedModeOnTaskEnd)
	{
#if DO_CHECK
		check(InstanceData.OwnerCharacter);
#endif

		InstanceData.OwnerCharacter->ResetGroundSpeedMode(InstanceData.GroundSpeedMode);
	}
}