// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/AIActionTasks/ActivateGameplayAbilityByClassStateTreeTask.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Abilities/GameplayAbility.h"
#include "Characters/EscapeChroniclesCharacter.h"

FActivateGameplayAbilityByClassStateTreeTask::FActivateGameplayAbilityByClassStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FActivateGameplayAbilityByClassStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.OwnerCharacter);
#endif

	UAbilitySystemComponent* AbilitySystemComponent = InstanceData.OwnerCharacter->GetAbilitySystemComponent();

	// ASC could be invalid if the character doesn't have a PlayerState yet
	if (!IsValid(AbilitySystemComponent))
	{
		return EStateTreeRunStatus::Failed;
	}

#if DO_ENSURE
	ensureAlways(IsValid(InstanceData.AbilityClass));
#endif

	// Try to activate the ability and return the status based on the result
	return AbilitySystemComponent->TryActivateAbilityByClass(InstanceData.AbilityClass) ?
		EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}