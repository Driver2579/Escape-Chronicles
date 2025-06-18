// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Net/Serialization/FastArraySerializer.h"
#include "Objects/InventoryItemInstance.h"
#include "InventorySlotsArray.generated.h"

struct FInventorySlotsTypedArray;
struct FInventorySlotsArray;

// A single slot in an inventory
USTRUCT()
struct FInventorySlot : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventorySlot() = default;

	explicit FInventorySlot(const FInventorySlotsArray* InInventorySlotsArray)
		: InventorySlotsArray(InInventorySlotsArray)
	{}

	const FInventorySlotsArray* GetInventorySlotsArray() const { return InventorySlotsArray; }

	UPROPERTY(Transient)
	TObjectPtr<UInventoryItemInstance> Instance = nullptr;

	void SetInventorySlotsArray(const FInventorySlotsArray* InInventorySlotsArray)
	{
		InventorySlotsArray = InInventorySlotsArray;
	}

private:
	const FInventorySlotsArray* InventorySlotsArray;
};

// Array of inventory slots
USTRUCT()
struct FInventorySlotsArray : public FFastArraySerializer
{
	GENERATED_BODY()

	const FInventorySlotsTypedArray* GetInventorySlotsTypedArray() const { return InventorySlotsTypedArray; }

	// Creates the specified number of empty slots and replicates them
	void Construct(const FInventorySlotsTypedArray* InInventorySlotsTypedArray, const int32 InSlotsNumber)
	{
		InventorySlotsTypedArray = InInventorySlotsTypedArray;

		Slots.Init(FInventorySlot(this), InSlotsNumber);

		MarkArrayDirty();
	}

	const TArray<FInventorySlot>& GetItems() const{ return Slots; }

	const FInventorySlot& operator[](const int32 Index) const { return Slots[Index]; }

	UInventoryItemInstance* GetInstance(const int32 Index) const
	{
		if (Slots.IsValidIndex(Index))
		{
			return Slots[Index].Instance;
		}

		return nullptr;
	}

	void SetInstance(UInventoryItemInstance* Instance, const int32 Index)
	{
		Slots[Index].Instance = Instance;
		MarkItemDirty(Slots[Index]);
	}

	int32 IndexOfByTag(const FInventorySlot& InInventorySlot) const
	{
		return Slots.IndexOfByPredicate([InInventorySlot](const FInventorySlot& InventorySlot)
			{
				return &InventorySlot == &InInventorySlot;
			});
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

	void UpdateOwningRefs(const FInventorySlotsTypedArray* InInventorySlotsTypedArray)
	{
		InventorySlotsTypedArray = InInventorySlotsTypedArray;

		for (FInventorySlot& Slot : Slots)
		{
			Slot.SetInventorySlotsArray(this);
		}
	}

private:
	UPROPERTY(Transient)
	TArray<FInventorySlot> Slots;

	const FInventorySlotsTypedArray* InventorySlotsTypedArray = nullptr;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotsArray> : TStructOpsTypeTraitsBase2<FInventorySlotsArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};