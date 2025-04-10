// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/SelectNextItemGameplayAbility.h"

#include "Components/ActorComponents/InventoryManagerComponent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

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

	const AEscapeChroniclesPlayerState* PlayerState = Cast<AEscapeChroniclesPlayerState>(ActorInfo->AvatarActor);
	if (!IsValid(PlayerState))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	const UInventoryManagerComponent* InteractionManagerComponent = PlayerState->GetInventoryManagerComponent();
	
	if (!IsValid(InteractionManagerComponent))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	//InteractionManagerComponent

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
