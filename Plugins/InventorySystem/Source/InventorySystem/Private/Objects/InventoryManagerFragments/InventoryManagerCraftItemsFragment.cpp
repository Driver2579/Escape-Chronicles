// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerCraftItemsFragment.h"
#include "Common/Structs/TableRowBases/InventoryManagerCraftData.h"

void UInventoryManagerCraftItemsFragment::Server_Craft_Implementation(const FName CraftName)
{
	FInventoryManagerCraftData* InventoryManagerCraftData =
			AvailableCraftListDataTable->FindRow<FInventoryManagerCraftData>(CraftName, "");

	if (!InventoryManagerCraftData)
	{
		return;
	}

	UInventoryManagerComponent* Inventory = GetInventoryManager();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	TArray<UInventoryItemInstance*> InventoryItemInstances;
	TArray<UInventoryItemInstance*> ItemInstancesToUse;

	Inventory->ForEachInventoryItemInstance([&](UInventoryItemInstance* ItemInstance)
	{
		InventoryItemInstances.Add(ItemInstance);
	});

	TFunction<void(const TSubclassOf<UInventoryItemDefinition>&)> MoveMatchingItemInstance =
		[&InventoryItemInstances, &ItemInstancesToUse](const TSubclassOf<UInventoryItemDefinition>& RequiredDefinition)
		{
			for (UInventoryItemInstance* InventoryItemInstance : InventoryItemInstances)
			{
				if (InventoryItemInstance->GetDefinition() == RequiredDefinition)
				{
					ItemInstancesToUse.Add(InventoryItemInstance);
					InventoryItemInstances.Remove(InventoryItemInstance);

					return;
				}
			}
		};

	for (const FCraftRequirement& CraftRequirement : InventoryManagerCraftData->MaterialsItemDefinition)
	{
		for (int32 Index = 0; 0 < CraftRequirement.Number; ++Index)
		{
			MoveMatchingItemInstance(CraftRequirement.ItemDefinitionClass);
		}
	}

}
