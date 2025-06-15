// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventorySlotsSaveData.h"
#include "PickupItemSaveData.generated.h"

// Represents the save data of a pickup item in the world
USTRUCT()
struct FPickupItemSaveData
{
	GENERATED_BODY()

	// A class of the pickup item that was saved
	UPROPERTY(SaveGame)
	TSoftClassPtr<AEscapeChroniclesInventoryPickupItem> PickupItemClass;

	// Transform of the pickup item that was saved
	UPROPERTY(SaveGame)
	FTransform PickupTransform;

	// Data of the pickup item's item instance that was saved
	UPROPERTY(SaveGame)
	FInventoryTypedSlotSaveData ItemInstanceSaveData;
};