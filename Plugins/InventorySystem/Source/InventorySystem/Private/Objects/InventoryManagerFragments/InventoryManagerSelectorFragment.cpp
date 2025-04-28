// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerSelectorFragment.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Actors/InventoryPickupItem.h"
#include "GameFramework/PlayerState.h"
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

void UInventoryManagerSelectorFragment::DropSelectedItem()
{
	Server_DropItem(CurrentSlotIndex);
}

void UInventoryManagerSelectorFragment::GetLifetimeReplicatedProps(
	TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentSlotIndex);
}

void UInventoryManagerSelectorFragment::Server_DropItem_Implementation(const int32 Offset)
{
	UInventoryManagerComponent* Inventory = GetInventoryManager();
	
	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	const APlayerState* PlayerState = Inventory->GetOwner<APlayerState>();

	if (!IsValid(PlayerState))
	{
		return;
	}

	const APawn* Pawn = PlayerState->GetPawn();

	if (!IsValid(Pawn))
	{
		return;
	}

	const FTransform Character = Pawn->GetActorTransform();
	
	const UInventoryItemInstance* ItemInstance = Inventory->GetItemInstance(Offset, SelectableSlotsType);

	if (!IsValid(ItemInstance))
	{
		return;
	}
	
	AInventoryPickupItem* ItemActor = GetWorld()->SpawnActorDeferred<AInventoryPickupItem>(DropItemActorClass,
		Character);
	
	if (!ensureAlways(IsValid(ItemActor)))
	{
		return;
	}

	UInventoryItemInstance* ItemInstanceDuplicate = ItemInstance->Duplicate(ItemActor);

	if (!ensureAlways(IsValid(ItemInstanceDuplicate)))
	{
		return;
	}
	
	ItemActor->SetItemInstance(ItemInstanceDuplicate);
	
	if (Inventory->DeleteItem(Offset, SelectableSlotsType))
	{
		ItemActor->FinishSpawning(Character);
	}
	else
	{
		ItemActor->Destroy();
	}
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
	
	CurrentSlotIndex = (((CurrentSlotIndex + Offset) % SlotsNumber) + SlotsNumber) % SlotsNumber;


	UE_LOG(LogTemp, Error, TEXT("====== ====== ====== SLOT INDEX: %i"), CurrentSlotIndex);
}
