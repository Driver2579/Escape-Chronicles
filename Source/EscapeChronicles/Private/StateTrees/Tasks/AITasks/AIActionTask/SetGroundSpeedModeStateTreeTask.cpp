// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/AIActionTasks/SetGroundSpeedModeStateTreeTask.h"

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

#if DO_CHECK
	check(InstanceData.OwnerCharacter);
#endif

	InstanceData.OwnerCharacter->OverrideGroundSpeedMode(InstanceData.GroundSpeedMode);

	return InstanceData.bRunTaskForever ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
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