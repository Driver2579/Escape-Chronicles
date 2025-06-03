// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerFragmentLooting.h"

void UInventoryManagerFragmentLooting::Server_Swap_Implementation(UInventoryManagerComponent* Inventory,
	const int32 FromSlotIndex, const int32 ToSlotIndex, const FGameplayTag& FromSlotsType,
	const FGameplayTag& ToSlotsType)
{
	Inventory->SwapItems(FromSlotIndex, ToSlotIndex, FromSlotsType, ToSlotsType);
}

bool UInventoryManagerFragmentLooting::Server_Loot_Validate(UInventoryManagerComponent* Inventory,
	UInventoryItemInstance* ItemInstance)
{
	return true;
}

void UInventoryManagerFragmentLooting::Server_Loot_Implementation(UInventoryManagerComponent* Inventory,
	UInventoryItemInstance* ItemInstance)
{
	Inventory->AddItem(ItemInstance);
	ItemInstance->Break();
}
