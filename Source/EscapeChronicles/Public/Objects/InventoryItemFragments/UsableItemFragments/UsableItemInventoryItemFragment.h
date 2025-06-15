// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryItemFragment.h"
#include "UsableItemInventoryItemFragment.generated.h"

// Base class for items' fragments that allow an item to be used by the player
UCLASS(Abstract, Hidden)
class ESCAPECHRONICLES_API UUsableItemInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	/**
	 * Attempts to use an item. If the item was used successfully, it will be removed from the inventory.
	 * @return True if the item was used successfully, false otherwise.
	 */
	bool TryUseItem(UInventoryManagerComponent* InventoryManager, UInventoryItemInstance* ItemInstance,
		const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const;

protected:
	/**
	 * An implementation of the item usage logic. Unimplemented by default and must be overridden in a derived class
	 * without calling the parent function.
	 * @return Whether the item was used successfully. If true is returned, the item will be removed from the inventory.
	 */
	virtual bool OnTriedToUseItem(UInventoryManagerComponent* InventoryManager, UInventoryItemInstance* ItemInstance,
		const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const
	{
		unimplemented();

		return true;
	}
};