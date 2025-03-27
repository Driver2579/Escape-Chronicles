// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/UInteractGameplayAbility.h"

#include "Components/ActorComponents/InteractionManagerComponent.h"

UUInteractGameplayAbility::UUInteractGameplayAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UUInteractGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		
		return;
	}
	
	UInteractionManagerComponent* InteractionManagerComponent = ActorInfo->AvatarActor
		->FindComponentByClass<UInteractionManagerComponent>();
	if (!IsValid(InteractionManagerComponent))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		return;
	}
	
	InteractionManagerComponent->TryInteract();
}
