// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemFragment.generated.h"

class UInventoryManagerComponent;
class UInventoryItemDefinition;
class UInventoryItemInstance;

struct FGameplayTag;

// Base class for item's fragments that can describe various logic for it
UCLASS(Abstract, EditInlineNew)
class INVENTORYSYSTEM_API UInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsValidConfiguration(UInventoryItemDefinition* ItemDefinition) { return true; }
	
	virtual void OnItemInstanceInitialized(UInventoryItemInstance* Instance) {}

	virtual void OnItemAddedToSlot(UInventoryItemInstance* ItemInstance,
		UInventoryManagerComponent* InventoryManagerComponent, const FGameplayTag& SlotTypeTag,
		const int32 SlotIndex) const
	{}

	virtual void OnItemRemovedFromSlot(UInventoryItemInstance* ItemInstance,
		UInventoryManagerComponent* InventoryManagerComponent, const FGameplayTag& SlotTypeTag,
		const int32 SlotIndex) const
	{}
};