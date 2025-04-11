// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Containers/Union.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "FragmentationLocalData.generated.h"

using FFragmentationUnionLocalDataType = TUnion<bool, int32, float, FString>;

USTRUCT()
struct FFragmentationLocalDataItem : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FFragmentationLocalDataItem() {}

	FFragmentationLocalDataItem(const FGameplayTag InName)
	{
		Name = InName;
	}
	
	FFragmentationLocalDataItem(const FGameplayTag InName, const FFragmentationUnionLocalDataType& InValue)
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

	bool operator==(const FFragmentationLocalDataItem& Other) const
	{
		return Name == Other.Name;
	}

	FGameplayTag Name;
	FFragmentationUnionLocalDataType Value;
};

template<>
struct TStructOpsTypeTraits<FFragmentationLocalDataItem> : public TStructOpsTypeTraitsBase2<FFragmentationLocalDataItem>
{
	enum 
	{
		WithNetSerializer = true,
	};
};

USTRUCT()
struct FFragmentationLocalData : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	const TArray<FFragmentationLocalDataItem>& GetAllData() const
	{
		return Array;
	}
	
	FFragmentationLocalDataItem* GetData(const FGameplayTag InName)
	{
		return Array.FindByKey(InName);
	}
	
	void SetData(const FFragmentationLocalDataItem& InData)
	{
		if (FFragmentationLocalDataItem* Data = GetData(InData.Name); Data == nullptr)
		{
			const int32 Index = Array.Add(InData);
			MarkItemDirty(Array[Index]);
		}
		else if (Data->Value != InData.Value)
		{
			Data->Value = InData.Value;
			MarkItemDirty(*Data);
		}
	}

	template<typename T>
	void SetData(const FGameplayTag InName, const T& InValue) 
	{
		SetData({ InName, FFragmentationUnionLocalDataType(InValue) });
	}

	void RemoveData(const FGameplayTag InName)
	{
		Array.Remove({ InName });
		MarkArrayDirty();
	}
	
	bool HasData(const FGameplayTag InName)
	{
		return GetData(InName) != nullptr;
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FFragmentationLocalDataItem, FFragmentationLocalData>(
			Array, DeltaParms, *this);
	}

private:
	UPROPERTY()
	TArray<FFragmentationLocalDataItem> Array;
};

template<>
struct TStructOpsTypeTraits<FFragmentationLocalData> : public TStructOpsTypeTraitsBase2<FFragmentationLocalData>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};
