// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/SelectPrevItemGameplayAbility.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryManagerFragments/InventoryManagerSelectorFragment.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

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

	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor);

	if (!IsValid(Character))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	const AEscapeChroniclesPlayerState* PlayerState = Character->GetPlayerState<AEscapeChroniclesPlayerState>();

	if (!IsValid(PlayerState))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	const UInventoryManagerComponent* InventoryManagerComponent = PlayerState->GetInventoryManagerComponent();
	
	if (!IsValid(InventoryManagerComponent))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	UInventoryManagerSelectorFragment* InventoryManagerSelectorFragment = 
		InventoryManagerComponent->GetFragmentByClass<UInventoryManagerSelectorFragment>();

	if (!IsValid(InventoryManagerSelectorFragment))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	InventoryManagerSelectorFragment->SelectPrevItem();

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
