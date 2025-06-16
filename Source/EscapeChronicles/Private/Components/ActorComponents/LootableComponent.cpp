// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/LootableComponent.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"

void ULootableComponent::BeginPlay()
{
	Super::BeginPlay();

	OnInteract.AddUObject(this, &ThisClass::OnOpenInventory);
}

void ULootableComponent::OnOpenInventory(UInteractionManagerComponent* InteractionManagerComponent)
{
#if DO_CHECK
	check(IsValid(InteractionManagerComponent))
#endif

	UInventoryManagerComponent* Inventory = GetOwner()->GetComponentByClass<UInventoryManagerComponent>();
	
	UInventoryManagerComponent* InInventory =
		InteractionManagerComponent->GetOwner()->GetComponentByClass<UInventoryManagerComponent>();

	if (!ensureAlways(IsValid(Inventory)) || !ensureAlways(IsValid(InInventory)))
	{
		return;
	}

	UInventoryManagerTransferItemsFragment* InventoryManagerTransferItemsFragment =
		Inventory->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();

	UInventoryManagerTransferItemsFragment* InInventoryManagerTransferItemsFragment =
		InInventory->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();

	if (!ensureAlways(IsValid(InventoryManagerTransferItemsFragment)) ||
		!ensureAlways(IsValid(InInventoryManagerTransferItemsFragment)))
	{
		return;
	}

	if (InventoryManagerTransferItemsFragment->HasInventoryAccess(InInventoryManagerTransferItemsFragment))
	{
		InInventoryManagerTransferItemsFragment->TrySetLootInventory(Inventory);
	}
}
