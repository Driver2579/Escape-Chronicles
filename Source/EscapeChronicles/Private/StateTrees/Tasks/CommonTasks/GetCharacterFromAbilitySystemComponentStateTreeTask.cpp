// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/GetCharacterFromAbilitySystemComponentStateTreeTask.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"

FGetCharacterFromAbilitySystemComponentStateTreeTask::FGetCharacterFromAbilitySystemComponentStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetCharacterFromAbilitySystemComponentStateTreeTask::EnterState(
	FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.AbilitySystemComponent);
#endif

	AActor* AvatarActor = InstanceData.AbilitySystemComponent->GetAvatarActor();

	// Return Failed if AvatarActor isn't a valid character
	if (!IsValid(AvatarActor) || !AvatarActor->IsA<AEscapeChroniclesCharacter>())
	{
		return EStateTreeRunStatus::Failed;
	}

	// Cast the AvatarActor to the character type and set it to the output if it's a valid character
	InstanceData.OutCharacter = CastChecked<AEscapeChroniclesCharacter>(AvatarActor);

	return EStateTreeRunStatus::Succeeded;
}