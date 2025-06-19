// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Objects/InventoryManagerFragment.h"
#include "InventoryManagerCraftItemsFragment.generated.h"

UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerCraftItemsFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	const UDataTable* GetAvailableCraftList() const { return AvailableCraftListDataTable; }

	UFUNCTION(Server, Reliable)
	void Server_Craft(const FName CraftName);

private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const UDataTable> AvailableCraftListDataTable;
};
