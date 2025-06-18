// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerSelectorFragment.h"

#include "InventorySystem.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemFragments/HoldingViewInventoryItemFragment.h"

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

	SelectableSlotsArray = &SlotsTypedArray->Array;

	Inventory->OnContentChanged.AddWeakLambda(this, [this]
	{
		// Start new holding
		if (bTryHoldItems)
		{
			UInventoryItemInstance* ItemInstance = SelectableSlotsArray->GetInstance(CurrentSlotIndex);

			if (IsValid(ItemInstance))
			{
				UHoldingViewInventoryItemFragment* HoldingViewInventoryItemFragment =
					ItemInstance->GetFragmentByClass<UHoldingViewInventoryItemFragment>();

				if (IsValid(HoldingViewInventoryItemFragment))
				{
					HoldingViewInventoryItemFragment->StartHolding(ItemInstance);
				}
			}
		}
	});
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

	SelectableSlotsArray = &SlotsTypedArray->Array;

	const int32 SlotsNumber = SelectableSlotsArray->GetItems().Num();

	// Offset the index taking into account the array size
	SetCurrentSlotIndex(((CurrentSlotIndex + Offset) % SlotsNumber + SlotsNumber) % SlotsNumber);

#if DO_ENSURE
	ensureAlways(SelectableSlotsArray->IsValidSlotIndex(CurrentSlotIndex));
#endif

	OnOffsetCurrentSlotIndex.Broadcast(CurrentSlotIndex);
}

void UInventoryManagerSelectorFragment::SetCurrentSlotIndex(const int32 NewIndex)
{
	// Stop Holding
	if (bTryHoldItems)
	{
		UInventoryItemInstance* ItemInstance = SelectableSlotsArray->GetInstance(CurrentSlotIndex);

		if (IsValid(ItemInstance))
		{
			UHoldingViewInventoryItemFragment* HoldingViewInventoryItemFragment =
				ItemInstance->GetFragmentByClass<UHoldingViewInventoryItemFragment>();

			if (IsValid(HoldingViewInventoryItemFragment))
			{
				HoldingViewInventoryItemFragment->StopHolding(ItemInstance);
			}
		}
	}

	CurrentSlotIndex = NewIndex;

	// Start new holding
	if (bTryHoldItems)
	{
		UInventoryItemInstance* ItemInstance = SelectableSlotsArray->GetInstance(CurrentSlotIndex);

		if (IsValid(ItemInstance))
		{
			UHoldingViewInventoryItemFragment* HoldingViewInventoryItemFragment =
				ItemInstance->GetFragmentByClass<UHoldingViewInventoryItemFragment>();

			if (IsValid(HoldingViewInventoryItemFragment))
			{
				HoldingViewInventoryItemFragment->StartHolding(ItemInstance);
			}
		}
	}
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
