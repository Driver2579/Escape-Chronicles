// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"
#include "Objects/InventoryItemInstance.h"

void UDurabilityInventoryItemFragment::OnItemInstanceInitialized(UInventoryItemInstance* Instance)
{
	Super::OnItemInstanceInitialized(Instance);

#if DO_CHECK
	check(IsValid(Instance))
#endif

	// Initialize the durability value
	Instance->GetInstanceStats_Mutable().SetStat(DurabilityStatTag, InitialDurability);
}

void UDurabilityInventoryItemFragment::ReduceDurability(UInventoryItemInstance* Instance, int32 Amount) const
{
#if DO_CHECK
	check(IsValid(Instance))
#endif

	const FInstanceStatsItem* DurabilityStat = Instance->GetInstanceStats_Mutable().GetStat(DurabilityStatTag);
	
#if DO_CHECK
	check(DurabilityStat)
#endif

	const float CurrentDurability = DurabilityStat->Value;

	const float NewDurability = CurrentDurability - Amount;
	
	Instance->GetInstanceStats_Mutable().SetStat(DurabilityStatTag, NewDurability);

	// Break item if durability has run out
	if (NewDurability <= 0)
	{
		Instance->Break();
	}
}
