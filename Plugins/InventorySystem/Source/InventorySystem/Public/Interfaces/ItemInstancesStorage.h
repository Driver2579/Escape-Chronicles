// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemInstancesStorage.generated.h"

class UInventoryItemInstance;

UINTERFACE()
class INVENTORYSYSTEM_API UItemInstancesStorage : public UInterface
{
	GENERATED_BODY()
};

// Interface for objects that can store and manage inventory item instances
class IItemInstancesStorage
{
	GENERATED_BODY()

public:
	/**
	 * Destroys an item instance. Owner must handle cleanup/removal.
	 * @param ItemInstance Item to break (ownership must be verified).
	 */
	virtual void BreakItemInstance(UInventoryItemInstance* ItemInstance) = 0;
};