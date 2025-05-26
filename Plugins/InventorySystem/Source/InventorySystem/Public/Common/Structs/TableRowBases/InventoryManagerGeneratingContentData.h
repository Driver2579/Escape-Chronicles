// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Common/Structs/FastArraySerializers/LocalData.h"

#include "InventoryManagerGeneratingContentData.generated.h"

class UInventoryItemDefinition;

USTRUCT(BlueprintType)
struct FInventoryManagerGeneratingContentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UInventoryItemDefinition> ItemDefinition;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLocalData LocalDataOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
	float Probability = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="1"))
	int32 MinNumber = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="1"))
	int32 MaxNumber = 1;
};