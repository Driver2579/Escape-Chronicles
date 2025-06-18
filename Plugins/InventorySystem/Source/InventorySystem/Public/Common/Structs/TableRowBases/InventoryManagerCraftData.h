// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Common/Structs/FastArraySerializers/InstanceStats.h"
#include "InventoryManagerCraftData.generated.h"

class UInventoryItemDefinition;

USTRUCT(BlueprintType)
struct FCraftRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UInventoryItemDefinition> ItemDefinitionClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FInstanceStats InstanceStatsOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Number = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bConsumeResource = true;
};

USTRUCT(BlueprintType)
struct FInventoryManagerCraftData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UInventoryItemDefinition> ResultItemDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FInstanceStats ResultInstanceStatsOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ResultNumber = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FCraftRequirement> MaterialsItemDefinition;
};
