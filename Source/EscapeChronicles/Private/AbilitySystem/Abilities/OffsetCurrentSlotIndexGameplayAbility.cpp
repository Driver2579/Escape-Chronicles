// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/OffsetCurrentSlotIndexGameplayAbility.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryManagerFragments/InventoryManagerSelectorFragment.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

UOffsetCurrentSlotIndexGameplayAbility::UOffsetCurrentSlotIndexGameplayAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UOffsetCurrentSlotIndexGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	const AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor.Get(),
		ECastCheckedType::NullAllowed);
	
	if (!ensureAlways(IsValid(Character)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	const UInventoryManagerComponent* InventoryManagerComponent = Character->GetInventoryManagerComponent();
	
	if (!ensureAlways(IsValid(InventoryManagerComponent)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	UInventoryManagerSelectorFragment* InventoryManagerSelectorFragment = 
		InventoryManagerComponent->GetFragmentByClass<UInventoryManagerSelectorFragment>();

	if (!ensureAlways(IsValid(InventoryManagerSelectorFragment)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	InventoryManagerSelectorFragment->Server_OffsetCurrentSlotIndex(Offset);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
