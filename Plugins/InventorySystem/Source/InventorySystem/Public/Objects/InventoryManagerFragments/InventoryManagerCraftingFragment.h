// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventoryManagerFragment.h"

#include "InventoryManagerCraftingFragment.generated.h"

UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerCraftingFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	/*
	UFUNCTION(Server, Reliable)
	void Server_DropItem(const int32 SlotIndex, const FGameplayTag SlotsType);
	*/

private:
	/*UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AInventoryPickupItem> DropItemActorClass;

	UPROPERTY(EditDefaultsOnly)
	FVector ThrowingDirection;*/

	UPROPERTY(EditDefaultsOnly, meta=(RowType="InventoryItemCraftData"))
	FDataTableRowHandle DataTableRow;
};
