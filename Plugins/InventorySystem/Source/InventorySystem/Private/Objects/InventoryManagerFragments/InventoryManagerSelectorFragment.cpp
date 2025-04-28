// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerSelectorFragment.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Net/UnrealNetwork.h"

void UInventoryManagerSelectorFragment::OnManagerInitialized(UInventoryManagerComponent* Inventory)
{
	Super::OnManagerInitialized(Inventory);

	if (!ensureAlways(IsValid(Inventory)))
	{
		CurrentSlotIndex = INDEX_NONE;
	}
	else
	{
		CurrentSlotIndex = 0;
	}
}

void UInventoryManagerSelectorFragment::GetLifetimeReplicatedProps(
	TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentSlotIndex);
}

void UInventoryManagerSelectorFragment::Server_OffsetCurrentSlotIndex_Implementation(const int32 Offset)
{
	UInventoryManagerComponent* Inventory = GetInventoryManager();
	
	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}
	
	const FInventorySlotsTypedArray* SlotsTypedArray = Inventory->GetTypedInventorySlotsLists().GetArrays()
		.FindByKey(SelectableSlotsType);
	
	if (!ensureAlways(SlotsTypedArray != nullptr))
	{
		return;
	}

	const int32 SlotsNumber = SlotsTypedArray->Array.GetSlots().Num();
	
	CurrentSlotIndex = ((CurrentSlotIndex + Offset) % SlotsNumber + SlotsNumber) % SlotsNumber;
	
	if (bLogCurrentSlotIndex)
	{
		LogCurrentSlotIndex();
	}
}

void UInventoryManagerSelectorFragment::OnRep_SelectedSlotIndex()
{
	if (bLogCurrentSlotIndex)
	{
		LogCurrentSlotIndex();
	}
}

void UInventoryManagerSelectorFragment::LogCurrentSlotIndex() const
{
	UE_LOG(LogTemp, Log, TEXT("UInventoryManagerSelectorFragment::LogCurrentSlotIndex: %i"), CurrentSlotIndex);
}
