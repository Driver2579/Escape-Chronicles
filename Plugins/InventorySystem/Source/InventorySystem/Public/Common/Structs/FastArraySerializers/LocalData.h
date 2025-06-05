// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LocalData.generated.h"

// Represents a single key-value pair in the local data container
USTRUCT(BlueprintType)
struct FLocalDataItem : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FLocalDataItem() {}

	FLocalDataItem(const FGameplayTag InTag)
	{
		Tag = InTag;
	}

	FLocalDataItem(const FGameplayTag& InTag, const float InValue)
	{
		Tag = InTag;
		Value = InValue;
	}

	bool operator==(const FLocalDataItem& Other) const
	{
		return Tag == Other.Tag;
	}

	// Unique key for this data item
	FGameplayTag Tag;

	// Value stored by tag
	float Value;
};

/**
 * Networked container for FLocalDataItem elements with efficient delta serialization. Provides storage and access to
 * data using FLocalDataItem.
 */
USTRUCT(BlueprintType)
struct FLocalData : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	const TArray<FLocalDataItem>& GetAllData() const
	{
		return Array;
	}

	const FLocalDataItem* GetData(const FGameplayTag InTag) const
	{
		return Array.FindByKey(InTag);
	}

	void SetData(const FLocalDataItem& InData)
	{
		FLocalDataItem* Data = Array.FindByKey(InData.Tag);

		// Create new data
		if (Data == nullptr)
		{
			MarkItemDirty(Array[Array.Add(InData)]);
		}
		// Rewrite the existing data
		else if (Data->Value != InData.Value)
		{
			Data->Value = InData.Value;
			MarkItemDirty(*Data);
		}
	}

	// Try to avoid calling this method as deleting an element completely leads to replication of the whole array
	void RemoveData(const FGameplayTag& InTag)
	{
		const int32 Index = Array.IndexOfByKey(InTag);

		if (Index != INDEX_NONE)
		{
			Array.RemoveAt(Index);
			MarkArrayDirty();
		}
	}

	bool HasData(const FGameplayTag& InTag) const
	{
		return GetData(InTag) != nullptr;
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FastArrayDeltaSerialize<FLocalDataItem, FLocalData>(Array, DeltaParms, *this);
	}

private:
	// Internal array storing all data items 
	UPROPERTY()
	TArray<FLocalDataItem> Array;
};

template<>
struct TStructOpsTypeTraits<FLocalData> : TStructOpsTypeTraitsBase2<FLocalData>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};
