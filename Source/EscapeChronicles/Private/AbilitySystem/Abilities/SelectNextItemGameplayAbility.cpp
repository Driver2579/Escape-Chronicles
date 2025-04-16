// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/SelectNextItemGameplayAbility.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryManagerFragments/InventoryManagerSelectorFragment.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

class UInventoryManagerSelectorFragment;

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

	InventoryManagerSelectorFragment->SelectNextItem();

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
