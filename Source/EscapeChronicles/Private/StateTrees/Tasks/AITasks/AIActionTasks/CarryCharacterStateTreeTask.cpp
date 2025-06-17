// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/AIActionTasks/CarryCharacterStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/CarryCharacterComponent.h"

FCarryCharacterStateTreeTask::FCarryCharacterStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FCarryCharacterStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.CarryingCharacter);
	check(InstanceData.TargetCharacter);
#endif

	// Carry the target character and remember the result
	const bool bResult = InstanceData.CarryingCharacter->GetCarryCharacterComponent()->SetCarriedCharacter(
		InstanceData.TargetCharacter);

	return bResult ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

void FCarryCharacterStateTreeTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Stop carrying the character on task exit if we should
	if (InstanceData.bStopCarryingOnExit)
	{
#if DO_CHECK
		check(InstanceData.CarryingCharacter);
#endif

		InstanceData.CarryingCharacter->GetCarryCharacterComponent()->SetCarriedCharacter(nullptr);
	}
}