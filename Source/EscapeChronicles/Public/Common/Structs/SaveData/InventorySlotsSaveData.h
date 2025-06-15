// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Common/Structs/FastArraySerializers/InstanceStats.h"
#include "InventorySlotsSaveData.generated.h"

class UInventoryItemDefinition;

/**
 * Represents the save data for one inventory slot, which contains information about the item placed in that slot but
 * doesn't contain information about the type of the slot itself.
 */
USTRUCT()
struct FInventoryTypedSlotSaveData
{
	GENERATED_BODY()

	// A class of the item that was placed in the slot
	UPROPERTY(SaveGame)
	TSoftClassPtr<UInventoryItemDefinition> DefinitionClass;

	// Stats of the item that was placed in the slot
	UPROPERTY(SaveGame)
	FInstanceStats InstanceStats;
};

// Represents the save data for a collection of inventory slots that have the same type
USTRUCT()
struct FInventoryTypedSlotsSaveData
{
	GENERATED_BODY()

	// Slots of the same type
	UPROPERTY(SaveGame)
	TArray<FInventoryTypedSlotSaveData> TypedSlots;
};

// Represents the save data for all inventory slots
USTRUCT()
struct FInventorySlotsSaveData
{
	GENERATED_BODY()

	/**
	 * @tparam KeyType A tag of the slots' type.
	 * @tparam ValueType A collection of slots that have the same type.
	 */
	UPROPERTY(SaveGame)
	TMap<FGameplayTag, FInventoryTypedSlotsSaveData> Slots;
};