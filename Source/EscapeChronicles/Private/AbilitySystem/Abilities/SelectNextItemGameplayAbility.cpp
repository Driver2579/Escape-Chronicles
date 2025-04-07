// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/SelectNextItemGameplayAbility.h"

USelectNextItemGameplayAbility::USelectNextItemGameplayAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void USelectNextItemGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
