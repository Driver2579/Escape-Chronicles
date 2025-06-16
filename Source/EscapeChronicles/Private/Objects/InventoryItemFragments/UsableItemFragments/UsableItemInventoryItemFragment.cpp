// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemFragments/UsableItemFragments/UsableItemInventoryItemFragment.h"

#include "ActorComponents/InventoryManagerComponent.h"

bool UUsableItemInventoryItemFragment::TryUseItem(UInventoryManagerComponent* InventoryManager,
	UInventoryItemInstance* ItemInstance, const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const
{
#if DO_CHECK
	check(IsValid(InventoryManager));
	check(IsValid(ItemInstance));
#endif

	// Try to use an item. Return false if we failed.
	if (!OnTriedToUseItem(InventoryManager, ItemInstance, SlotTypeTag, SlotIndex))
	{
		return false;
	}

	// Remove an item from the inventory and ensure it was removed successfully
	ensureAlways(InventoryManager->DeleteItem(SlotIndex, SlotTypeTag));

	// All good, the item was successfully used and removed from the inventory
	return true;
}