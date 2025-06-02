// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryManagerFragments/InventoryManagerFragment.h"
#include "InventoryManagerFragmentContentGenerator.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerFragmentContentGenerator : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	virtual void OnManagerInitialized(UInventoryManagerComponent* Inventory) override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UDataTable> DataTable;

	TSharedPtr<FStreamableHandle> DataTableHandle;
	
	void OnDataTableLoaded(UInventoryManagerComponent* Inventory);
};
