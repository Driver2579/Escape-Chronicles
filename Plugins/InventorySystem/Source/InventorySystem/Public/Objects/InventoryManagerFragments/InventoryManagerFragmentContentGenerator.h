/*
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryManagerFragment.h"
#include "InventoryManagerFragmentContentGenerator.generated.h"

// Inventory manager fragment for procedural content generation (use DataTable)
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerFragmentContentGenerator : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	// Soft reference to DataTable containing spawn rules (FInventoryManagerGeneratingContentData rows)
	virtual void OnManagerInitialized() override;
	
private:
	UPROPERTY(EditDefaultsOnly)/*#1#
	TSoftObjectPtr<UDataTable> DataTable;

	// Handle for async DataTable loading
	TSharedPtr<FStreamableHandle> DataTableHandle;

	// Processes loot rules and populates inventory
	void OnDataTableLoaded(UInventoryManagerComponent* Inventory);
};
*/
