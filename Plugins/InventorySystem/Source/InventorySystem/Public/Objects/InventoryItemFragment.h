// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemFragment.generated.h"

class UInventoryItemInstance;

/**
 * Base class for creating fragments of an item that can describe various logic for it
 */
UCLASS(DefaultToInstanced, Abstract)
class INVENTORYSYSTEM_API UInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UInventoryItemInstance* Instance) {}
	virtual void OnInstanceDestroyed(UInventoryItemInstance* Instance) {} 
};
