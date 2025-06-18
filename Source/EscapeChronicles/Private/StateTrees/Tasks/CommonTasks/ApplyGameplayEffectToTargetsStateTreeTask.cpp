// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/ApplyGameplayEffectToTargetsStateTreeTask.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "StateTreeExecutionContext.h"

FApplyGameplayEffectToTargetsStateTreeTask::FApplyGameplayEffectToTargetsStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FApplyGameplayEffectToTargetsStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!ensureAlways(IsValid(InstanceData.GameplayEffectClass)))
	{
		return EStateTreeRunStatus::Failed;
	}

	bool bAppliedAtLeastOneGameplayEffect = false;

	for (const AActor* Target : InstanceData.Targets)
	{
		if (!IsValid(Target))
		{
#if !NO_LOGGING
			UE_LOG(LogTemp, Warning,
				TEXT("Invalid target actor provided in ApplyGameplayEffectToTargetsStateTreeTask!"));
#endif

			continue;
		}

		UAbilitySystemComponent* TargetAbilitySystemComponent =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);

		// Skip targets that don't have a valid Ability System Component or that match the ignored actors tag query
		const bool bCanApplyGameplayEffect =
			ensureAlwaysMsgf(IsValid(TargetAbilitySystemComponent),
				TEXT("Provided Target doesn't have a valid Ability System Component!")) &&
				TargetAbilitySystemComponent->MatchesGameplayTagQuery(InstanceData.IgnoredActorsTagQuery);

		if (!bCanApplyGameplayEffect)
		{
			continue;
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

		if (!bAppliedAtLeastOneGameplayEffect && AppliedGameplayEffectHandle.IsValid())
		{
			bAppliedAtLeastOneGameplayEffect = true;
		}
	}

	// Return Succeeded or Failed based on whether the gameplay effect was applied successfully to at least one target
	return bAppliedAtLeastOneGameplayEffect ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}