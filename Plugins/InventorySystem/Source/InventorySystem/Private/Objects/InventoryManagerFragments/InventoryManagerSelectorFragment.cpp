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

	if (!bTryHoldItems) return;

	Inventory->OnContentChanged.AddWeakLambda(this, [this]()
	{
		UpdateHolding();
	});

	Inventory->OnPreDeleteItem.AddWeakLambda(this, [this, Inventory](int32 SlotIndex, const FGameplayTag& SlotTypeTag)
	{
		if (SlotIndex != CurrentSlotIndex) return;

		UInventoryItemInstance* ItemInstance = Inventory->GetItemInstance(SlotIndex, SlotTypeTag);

		if (!ensureAlways(IsValid(ItemInstance))) return;

		UHoldingViewInventoryItemFragment* HoldingViewInventoryItemFragment =
			ItemInstance->GetFragmentByClass<UHoldingViewInventoryItemFragment>();

		if (!IsValid(HoldingViewInventoryItemFragment)) return;

		if (HoldingViewInventoryItemFragment->IsHoldingItem(ItemInstance))
		{
			HoldingViewInventoryItemFragment->StopHolding(ItemInstance);
		}
	});

	OnOffsetCurrentSlotIndex.AddWeakLambda(this, [this](int32 Index)
	{
		UpdateHolding();
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
	CurrentSlotIndex = NewIndex;

	UpdateHolding();
}

void UInventoryManagerSelectorFragment::OnRep_SelectedSlotIndex(int32 OldIndex)
{
	OnOffsetCurrentSlotIndex.Broadcast(CurrentSlotIndex);

	UpdateHolding();
}

void UInventoryManagerSelectorFragment::UpdateHolding() const
{
	for (int32 Index = 0; Index < SelectableSlotsArray->GetItems().Num(); ++Index)
	{
		UInventoryItemInstance* ItemInstance = (*SelectableSlotsArray)[Index].Instance;
	
		if (!IsValid(ItemInstance)) continue;

		UHoldingViewInventoryItemFragment* HoldingViewInventoryItemFragment =
			ItemInstance->GetFragmentByClass<UHoldingViewInventoryItemFragment>();

		if (!IsValid(HoldingViewInventoryItemFragment)) continue;

		if (CurrentSlotIndex == Index && !HoldingViewInventoryItemFragment->IsHoldingItem(ItemInstance))
		{
			HoldingViewInventoryItemFragment->StartHolding(ItemInstance);
		}
		else if (CurrentSlotIndex != Index && HoldingViewInventoryItemFragment->IsHoldingItem(ItemInstance))
		{
			HoldingViewInventoryItemFragment->StopHolding(ItemInstance);
		}
	}
}


#if WITH_EDITORONLY_DATA && !NO_LOGGING
void UInventoryManagerSelectorFragment::LogCurrentSlotIndex() const
{
	UE_LOG(LogInventorySystem, Display, TEXT("UInventoryManagerSelectorFragment: %i"), CurrentSlotIndex);
}
#endif
