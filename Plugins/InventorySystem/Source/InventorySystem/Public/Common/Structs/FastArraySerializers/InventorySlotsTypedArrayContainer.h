// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventorySlotsArray.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventorySlotsTypedArrayContainer.generated.h"

class UInventoryItemInstance;
class UInventoryManagerComponent;

struct FInventorySlotsArray;
struct FInventorySlotsTypedArrayContainer;

// Typifies an array of slots with tag
USTRUCT()
struct FInventorySlotsTypedArray : public FFastArraySerializerItem
{
	GENERATED_BODY()

	void Construct(const FInventorySlotsTypedArrayContainer* InInventorySlotsTypedArrayContainer,
		const FGameplayTag& InTypeTag, const int32 InSlotsNumber)
	{
		InventorySlotsTypedArrayContainer = InInventorySlotsTypedArrayContainer;

		TypeTag = InTypeTag;

		Array.Construct(this, InSlotsNumber);
	}

	const FInventorySlotsTypedArrayContainer* GetInventorySlotsTypedArrayContainer() const
	{
		return InventorySlotsTypedArrayContainer;
	}

	UPROPERTY(Transient)
	FGameplayTag TypeTag;

	UPROPERTY(Transient)
	FInventorySlotsArray Array;

	bool operator==(const FGameplayTag& InTypeTag) const
	{
		return TypeTag == InTypeTag;
	}

	bool operator==(const FInventorySlotsTypedArray& Other) const
	{
		return TypeTag == Other.TypeTag;
	}

private:
	const FInventorySlotsTypedArrayContainer* InventorySlotsTypedArrayContainer = nullptr;
};

// Contain arrays of slots by their types
USTRUCT()
struct FInventorySlotsTypedArrayContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	UInventoryManagerComponent* GetInventoryManagerComponent() const { return InventoryManagerComponent; }

	/**
	 * Initializes inventory slots from configuration data.
	 * @tparam KeyType Tag of the slot's type.
	 * @tparam ValueType Number of slots.
	 */
	void Construct(UInventoryManagerComponent* InInventoryManagerComponent,
		const TMap<FGameplayTag, int32>& InitializationData)
	{
		InventoryManagerComponent = InInventoryManagerComponent;

		Arrays.Empty();

		for (const TPair<FGameplayTag, int32>& Pair : InitializationData)
		{
			const int32 Index = Arrays.Add(FInventorySlotsTypedArray());

			Arrays[Index].Construct(this, Pair.Key, Pair.Value);
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

	int32 IndexOfByTag(const FGameplayTag& TypeTag) const
	{
		return Arrays.IndexOfByPredicate([TypeTag](const FInventorySlotsTypedArray& List)
			{
				return List.TypeTag == TypeTag;
			});
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInventorySlotsTypedArray, FInventorySlotsTypedArrayContainer>(Arrays,
			DeltaParams, *this);
	}

private:
	// Arrays of slots by their types
	UPROPERTY(Transient)
	TArray<FInventorySlotsTypedArray> Arrays;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryManagerComponent> InventoryManagerComponent;
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