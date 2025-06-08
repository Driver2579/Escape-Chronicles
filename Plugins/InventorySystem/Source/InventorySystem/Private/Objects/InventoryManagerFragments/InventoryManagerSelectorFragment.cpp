// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerSelectorFragment.h"

#include "InventorySystem.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Net/UnrealNetwork.h"

void UInventoryManagerSelectorFragment::OnManagerInitialized()
{
	Super::OnManagerInitialized();

#if WITH_EDITORONLY_DATA && !NO_LOGGING
	if (bLogCurrentSlotIndex)
	{
		OnOffsetCurrentSlotIndex.AddLambda([this](int32 Index)
		{
			LogCurrentSlotIndex();
		});	
	}
#endif
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

	const FInventorySlotsTypedArray* SlotsTypedArray =
		Inventory->GetInventoryContent().GetItems().FindByKey(SelectableSlotsTypeTag);

	if (!ensureAlways(SlotsTypedArray))
	{
		return;
	}

	const int32 SlotsNumber = SlotsTypedArray->Array.GetItems().Num();

	// Offset the index taking into account the array size
	CurrentSlotIndex = ((CurrentSlotIndex + Offset) % SlotsNumber + SlotsNumber) % SlotsNumber;

#if DO_ENSURE
	ensureAlways(SlotsTypedArray->Array.IsValidSlotIndex(CurrentSlotIndex));
#endif

#if WITH_EDITORONLY_DATA && !NO_LOGGING
	if (bLogCurrentSlotIndex)
	{
		LogCurrentSlotIndex();
	}
#endif

	OnOffsetCurrentSlotIndex.Broadcast(CurrentSlotIndex);
}

void UInventoryManagerSelectorFragment::OnRep_SelectedSlotIndex()
{
	OnOffsetCurrentSlotIndex.Broadcast(CurrentSlotIndex);
}

#if WITH_EDITORONLY_DATA && !NO_LOGGING
void UInventoryManagerSelectorFragment::LogCurrentSlotIndex() const
{
	UE_LOG(LogInventorySystem, Display, TEXT("UInventoryManagerSelectorFragment: %i"), CurrentSlotIndex);
}
#endif
