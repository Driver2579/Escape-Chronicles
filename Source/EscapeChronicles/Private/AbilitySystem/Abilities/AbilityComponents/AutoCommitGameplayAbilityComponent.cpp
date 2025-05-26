// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/AbilityComponents/AutoCommitGameplayAbilityComponent.h"

void UAutoCommitGameplayAbilityComponent::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

#if DO_CHECK
	check(IsValid(GetOwner()));
#endif

	GetOwner()->CommitAbility(Handle, ActorInfo, ActivationInfo);
}
