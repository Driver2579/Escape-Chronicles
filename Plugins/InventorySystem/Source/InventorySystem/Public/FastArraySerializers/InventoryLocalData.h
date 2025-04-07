// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Containers/Union.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryLocalData.generated.h"

using FInventoryUnionLocalDataType = TUnion<bool, int32, float, FString>;

USTRUCT()
struct FInventoryLocalDataItem : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FGameplayTag GetName() const
	{
		return Name;
	}

	template<typename T>
	T GetData() const
	{
		return Data.GetSubtype<T>();
	}

	template<typename T>
	void SetData(T InData)
	{
		Data.SetSubtype<T>(InData);
	}
	
	FInventoryLocalDataItem() {}

	template<typename T>
	FInventoryLocalDataItem(const FGameplayTag InName, T InData)
	{
		Name = InName;
		Data.SetSubtype<T>(InData);
	}
	
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << Name;
		Ar << Data;
		
		bOutSuccess = true;
		return true;
	}

private:
	FGameplayTag Name;
	FInventoryUnionLocalDataType Data;
};

template<>
struct TStructOpsTypeTraits<FInventoryLocalDataItem> : public TStructOpsTypeTraitsBase2<FInventoryLocalDataItem>
{
	enum 
	{
		WithNetSerializer = true,
	};
};

USTRUCT()
struct FInventoryLocalData : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	template<typename T>
	FInventoryLocalDataItem* FindByName(const FGameplayTag InName)
	{
		return Array.FindByPredicate([InName](const FInventoryLocalDataItem& Item)
		{
			return Item.GetName() == InName;
		});
	}
	
	template<typename T>
	void SetData(const FGameplayTag InName, T InData)
	{
		FInventoryLocalDataItem* Data = FindByName<T>(InName);

		if (Data == nullptr)
		{
			const int32 Index = Array.Add(FInventoryLocalDataItem(InName, InData));
			MarkItemDirty(Array[Index]);
			return;
		}
		else if (Data->GetData<T>() == InData)
		{
			return;
		}
		
		Data->SetData(InData);
		MarkItemDirty(*Data);
	}

	template<typename T>
	void RemoveData(const FGameplayTag InName)
	{
		const FInventoryLocalDataItem* Data = FindByName<T>(InName);

		if (Data == nullptr)
		{
			return;
		}
		
		Array.Remove(*Data);
		MarkArrayDirty();
	}
	
	template<typename T>
	bool HasData(const FGameplayTag InName)
	{
		return FindByName<T>(InName) != nullptr;
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryLocalDataItem, FInventoryLocalData>(Array,
			DeltaParms, *this);
	}

private:
	UPROPERTY()
	TArray<FInventoryLocalDataItem> Array;
};

template<>
struct TStructOpsTypeTraits<FInventoryLocalData> : public TStructOpsTypeTraitsBase2<FInventoryLocalData>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};
