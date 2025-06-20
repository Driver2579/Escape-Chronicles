// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerCraftItemsFragment.h"
#include "Common/Structs/TableRowBases/InventoryManagerCraftData.h"


bool UInventoryManagerCraftItemsFragment::IsCraftPossible(const FName& CraftName)
{
	FInventoryManagerCraftData* InventoryManagerCraftData =
		AvailableCraftListDataTable->FindRow<FInventoryManagerCraftData>(CraftName, "");

	if (!InventoryManagerCraftData)
	{
		return false;
	}

	const UInventoryManagerComponent* Inventory = GetInventoryManager();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return false;
	}

	TArray<UInventoryItemInstance*> InventoryItemInstances;
	TArray<UInventoryItemInstance*> ItemInstancesToUse;

	Inventory->ForEachInventoryItemInstance([&](UInventoryItemInstance* ItemInstance)
	{
		InventoryItemInstances.Add(ItemInstance);
	});

	for (const FCraftRequirement& CraftRequirement : InventoryManagerCraftData->ItemMaterials)
	{
		if (!ensureAlways(IsValid(CraftRequirement.ItemDefinitionClass)))
		{
			continue;
		}

		for (int32 Index = 0; Index < CraftRequirement.Number; ++Index)
		{
			UInventoryItemInstance* MatchingItemInstance = GetMatchingItemInstance(CraftRequirement,
					InventoryItemInstances);

			if (!MatchingItemInstance)
			{
				return false;
			}

			InventoryItemInstances.Remove(MatchingItemInstance);

			if (CraftRequirement.bConsumeResource)
			{
				ItemInstancesToUse.Add(MatchingItemInstance);
			}
		}
	}

	return true;
}

UInventoryItemInstance* UInventoryManagerCraftItemsFragment::GetMatchingItemInstance(
	const FCraftRequirement& CraftRequirement, const TArray<UInventoryItemInstance*>& ItemInstances)
{
	for (UInventoryItemInstance* InventoryItemInstance : ItemInstances)
	{
#if DO_CHECK
		check(InventoryItemInstance->IsInitialized());
		check(IsValid(InventoryItemInstance->GetDefinition()));
#endif

		const bool bIsMatching =
			InventoryItemInstance->GetDefinition() == CraftRequirement.ItemDefinitionClass &&
				InventoryItemInstance->GetInstanceStats_Mutable().HasAllMatchingStats(
					CraftRequirement.InstanceStatsOverride);

		if (bIsMatching)
		{
			return InventoryItemInstance;
		}
	}

	return nullptr;
}

void UInventoryManagerCraftItemsFragment::Server_Craft_Implementation(const FName CraftName)
{
	

}
