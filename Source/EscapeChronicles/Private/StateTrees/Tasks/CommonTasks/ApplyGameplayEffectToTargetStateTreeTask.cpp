// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/ApplyGameplayEffectToTargetStateTreeTask.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "StateTreeExecutionContext.h"

FApplyGameplayEffectToTargetStateTreeTask::FApplyGameplayEffectToTargetStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FApplyGameplayEffectToTargetStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!ensureAlways(IsValid(InstanceData.GameplayEffectClass)))
	{
		return EStateTreeRunStatus::Failed;
	}

#if DO_ENSURE
	ensureAlways(InstanceData.Target);
#endif

	UAbilitySystemComponent* TargetAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(
		InstanceData.Target);

	const bool bValidTargetASC = ensureAlwaysMsgf(IsValid(TargetAbilitySystemComponent),
		TEXT("Provided Target doesn't have a valid Ability System Component!"));

	if (!bValidTargetASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	FActiveGameplayEffectHandle AppliedGameplayEffectHandle;

	// If an instigator is provided, use it to apply the gameplay effect
	if (InstanceData.Instigator)
	{
		UAbilitySystemComponent* InstigatorAbilitySystemComponent =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.Instigator);

		const bool bValidInstigatorASC = ensureAlwaysMsgf(IsValid(InstigatorAbilitySystemComponent),
			TEXT("Provided Instigator doesn't have a valid Ability System Component!"));

		if (bValidInstigatorASC)
		{
			AppliedGameplayEffectHandle = InstigatorAbilitySystemComponent->ApplyGameplayEffectToTarget(
				InstanceData.GameplayEffectClass.GetDefaultObject(), TargetAbilitySystemComponent);
		}
	}
	// Otherwise, the target applies the gameplay effect to itself
	else
	{
		AppliedGameplayEffectHandle = TargetAbilitySystemComponent->ApplyGameplayEffectToSelf(
			InstanceData.GameplayEffectClass.GetDefaultObject(), UGameplayEffect::INVALID_LEVEL,
			TargetAbilitySystemComponent->MakeEffectContext());
	}

	// Return Succeeded or Failed based on whether the gameplay effect was applied successfully
	return AppliedGameplayEffectHandle.IsValid() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}