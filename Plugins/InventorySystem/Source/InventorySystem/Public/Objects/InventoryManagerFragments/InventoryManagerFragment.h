// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryManagerFragment.generated.h"

class UInventoryManagerComponent;

// Base class for creating fragments of an inventory that can describe various logic for it
UCLASS(Abstract, EditInlineNew)
class INVENTORYSYSTEM_API UInventoryManagerFragment : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void OnManagerInitialized(UInventoryManagerComponent* Inventory) {}
};