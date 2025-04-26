// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/InteractGameplayAbility.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"

UInteractGameplayAbility::UInteractGameplayAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UInteractGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor);

	if (!IsValid(Character))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}
	
	UInteractionManagerComponent* InteractionManagerComponent = Character->GetInteractionManagerComponent();

	const bool IsInteractionCorrect = InteractionManagerComponent->TryInteract();

	if (!IsInteractionCorrect)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
