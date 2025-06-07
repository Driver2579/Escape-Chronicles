// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerDropItemsFragment.h"

#include "Actors/InventoryPickupItem.h"

void UInventoryManagerDropItemsFragment::Server_DropItem_Implementation(const int32 SlotIndex,
	const FGameplayTag& SlotsType)
{
	// Getting the right item instance

	UInventoryManagerComponent* Inventory = GetInventoryManager();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	const UInventoryItemInstance* ItemInstance = Inventory->GetItemInstance(SlotIndex, SlotsType);

	if (!IsValid(ItemInstance))
	{
		return;
	}

	// === Try to spawn an actor ===

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
