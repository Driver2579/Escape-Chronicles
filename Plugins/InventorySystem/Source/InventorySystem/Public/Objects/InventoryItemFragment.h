// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemFragment.generated.h"

class UInventoryItemDefinition;
class UInventoryItemInstance;

// Base class for item's fragments that can describe various logic for it
UCLASS(Abstract, EditInlineNew)
class INVENTORYSYSTEM_API UInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Checks whether this fragment is properly configured for the given ItemDefinition. Returns true if the fragment is
	 * valid and can be used. Can be overridden in derived classes to validate required data (e.g., asset references,
	 * numeric parameters, or compatibility with other fragments).
	 */
	virtual bool IsValidConfiguration(UInventoryItemDefinition* ItemDefinition) { return true; }
	
	virtual void OnItemInstanceInitialized(UInventoryItemInstance* Instance) {}
};