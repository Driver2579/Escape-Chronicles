// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemFragment.generated.h"

class UInventoryItemDefinition;
class UInventoryItemInstance;

// Base class for creating fragments of an item that can describe various logic for it
UCLASS(Abstract, EditInlineNew)
class INVENTORYSYSTEM_API UInventoryItemFragment : public UObject
{
	GENERATED_BODY()
	
public:
	virtual bool HasValidProperties(UInventoryItemDefinition* ItemDefinition) {	return true; }
	
	virtual void OnInstanceInitialized(UInventoryItemInstance* Instance) {}
};