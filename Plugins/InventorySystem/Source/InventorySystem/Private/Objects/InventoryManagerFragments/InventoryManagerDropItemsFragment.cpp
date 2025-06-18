// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerDropItemsFragment.h"
#include "Actors/InventoryPickupItem.h"
#include "Objects/InventoryItemFragments/HoldingViewInventoryItemFragment.h"

void UInventoryManagerDropItemsFragment::Server_DropItem_Implementation(const int32 SlotIndex,
	const FGameplayTag& SlotsType)
{
	// === Get the right item instance by the given parameters ===

	UInventoryManagerComponent* Inventory = GetInventoryManager();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	UInventoryItemInstance* ItemInstance = Inventory->GetItemInstance(SlotIndex, SlotsType);

	if (!IsValid(ItemInstance))
	{
		return;
	}

	// Spawn an item into the world to make it able to pick up later
	AInventoryPickupItem* ItemActor = GetWorld()->SpawnActorDeferred<AInventoryPickupItem>(DropItemActorClass,
		Inventory->GetOwner()->GetActorTransform());

	if (!ensureAlways(IsValid(ItemActor)))
	{
		return;
	}

	ItemActor->SetItemInstance(ItemInstance->Duplicate(ItemActor));

#if DO_CHECK
	check(IsValid(ItemActor->GetItemInstance()));
#endif

	Multicast_PreDropItem(SlotIndex, SlotsType);

	// Remove an item from the slot because we dropped it
	ensureAlways(Inventory->DeleteItem(SlotIndex, SlotsType));

	const FTransform OwnerActorTransform = Inventory->GetOwner()->GetActorTransform();

	ItemActor->FinishSpawning(OwnerActorTransform);

	// === Add a throw impulse ===

	UPrimitiveComponent* ItemActorMeshComponent = ItemActor->GetMesh();

	if (ensureAlways(IsValid(ItemActorMeshComponent)))
	{
		const FVector RotatedImpulseVector = OwnerActorTransform.GetRotation().RotateVector(ThrowingImpulse);
		ItemActorMeshComponent->AddImpulse(RotatedImpulseVector + ItemActor->GetVelocity());
	}
}

void UInventoryManagerDropItemsFragment::Multicast_PreDropItem_Implementation(const int32 SlotIndex,
	const FGameplayTag& SlotsType)
{
	UInventoryManagerComponent* Inventory = GetInventoryManager();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	UInventoryItemInstance* ItemInstance = Inventory->GetItemInstance(SlotIndex, SlotsType);

	if (!IsValid(ItemInstance))
	{
		return;
	}

	// Stop Holding (This shouldn't be here. But damn because of the mistake earlier I have to. TODO: Change it)
	UHoldingViewInventoryItemFragment* HoldingViewInventoryItemFragment =
			ItemInstance->GetFragmentByClass<UHoldingViewInventoryItemFragment>();

	if (IsValid(HoldingViewInventoryItemFragment))
	{
		HoldingViewInventoryItemFragment->StopHolding(ItemInstance);
	}
}
