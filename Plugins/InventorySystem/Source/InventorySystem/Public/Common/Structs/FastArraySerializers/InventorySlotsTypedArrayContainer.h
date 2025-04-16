// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventorySlotsArray.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventorySlotsTypedArrayContainer.generated.h"

class UInventoryItemInstance;
struct FInventorySlotsArray;

// Array of inventory slots with type
USTRUCT()
struct FInventorySlotsTypedArray : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FGameplayTag Type;
	
	UPROPERTY()
	FInventorySlotsArray Array;

	bool operator==(const FGameplayTag& InType) const
	{
		return Type == InType;
	}

	bool operator==(const FInventorySlotsTypedArray& Other) const
	{
		return Type == Other.Type;
	}
};

// Contain arrays of slots by their types
USTRUCT()
struct FInventorySlotsTypedArrayContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	const TArray<FInventorySlotsTypedArray>& GetArrays() const
	{
		return Arrays;
	}

	const FInventorySlotsTypedArray& operator[](const size_t Index) const
	{
		return Arrays[Index];
	}
	
	void SetInstance(UInventoryItemInstance* Instance, const size_t ArrayIndex, const size_t SlotIndex)
	{
		Arrays[ArrayIndex].Array.SetInstance(Instance, SlotIndex);
		MarkItemDirty(Arrays[ArrayIndex]);
	}

	size_t IndexOfByTag(const FGameplayTag Type) const
	{
		return Arrays.IndexOfByPredicate(
			[Type](const FInventorySlotsTypedArray& List)
			{
				return List.Type == Type;
			});
	}
	
	/**
	* @tparam FGameplayTag Inventory slots type;
	* @tparam int32 Number of slots;
	 */
	void Initialize(const TMap<FGameplayTag, int32>& InitializationData)
	{
		for (const auto& Pair : InitializationData)
		{
			FInventorySlotsTypedArray InventorySlotsTypedArray = FInventorySlotsTypedArray();
			InventorySlotsTypedArray.Type = Pair.Key;
			InventorySlotsTypedArray.Array.Initialize(Pair.Value);
			
			Arrays.Add(InventorySlotsTypedArray);
		}

		MarkArrayDirty();
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FastArrayDeltaSerialize<FInventorySlotsTypedArray,
			FInventorySlotsTypedArrayContainer>(Arrays, DeltaParms, *this);
	}

private:
	UPROPERTY()
	TArray<FInventorySlotsTypedArray> Arrays;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotsTypedArrayContainer>
	: TStructOpsTypeTraitsBase2<FInventorySlotsTypedArrayContainer>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};