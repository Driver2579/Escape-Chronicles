// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Objects/InventoryManagerFragment.h"
#include "InventoryManagerCraftItemsFragment.generated.h"

UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerCraftItemsFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:

private:
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UDataTable> AvailableCraftListDataTable;
};
