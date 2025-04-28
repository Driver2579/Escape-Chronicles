// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Net/Serialization/FastArraySerializer.h"
#include "Objects/InventoryItemInstance.h"
#include "InventorySlotsArray.generated.h"

// A single slot in an inventory
USTRUCT()
struct FInventorySlot : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance = nullptr;
};

// Array of inventory slots
USTRUCT()
struct FInventorySlotsArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	const TArray<FInventorySlot>& GetSlots() const
	{
		return Slots;
	}
	
	const FInventorySlot& operator[](const int32 Index) const
	{
		return Slots[Index];
	}

	UInventoryItemInstance* GetInstance(const int32 Index) const
	{
		return Slots[Index].Instance;
	}
	
	void SetInstance(UInventoryItemInstance* Instance, const int32 Index)
	{
		Slots[Index].Instance = Instance;
		MarkItemDirty(Slots[Index]);
	}

	int32 GetEmptySlotIndex() const
	{
		for (int32 i = 0; i < Slots.Num(); ++i)
		{
			if (IsEmptySlot(i))
			{
				return i;
			}
		}
		
		return -1;
	}

	bool IsValidSlotIndex(const int32 Index) const
	{
		return Index >= 0 && Index <= Slots.Num() - 1;
	}
	
	bool IsEmptySlot(const int32 Index) const
	{
		return !IsValid(Slots[Index].Instance);
	}
	
	void Initialize(const int32 InSlotsNumber)
	{
		Slots.Init(FInventorySlot(), InSlotsNumber);
		MarkArrayDirty();
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FastArrayDeltaSerialize<FInventorySlot, FInventorySlotsArray>(Slots, DeltaParms, *this);
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