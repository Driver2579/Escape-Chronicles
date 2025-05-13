// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AbilityComponents/EndWhenInputReleasedGameplayAbilityComponent.h"

void UEndWhenInputReleasedGameplayAbilityComponent::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                                                  const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	
	CallEndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, false);
}
