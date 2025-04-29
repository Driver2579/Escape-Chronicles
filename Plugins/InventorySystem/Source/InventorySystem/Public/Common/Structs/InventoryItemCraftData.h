// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventoryItemCraftData.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItemCraftData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName TestName;
};