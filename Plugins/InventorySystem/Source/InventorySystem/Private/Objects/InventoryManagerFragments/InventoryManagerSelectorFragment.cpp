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

void UInventoryManagerSelectorFragment::DropSelectedItem(const FVector ThrowingDirection)
{
	Server_DropItem(CurrentSlotIndex, ThrowingDirection);
}

void UInventoryManagerSelectorFragment::GetLifetimeReplicatedProps(
	TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentSlotIndex);
}

bool UInventoryManagerSelectorFragment::Server_DropItem_Validate(const int32 Offset, const FVector ThrowingDirection)
{
	return ThrowingDirection.Length() < MaxTrowingPower;
}

void UInventoryManagerSelectorFragment::Server_DropItem_Implementation(const int32 Offset, const FVector ThrowingDirection)
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

	const FTransform ActorTransform = Pawn->GetActorTransform();
	
	const UInventoryItemInstance* ItemInstance = Inventory->GetItemInstance(Offset, SelectableSlotsType);

	if (!IsValid(ItemInstance))
	{
		return;
	}
	
	AInventoryPickupItem* ItemActor = GetWorld()->SpawnActorDeferred<AInventoryPickupItem>(DropItemActorClass,
		ActorTransform);
	
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
	
	if (!Inventory->DeleteItem(Offset, SelectableSlotsType))
	{
		ItemActor->Destroy();

		return;
	}
	
	ItemActor->FinishSpawning(ActorTransform);

	UPrimitiveComponent* ItemActorMeshComponent = ItemActor->GetStaticMeshComponent();

	if (ensureAlways(IsValid(ItemActorMeshComponent)))
	{
		const FVector RotatedImpulseVector = ActorTransform.GetRotation().RotateVector(ThrowingDirection);
		ItemActorMeshComponent->AddImpulse(RotatedImpulseVector + ItemActor->GetVelocity());
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
