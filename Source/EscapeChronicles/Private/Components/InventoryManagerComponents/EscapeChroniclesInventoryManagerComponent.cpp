// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InventoryManagerComponents/EscapeChroniclesInventoryManagerComponent.h"

void UEscapeChroniclesInventoryManagerComponent::OnPreSaveObject()
{
	// Empty the saved slots container before saving the new data
	SavedSlotsContainer.Slots.Empty();

	for (const FInventorySlotsTypedArray& Slots : GetInventoryContent().GetItems())
	{
		// Allocate the slots of the currently iterated type at the saved slots map
		FInventoryTypedSlotsSaveData& SavedSlots = SavedSlotsContainer.Slots.Add(Slots.TypeTag);

		// Number of slots in the currently iterated type
		const int32 TypedSlotsNumber = Slots.Array.GetItems().Num();

		// Reserve space for all slots of this type to avoid reallocations
		SavedSlots.TypedSlots.Reserve(TypedSlotsNumber);

		for (int32 i = 0; i < TypedSlotsNumber; ++i)
		{
			const FInventorySlot& TypedSlot = Slots.Array.GetItems()[i];

			// Allocate the slot in the saved slots container of the currently iterated type
			FInventoryTypedSlotSaveData& SavedTypedSlot = SavedSlots.TypedSlots.AddDefaulted_GetRef();

			// This slot is empty, so save it as empty and go to the next one
			if (!TypedSlot.Instance)
			{
				continue;
			}

			// Save the definition and the stats of the item in the slot
			SavedTypedSlot.DefinitionClass = TypedSlot.Instance->GetDefinition();
			SavedTypedSlot.InstanceStats = TypedSlot.Instance->GetInstanceStats_Mutable();
		}
	}

	// Remember that the inventory was saved at least once
	bWasEverSaved = true;
}

void UEscapeChroniclesInventoryManagerComponent::OnPostLoadObject()
{
	// Clear inventory because we are going to reconstruct it with the loaded data
	ClearInventory();

	// Reconstruct the inventory content from the loaded data. Iterate through each type of slots and each of its slots.
	for (const TPair<FGameplayTag, FInventoryTypedSlotsSaveData>& Pair : SavedSlotsContainer.Slots)
	{
		// Iterate through each slot of the currently iterated type
		for (int32 i = 0; i < Pair.Value.TypedSlots.Num(); ++i)
		{
			const FInventoryTypedSlotSaveData& TypedSlot = Pair.Value.TypedSlots[i];

			// Skip the loaded slot if it doesn't contain an item
			if (TypedSlot.DefinitionClass.IsNull())
			{
				continue;
			}

			// Construct a new item instance to fill it with the loaded data
			UInventoryItemInstance* ItemInstance = NewObject<UInventoryItemInstance>(this);

			// Set the loaded definition class and stats to the item instance
			ItemInstance->Initialize(TypedSlot.DefinitionClass.LoadSynchronous());
			ItemInstance->GetInstanceStats_Mutable() = TypedSlot.InstanceStats;

			// Add the item instance to the inventory
			AddItem(ItemInstance, i, Pair.Key);
		}
	}
}