// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/EmptyGameplayAbility.h"


void UEmptyGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	if (bAutoEndAbility)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}
}
