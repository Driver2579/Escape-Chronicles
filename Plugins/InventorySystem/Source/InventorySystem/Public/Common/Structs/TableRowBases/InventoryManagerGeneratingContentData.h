// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Common/Structs/FastArraySerializers/InstanceStats.h"

#include "InventoryManagerGeneratingContentData.generated.h"

class UInventoryItemDefinition;

// Defines loot generation rules for inventory
USTRUCT(BlueprintType)
struct FInventoryManagerGeneratingContentData : public FTableRowBase
{
	GENERATED_BODY()

	// Base item definition to spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UInventoryItemDefinition> ItemDefinition;

	// Initial stat overrides for spawned instances
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FInstanceStats InstanceStatsOverride;

	// Chance to spawn this entry (0-1 range)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
	float Probability = 1.0f;

	// Minimum quantity to spawn (inclusive)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="1"))
	int32 MinNumber = 1;

	// Maximum quantity to spawn (inclusive)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="1"))
	int32 MaxNumber = 1;
};
