// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryManagerFragment.h"
#include "InventoryManagerUseItemsFragment.generated.h"

/**
 * A fragment for the inventory manager that adds support for using items that are equipped in the inventory and that
 * have a child fragment of the UUsableItemInventoryItemFragment.
 */
UCLASS()
class ESCAPECHRONICLES_API UInventoryManagerUseItemsFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	/**
	 * Attempts to use an item that is currently selected in the inventory. An inventory manager must have a
	 * UInventoryManagerSelectorFragment for this function to work properly.
	 * @return Whether the item was used successfully.
	 */
	bool TryToUseSelectedItem() const;

	/**
	 * Attempts to use an item at the specified slot type and index.
	 * @return Whether the item was used successfully.
	 */
	bool TryToUseItem(const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const;
};