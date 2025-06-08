// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryManagerFragment.h"
#include "InventoryManagerFragmentContentGenerator.generated.h"

// TODO: do docs
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerFragmentContentGenerator : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	virtual void OnManagerInitialized() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UDataTable> DataTable;

	TSharedPtr<FStreamableHandle> DataTableHandle;
	
	void OnDataTableLoaded(UInventoryManagerComponent* Inventory);
};
