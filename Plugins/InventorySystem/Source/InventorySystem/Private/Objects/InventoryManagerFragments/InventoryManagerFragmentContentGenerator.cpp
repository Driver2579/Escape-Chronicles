// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerFragmentContentGenerator.h"
#include "Common/Structs/TableRowBases/InventoryManagerGeneratingContentData.h"
#include "Engine/AssetManager.h"

void UInventoryManagerFragmentContentGenerator::OnManagerInitialized()
{
	Super::OnManagerInitialized();

	UInventoryManagerComponent* Inventory = GetInventoryManager();

	// Start async load of configured DataTable if inventory is valid
	if (ensureAlways(IsValid(Inventory)))
	{
		UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(DataTable.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnDataTableLoaded, Inventory));
	}
}

void UInventoryManagerFragmentContentGenerator::OnDataTableLoaded(UInventoryManagerComponent* Inventory)
{
	// Retrieve all loot table rows
	TArray<FInventoryManagerGeneratingContentData*> GeneratingContent;
	DataTable->GetAllRows("", GeneratingContent);

	// Process each loot table entry
	for (const FInventoryManagerGeneratingContentData* Row : GeneratingContent)
	{
		// Skip if probability check fails
		if (FMath::Rand() < Row->Probability)
		{
			continue;
		}

		// Determine quantity to spawn
		const int32 Number = FMath::RandRange(Row->MinNumber, Row->MaxNumber);
		
		for (int32 Index = 0; Index < Number; Index++)
		{
			UInventoryItemInstance* ItemInstance = NewObject<UInventoryItemInstance>();
			ItemInstance->Initialize(Row->ItemDefinition);

			// Apply stat overrides
			for (const FInstanceStatsItem& Stat : Row->InstanceStatsOverride.GetAllStats())
			{
				ItemInstance->GetInstanceStats_Mutable().SetStat(Stat);
			}

			Inventory->AddItem(ItemInstance);
		}
	}

	// Clean up async loading handle
	if (DataTableHandle.IsValid())
	{
		DataTableHandle->CancelHandle();
		DataTableHandle.Reset();
	}
}
