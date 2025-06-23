// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InstanceStats.generated.h"

/**
 * Represents a single instance stat as a key-value pair in the instance stats container. It also supports being saved
 * to the file.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInstanceStatsItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInstanceStatsItem() = default;

	FInstanceStatsItem(const FGameplayTag& InTag)
	{
		Tag = InTag;
	}

	FInstanceStatsItem(const FGameplayTag& InTag, const float InValue)
	{
		Tag = InTag;
		Value = InValue;
	}

	bool operator==(const FInstanceStatsItem& Other) const
	{
		return Tag == Other.Tag;
	}

	// Unique key for this stat item
	UPROPERTY(EditAnywhere, SaveGame)
	FGameplayTag Tag;

	// Value stored by tag
	UPROPERTY(EditAnywhere, SaveGame)
	float Value = 0;
};

/**
 * Networked container for FInstanceStatsItem elements with efficient delta serialization. Provides storage and access
 * to stat using FInstanceStatsItem. It also supports being saved to the file.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInstanceStats : public FFastArraySerializer
{
	GENERATED_BODY()

	const TArray<FInstanceStatsItem>& GetAllStats() const
	{
		return Array;
	}

	const FInstanceStatsItem* GetStat(const FGameplayTag& InTag) const
	{
		return Array.FindByKey(InTag);
	}

	bool HasStat(const FGameplayTag& InTag) const
	{
		return GetStat(InTag) != nullptr;
	}

	void SetStat(const FInstanceStatsItem& InStat)
	{
		FInstanceStatsItem* Stat = Array.FindByKey(InStat.Tag);

		// Create new stat
		if (Stat == nullptr)
		{
			MarkItemDirty(Array[Array.Add(InStat)]);
		}
		// Rewrite the existing stat
		else if (Stat->Value != InStat.Value)
		{
			Stat->Value = InStat.Value;
			MarkItemDirty(*Stat);
		}
	}

	void SetStat(FGameplayTag Tag, float Value)
	{
		SetStat(FInstanceStatsItem(Tag, Value));
	}

	// Try to avoid calling this method as deleting an element completely leads to replication of the whole array
	void RemoveStat(const FGameplayTag& InTag)
	{
		const int32 Index = Array.IndexOfByKey(InTag);

		if (Index != INDEX_NONE)
		{
			Array.RemoveAt(Index);
			MarkArrayDirty();
		}
	}

	FInstanceStats& operator=(const FInstanceStats& Other)
	{
		Array = Other.Array;
		MarkArrayDirty();

		return *this;
	}

	bool HasMatchingStat(const FInstanceStatsItem& InStat) const
	{
		const FInstanceStatsItem* Stat = GetStat(InStat.Tag);

		return Stat && Stat->Value == InStat.Value;
	}

	bool HasAllMatchingStats(const FInstanceStats& InStats) const
	{
		for (const FInstanceStatsItem& InItem : InStats.GetAllStats())
		{
			if (!HasMatchingStat(InItem))
			{
				return false;
			}
		}

		return true;
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInstanceStatsItem, FInstanceStats>(Array, DeltaParams, *this);
	}

private:
	// Internal array storing all stat items 
	UPROPERTY(EditAnywhere, SaveGame)
	TArray<FInstanceStatsItem> Array;
};

template<>
struct TStructOpsTypeTraits<FInstanceStats> : TStructOpsTypeTraitsBase2<FInstanceStats>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};
