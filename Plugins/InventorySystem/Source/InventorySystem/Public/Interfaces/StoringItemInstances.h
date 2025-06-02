// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StoringItemInstances.generated.h"

class UInventoryItemInstance;

UINTERFACE()
class INVENTORYSYSTEM_API UStoringItemInstances : public UInterface
{
	GENERATED_BODY()
};

// Used for objects that can store UInventoryItemInstance in themselves
class IStoringItemInstances
{
	GENERATED_BODY()

public:
	virtual void BreakItemInstance(UInventoryItemInstance* ItemInstance) = 0;
};