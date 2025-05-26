// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/InventoryManagerFragments/InventoryManagerFragmentContentGenerator.h"

#include "Common/Structs/TableRowBases/InventoryManagerGeneratingContentData.h"
#include "Engine/AssetManager.h"

void UInventoryManagerFragmentContentGenerator::OnManagerInitialized(UInventoryManagerComponent* Inventory)
{
	Super::OnManagerInitialized(Inventory);

	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(DataTable.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnDataTableLoaded, Inventory));
}

void UInventoryManagerFragmentContentGenerator::OnDataTableLoaded(UInventoryManagerComponent* Inventory)
{
	TArray<FInventoryManagerGeneratingContentData*> GeneratingContent;

	DataTable->GetAllRows("", GeneratingContent);
	
	for (const FInventoryManagerGeneratingContentData* Row : GeneratingContent)
	{
		if (FMath::Rand() < Row->Probability)
		{
			return;
		}

		const int32 Number = FMath::RandRange(Row->MinNumber, Row->MaxNumber);
		
		for (int32 Index = 0; Index < Number; Index++)
		{
			UInventoryItemInstance* ItemInstance = NewObject<UInventoryItemInstance>();
			ItemInstance->Initialize(Row->ItemDefinition);

			for (FLocalDataItem Data : Row->LocalDataOverride.GetAllData())
			{
				ItemInstance->LocalData.SetData(Data);
			}
			
			Inventory->AddItem(ItemInstance);
		}
	}

	if (DataTableHandle.IsValid())
	{
		DataTableHandle->CancelHandle();
		DataTableHandle.Reset();
	}
}
