// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"

#include "Objects/InventoryItemInstance.h"

void UDurabilityInventoryItemFragment::ReduceDurability(UInventoryItemInstance* Instance, int32 Amount) const
{
	const float CurrentDurability = Instance->GetLocalData_Mutable().GetData(DurabilityDataTag)->Value;

	const float NewDurability = CurrentDurability - Amount;
	
	Instance->GetLocalData_Mutable().SetData(DurabilityDataTag, NewDurability);

	// Break item
	if (NewDurability <= 0)
	{
		Instance->Break();
	}
}

void UDurabilityInventoryItemFragment::OnInstanceInitialized(UInventoryItemInstance* Instance)
{
	Super::OnInstanceInitialized(Instance);

	Instance->GetLocalData_Mutable().SetData(DurabilityDataTag, InitialDurability);
}
