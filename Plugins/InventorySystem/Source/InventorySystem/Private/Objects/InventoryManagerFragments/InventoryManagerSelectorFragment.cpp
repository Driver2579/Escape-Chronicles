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

void UInventoryManagerSelectorFragment::SelectNextItem()
{
	Server_OffsetCurrentSlotIndex(1);
}

void UInventoryManagerSelectorFragment::SelectPrevItem()
{
	Server_OffsetCurrentSlotIndex(-1);
}

void UInventoryManagerSelectorFragment::GetLifetimeReplicatedProps(
	TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentSlotIndex);
}

void UInventoryManagerSelectorFragment::Server_OffsetCurrentSlotIndex_Implementation(const int32 Offset)
{
	UObject* Outer = GetOuter();

	if (!ensureAlways(IsValid(Outer)))
	{
		return;
	}
	
	UInventoryManagerComponent* Inventory = Cast<UInventoryManagerComponent>(Outer);
	
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
	
	CurrentSlotIndex = (((CurrentSlotIndex + Offset) % SlotsNumber) + SlotsNumber) % SlotsNumber;


	UE_LOG(LogTemp, Error, TEXT("====== ====== ====== SLOT INDEX: %i"), CurrentSlotIndex);
}
