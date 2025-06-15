// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerUseItemsFragment.h"

#include "Objects/InventoryItemFragments/UsableItemFragments/UsableItemInventoryItemFragment.h"
#include "Objects/InventoryManagerFragments/InventoryManagerSelectorFragment.h"

bool UInventoryManagerUseItemsFragment::TryToUseSelectedItem() const
{
	const UInventoryManagerComponent* InventoryManagerComponent = GetInventoryManager();

#if DO_CHECK
	check(IsValid(InventoryManagerComponent));
#endif

	// Get the selector fragment from the inventory manager component to get the currently selected item slot
	const UInventoryManagerSelectorFragment* SelectorFragment = InventoryManagerComponent->GetFragmentByClass<
		UInventoryManagerSelectorFragment>();

	// We can't find a selected item slot if the InventoryManagerComponent doesn't have a selector fragment
	if (!IsValid(SelectorFragment))
	{
#if !NO_LOGGING
		UE_LOG(LogTemp, Warning,
			TEXT("UInventoryManagerUseItemsFragment::TryToUseSelectedItem: Unable to use an item because a "
				"UInventoryManagerComponent of actor %s doesn't have a UInventoryManagerSelectorFragment"),
			*InventoryManagerComponent->GetOwner()->GetName());
#endif

		return false;
	}

	// Try to use the item at the currently selected slot type and index
	return TryToUseItem(SelectorFragment->GetSelectableSlotsTypeTag(), SelectorFragment->GetCurrentSlotIndex());
}

bool UInventoryManagerUseItemsFragment::TryToUseItem(const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const
{
	UInventoryManagerComponent* InventoryManagerComponent = GetInventoryManager();

#if DO_CHECK
	check(IsValid(InventoryManagerComponent));
#endif

	UInventoryItemInstance* ItemInstance = InventoryManagerComponent->GetItemInstance(SlotIndex, SlotTypeTag);

	// We can't use an item if the given slot is empty or invalid
	if (!IsValid(ItemInstance))
	{
		return false;
	}

	const UUsableItemInventoryItemFragment* UsableItemFragment = ItemInstance->GetFragmentByClass<
		UUsableItemInventoryItemFragment>();

	// We can't use an item if it doesn't have a usable item fragment
	if (!IsValid(UsableItemFragment))
	{
		return false;
	}

	// Try to use the given item instance at the given slot and return the result
	return UsableItemFragment->TryUseItem(InventoryManagerComponent, ItemInstance, SlotTypeTag, SlotIndex);
}