// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/EndWhenInputReleasedGameplayAbility.h"

void UEndWhenInputReleasedGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
