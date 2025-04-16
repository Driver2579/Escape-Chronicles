// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Containers/Union.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LocalData.generated.h"

using FUnionLocalDataType = TUnion<bool, int32, float, FString>;

// Represents a single key-value pair in the local data container
USTRUCT()
struct FLocalDataItem : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FLocalDataItem() {}

	FLocalDataItem(const FGameplayTag InName)
	{
		Name = InName;
	}
	
	FLocalDataItem(const FGameplayTag InName, const FUnionLocalDataType& InValue)
	{
		Name = InName;
		Value = InValue;
	}
	
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << Name;
		Ar << Value;
		
		bOutSuccess = true;
		return true;
	}

	bool operator==(const FLocalDataItem& Other) const
	{
		return Name == Other.Name;
	}

	// Unique key for this data item
	FGameplayTag Name;

	// Union container holding the value 
	FUnionLocalDataType Value;
};

template<>
struct TStructOpsTypeTraits<FLocalDataItem> : TStructOpsTypeTraitsBase2<FLocalDataItem>
{
	enum 
	{
		WithNetSerializer = true,
	};
};

/**
 * Networked container for FLocalDataItem elements with efficient delta serialization.
 * Provides storage and access to various data types using Gameplay Tags.
 */
USTRUCT()
struct FLocalData : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	const TArray<FLocalDataItem>& GetAllData() const
	{
		return Array;
	}
	
	const FLocalDataItem* GetData(const FGameplayTag InName) const
	{
		return Array.FindByKey(InName);
	}
	
	void SetData(const FLocalDataItem& InData)
	{
		FLocalDataItem* Data = Array.FindByKey(InData.Name);
		
		if (Data == nullptr)
		{
			const size_t Index = Array.Add(InData);
			MarkItemDirty(Array[Index]);
		}
		else if (Data->Value != InData.Value)
		{
			Data->Value = InData.Value;
			MarkItemDirty(*Data);
		}
	}

	template<typename T>
	void SetData(const FGameplayTag InName, const T InValue) 
	{
		SetData({ InName, FUnionLocalDataType(InValue) });
	}

	void RemoveData(const FGameplayTag InName)
	{
		const size_t Index = Array.IndexOfByKey(InName);

		if (Index == INDEX_NONE)
		{
			return;
		}
		
		Array.RemoveAt(Index);
		MarkArrayDirty();
	}
	
	bool HasData(const FGameplayTag InName) const
	{
		return GetData(InName) != nullptr;
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FastArrayDeltaSerialize<FLocalDataItem, FLocalData>(
			Array, DeltaParms, *this);
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
