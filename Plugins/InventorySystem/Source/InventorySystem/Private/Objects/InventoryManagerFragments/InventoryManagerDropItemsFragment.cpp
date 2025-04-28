// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragments/InventoryManagerDropItemsFragment.h"

#include "Actors/InventoryPickupItem.h"

void UInventoryManagerDropItemsFragment::Server_DropItem_Implementation(const int32 SlotIndex,
	const FGameplayTag SlotsType)
{
	UInventoryManagerComponent* Inventory = GetInventoryManager();
	
	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	const UInventoryItemInstance* ItemInstance = Inventory->GetItemInstance(SlotIndex);

	if (!IsValid(ItemInstance))
	{
		return;
	}
	
	const AActor* OwnerActor = Inventory->GetOwner();

	if (!IsValid(OwnerActor))
	{
		return;
	}
	
	const FTransform OwnerActorTransform = OwnerActor->GetActorTransform();

	// === Try to spawn actor ===
	
	AInventoryPickupItem* ItemActor = GetWorld()->SpawnActorDeferred<AInventoryPickupItem>(DropItemActorClass,
		OwnerActorTransform);
	
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
	
	if (!Inventory->DeleteItem(SlotIndex, SlotsType))
	{
		ItemActor->Destroy();

		return;
	}
	
	ItemActor->FinishSpawning(OwnerActorTransform);

	// === Add throw impulse
	
	UPrimitiveComponent* ItemActorMeshComponent = ItemActor->GetStaticMeshComponent();

	if (ensureAlways(IsValid(ItemActorMeshComponent)))
	{
		const FVector RotatedImpulseVector = OwnerActorTransform.GetRotation().RotateVector(ThrowingDirection);
		ItemActorMeshComponent->AddImpulse(RotatedImpulseVector + ItemActor->GetVelocity());
	}
}
