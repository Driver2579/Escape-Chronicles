// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemFragments/DoorKeyInventoryItemFragment.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"

bool UDoorKeyInventoryItemFragment::IsValidConfiguration(UInventoryItemDefinition* ItemDefinition)
{
	// Non-durability keys are always valid
	if (!bUseDurability)
	{
		return true;
	}

	// Durability keys must include durability fragment
	return ensureAlwaysMsgf(ItemDefinition->GetFragments().FindItemByClass<UDurabilityInventoryItemFragment>(),
		TEXT("If bUseDurability is true, the definition must include UDurabilityInventoryItemFragment"));
}