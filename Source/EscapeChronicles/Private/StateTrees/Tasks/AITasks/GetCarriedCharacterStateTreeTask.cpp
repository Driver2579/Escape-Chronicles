// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/GetCarriedCharacterStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/CarryCharacterComponent.h"

FGetCarriedCharacterStateTreeTask::FGetCarriedCharacterStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetCarriedCharacterStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.CarryingCharacter);
#endif

	AEscapeChroniclesCharacter* CarriedCharacter =
		InstanceData.CarryingCharacter->GetCarryCharacterComponent()->GetCarriedCharacter();

	// If the character is not carrying anyone, return the Failed status
	if (!IsValid(CarriedCharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.OutCarriedCharacter = CarriedCharacter;

	return EStateTreeRunStatus::Succeeded;
}