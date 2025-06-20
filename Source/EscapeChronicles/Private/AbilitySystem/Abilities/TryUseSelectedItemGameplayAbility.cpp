// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/TryUseSelectedItemGameplayAbility.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryManagerFragments/InventoryManagerUseItemsFragment.h"

UTryUseSelectedItemGameplayAbility::UTryUseSelectedItemGameplayAbility()
{
	// This ability could work only on the server. Client can only request it to be executed.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UTryUseSelectedItemGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Cancel ability if it can't be committed
	if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);

		return;
	}

	const AEscapeChroniclesCharacter* OwningCharacter = CastChecked<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor,
		ECastCheckedType::NullAllowed);

	if (!ensureAlways(IsValid(OwningCharacter)))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);

		return;
	}

	const UInventoryManagerUseItemsFragment* UseItemsInventoryFragment =
		OwningCharacter->GetInventoryManagerComponent()->GetFragmentByClass<UInventoryManagerUseItemsFragment>();

	/**
	 * An InventoryManagerComponent of the owning character must have a UInventoryManagerUseItemsFragment to use this
	 * ability.
	 */
	const bool bUseItemsInventoryFragmentValid = ensureAlwaysMsgf(IsValid(UseItemsInventoryFragment),
		TEXT("An inventory must have a UInventoryManagerUseItemsFragment!"));

	if (!bUseItemsInventoryFragmentValid)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);

		return;
	}

	// Try to use the selected item in the inventory and commit the ability if the usage was successful
	if (UseItemsInventoryFragment->TryToUseSelectedItem())
	{
		CommitAbility(Handle, ActorInfo, ActivationInfo);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}