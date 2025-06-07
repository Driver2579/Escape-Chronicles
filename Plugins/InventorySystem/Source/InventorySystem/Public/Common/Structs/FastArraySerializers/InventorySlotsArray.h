// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Net/Serialization/FastArraySerializer.h"
#include "Objects/InventoryItemInstance.h"
#include "InventorySlotsArray.generated.h"

// A single slot in an inventory
USTRUCT()
struct FInventorySlot : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance = nullptr;
};

// Array of inventory slots
USTRUCT()
struct FInventorySlotsArray : public FFastArraySerializer
{
	GENERATED_BODY()

	// Creates the specified number of empty slots and replicates them
	void Construct(const int32 InSlotsNumber)
	{
		Slots.Init(FInventorySlot(), InSlotsNumber);

		MarkArrayDirty();
	}

	const TArray<FInventorySlot>& GetItems() const{ return Slots; }

	const FInventorySlot& operator[](const int32 Index) const { return Slots[Index]; }

	UInventoryItemInstance* GetInstance(const int32 Index) const { return Slots[Index].Instance; }

	void SetInstance(UInventoryItemInstance* Instance, const int32 Index)
	{
		Slots[Index].Instance = Instance;
		MarkItemDirty(Slots[Index]);
	}

	/**
	 * Finds first available empty slot in inventory
	 * @return Index of first empty slot, or INDEX_NONE if all slots are occupied
	 */
	int32 GetEmptySlotIndex() const
	{
		for (int32 Index = 0; Index < Slots.Num(); ++Index)
		{
			if (IsSlotEmpty(Index))
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}

	bool IsValidSlotIndex(const int32 Index) const
	{
		return Slots.IsValidIndex(Index);
	}

	bool IsSlotEmpty(const int32 Index) const
	{
		return !IsValid(Slots[Index].Instance);
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInventorySlot, FInventorySlotsArray>(Slots, DeltaParams, *this);
	}

private:
	UPROPERTY()
	TArray<FInventorySlot> Slots;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotsArray> : TStructOpsTypeTraitsBase2<FInventorySlotsArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};