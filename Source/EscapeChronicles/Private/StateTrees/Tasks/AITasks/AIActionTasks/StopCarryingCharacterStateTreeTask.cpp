// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/AIActionTasks/StopCarryingCharacterStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/CarryCharacterComponent.h"

FStopCarryingCharacterStateTreeTask::FStopCarryingCharacterStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FStopCarryingCharacterStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.CarryingCharacter);
#endif

	// Stop carrying the character if any
	InstanceData.CarryingCharacter->GetCarryCharacterComponent()->SetCarriedCharacter(nullptr);

	return EStateTreeRunStatus::Succeeded;
}