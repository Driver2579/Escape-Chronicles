// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventorySlotsArray.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventorySlotsTypedArrayContainer.generated.h"

class UInventoryItemInstance;
struct FInventorySlotsArray;

// Typifies an array of slots with tag
USTRUCT()
struct FInventorySlotsTypedArray : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FGameplayTag TypeTag;

	UPROPERTY()
	FInventorySlotsArray Array;

	bool operator==(const FGameplayTag& InTypeTag) const
	{
		return TypeTag == InTypeTag;
	}

	bool operator==(const FInventorySlotsTypedArray& Other) const
	{
		return TypeTag == Other.TypeTag;
	}
};

// Contain arrays of slots by their types
USTRUCT()
struct FInventorySlotsTypedArrayContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	/**
	 * Creates arrays typed arrays of slots by Map.
	 * @tparam FGameplayTag Inventory slots type.
	 * @tparam int32 Number of slots.
	 */
	void Construct(const TMap<FGameplayTag, int32>& InitializationData)
	{
		Arrays.Empty();

		for (const TPair<FGameplayTag, int32>& Pair : InitializationData)
		{
			FInventorySlotsTypedArray InventorySlotsTypedArray = FInventorySlotsTypedArray();
			InventorySlotsTypedArray.TypeTag = Pair.Key;
			InventorySlotsTypedArray.Array.Construct(Pair.Value);
			
			Arrays.Add(InventorySlotsTypedArray);
		}

		MarkArrayDirty();
	}

	const TArray<FInventorySlotsTypedArray>& GetItems() const { return Arrays; }

	const FInventorySlotsTypedArray& operator[](const int32 Index) const { return Arrays[Index]; }

	UInventoryItemInstance* GetInstance(const int32 ArrayIndex, const int32 SlotIndex) const
	{
		return Arrays[ArrayIndex].Array.GetInstance(SlotIndex);
	}

	void SetInstance(UInventoryItemInstance* Instance, const int32 ArrayIndex, const int32 SlotIndex)
	{
		Arrays[ArrayIndex].Array.SetInstance(Instance, SlotIndex);
		MarkItemDirty(Arrays[ArrayIndex]);
	}

	int32 IndexOfByTag(const FGameplayTag TypeTag) const
	{
		return Arrays.IndexOfByPredicate([TypeTag](const FInventorySlotsTypedArray& List)
			{
				return List.TypeTag == TypeTag;
			});
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FastArrayDeltaSerialize<FInventorySlotsTypedArray, FInventorySlotsTypedArrayContainer>(Arrays,
			DeltaParms, *this);
	}

private:
	// Arrays of slots by their types
	UPROPERTY()
	TArray<FInventorySlotsTypedArray> Arrays;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotsTypedArrayContainer> :
	TStructOpsTypeTraitsBase2<FInventorySlotsTypedArrayContainer>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};