// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InstanceStats.generated.h"

// Represents a single instance stat as a key-value pair in the instance stats container
USTRUCT(BlueprintType)
struct FInstanceStatsItem : public FFastArraySerializerItem
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
	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;

	// Value stored by tag
	UPROPERTY(EditAnywhere)
	float Value = 0;
};

/**
 * Networked container for FInstanceStatsItem elements with efficient delta serialization. Provides storage and access
 * to stat using FInstanceStatsItem.
 */
USTRUCT(BlueprintType)
struct FInstanceStats : public FFastArraySerializer
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

	bool HasStat(const FGameplayTag& InTag) const
	{
		return GetStat(InTag) != nullptr;
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInstanceStatsItem, FInstanceStats>(Array, DeltaParams, *this);
	}

private:
	// Internal array storing all stat items 
	UPROPERTY(EditAnywhere)
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
