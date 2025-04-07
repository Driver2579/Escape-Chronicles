// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/SelectPrevItemGameplayAbility.h"

USelectPrevItemGameplayAbility::USelectPrevItemGameplayAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void USelectPrevItemGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	//...

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
