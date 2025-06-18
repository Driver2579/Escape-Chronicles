// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerLimitItemAllowedSlotsByTypeFragment.h"

bool UInventoryManagerLimitItemAllowedSlotsByTypeFragment::CanEquipItemInSlot(const UInventoryItemInstance* Item,
	const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const
{
	if (!Super::CanEquipItemInSlot(Item, SlotTypeTag, SlotIndex))
	{
		return false;
	}

	const FInventoryItemFragmentsClassesContainer* AllowedDefinitionsContainer = AllowedItemFragmentsForSlotTypes.Find(
		SlotTypeTag);

	// If the slot type isn't in the map, allow all items being added to it
	if (!AllowedDefinitionsContainer)
	{
		return true;
	}

#if DO_ENSURE
	ensureAlways(!AllowedDefinitionsContainer->ItemFragmentsClasses.IsEmpty());
#endif

	// Return false if the item doesn't have all the required fragments for this slot type
	for (const TSoftClassPtr<UInventoryItemFragment>& ItemFragmentClass :
		AllowedDefinitionsContainer->ItemFragmentsClasses)
	{
		/**
		 * If the item fragment class is invalid, then the item for sure doesn't have the fragment of this class because
		 * it isn't event loaded.
		 */
		if (!ItemFragmentClass.IsValid() || IsValid(Item->GetFragmentByClass(ItemFragmentClass.Get())))
		{
			return false;
		}
	}

	// If the item has all the required fragments, allow it to be added to the slot
	return true;
}