// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemFragment.generated.h"

class UInventoryItemInstance;

// Base class for item's fragments that can describe various logic for it
UCLASS(Abstract, EditInlineNew)
class INVENTORYSYSTEM_API UInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnItemInstanceInitialized(UInventoryItemInstance* Instance) {}
};