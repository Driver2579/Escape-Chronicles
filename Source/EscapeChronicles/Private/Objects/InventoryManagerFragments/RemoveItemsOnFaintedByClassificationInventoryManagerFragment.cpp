// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/RemoveItemsOnFaintedByClassificationInventoryManagerFragment.h"

#include "EscapeChroniclesGameplayTags.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryItemFragments/ClassificationInventoryItemFragment.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void URemoveItemsOnFaintedByClassificationInventoryManagerFragment::OnManagerInitialized()
{
	Super::OnManagerInitialized();

#if DO_ENSURE
	ensureAlwaysMsgf(!ClassificationsToRemove.IsEmpty(),
		TEXT("There is no point in this fragment being added if ClassificationsToRemove is empty!"));
#endif

	const UInventoryManagerComponent* InventoryManagerComponent = GetInventoryManager();

#if DO_CHECK
	check(IsValid(InventoryManagerComponent));
#endif

	AEscapeChroniclesCharacter* OwningCharacter = InventoryManagerComponent->GetOwner<AEscapeChroniclesCharacter>();

	// This fragment should only be used on characters
	if (!ensureAlways(IsValid(OwningCharacter)))
	{
		return;
	}

	AEscapeChroniclesPlayerState* OwningPlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		OwningCharacter->GetPlayerState(), ECastCheckedType::NullAllowed);

	// Call OnOwningPlayerStateChanged event already if the PlayerState is already valid
	if (IsValid(OwningPlayerState))
	{
		OnOwningPlayerStateChanged(OwningPlayerState, nullptr);
	}

	// Listen to delegate for when the PlayerState is initialized or changed
	OwningCharacter->OnPlayerStateChangedDelegate.AddUObject(this, &ThisClass::OnOwningPlayerStateChanged);
}

void URemoveItemsOnFaintedByClassificationInventoryManagerFragment::OnOwningPlayerStateChanged(
	APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	const AEscapeChroniclesPlayerState* CastedOldPlayerState = Cast<AEscapeChroniclesPlayerState>(OldPlayerState);

	// Unregister the fainted event from the old PlayerState if it existed and if we ever registered it
	if (IsValid(CastedOldPlayerState))
	{
		CastedOldPlayerState->GetAbilitySystemComponent()->UnregisterGameplayTagEvent(
			OnFaintedGameplayTagChangedDelegateHandle, EscapeChroniclesGameplayTags::Status_Fainted);

		OnFaintedGameplayTagChangedDelegateHandle.Reset();
	}

	const AEscapeChroniclesPlayerState* CastedNewPlayerState = Cast<AEscapeChroniclesPlayerState>(NewPlayerState);

	// Subscribe to the fainted event on the new PlayerState if it exists
	if (IsValid(CastedNewPlayerState))
	{
		OnFaintedGameplayTagChangedDelegateHandle =
			CastedNewPlayerState->GetAbilitySystemComponent()->RegisterAndCallGameplayTagEvent(
				EscapeChroniclesGameplayTags::Status_Fainted,
				FOnGameplayEffectTagCountChanged::FDelegate::CreateUObject(this,
					&ThisClass::OnFaintedGameplayTagChanged));
	}
}

void URemoveItemsOnFaintedByClassificationInventoryManagerFragment::OnFaintedGameplayTagChanged(
	FGameplayTag GameplayTag, int32 NewCount) const
{
	const bool bFainted = NewCount > 0;

	// Don't do anything if the character isn't fainted
	if (!bFainted)
	{
		return;
	}

	UInventoryManagerComponent* InventoryManagerComponent = GetInventoryManager();

#if DO_CHECK
	check(IsValid(InventoryManagerComponent));
#endif

	// Iterate all slots to find items with that have a classification that we want to remove
	for (const FInventorySlotsTypedArray& TypedSlots : InventoryManagerComponent->GetInventoryContent().GetItems())
	{
		for (int32 i = 0; i < TypedSlots.Array.GetItems().Num(); ++i)
		{
			const FInventorySlot& TypedSlot = TypedSlots.Array.GetItems()[i];

			// Skip if the slot is empty
			if (!TypedSlot.Instance)
			{
				continue;
			}

			// Try to find the classification fragment in the item
			const UClassificationInventoryItemFragment* ClassificationItemFragment =
				TypedSlot.Instance->GetFragmentByClass<UClassificationInventoryItemFragment>();

			/**
			 * Check if the item has a classification fragment and if its classification is in the set of
			 * classifications we want to remove.
			 */
			const bool bCanRemoveItem = IsValid(ClassificationItemFragment) &&
				ClassificationsToRemove.Contains(ClassificationItemFragment->GetItemClassification());

			// Remove the item from the inventory if it has a classification that we want to remove
			if (bCanRemoveItem)
			{
				InventoryManagerComponent->DeleteItem(i, TypedSlots.TypeTag);
			}
		}
	}
}