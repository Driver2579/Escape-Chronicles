// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/LootableComponent.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"

ULootableComponent::ULootableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

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

	const UInventoryManagerComponent* InInventory =
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

		Looters.Add(InInventoryManagerTransferItemsFragment);
	}
}


void ULootableComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority() && Looters.Num() > 0)
	{
		return;
	}

	TArray<TWeakObjectPtr<UInventoryManagerTransferItemsFragment>> LootersToRemove;

	for (const TWeakObjectPtr<UInventoryManagerTransferItemsFragment>& Looter : Looters)
	{
		const UInventoryManagerComponent* LooterInventory = Looter->GetInventoryManager();

		if (!ensureAlways(IsValid(LooterInventory)))
		{
			continue;
		}

		const float DistanceToLooter =
			FVector::Distance(LooterInventory->GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation());

		if (DistanceToLooter > MaxLootingDistance)
		{
			LootersToRemove.Add(Looter);
		}
	}

	for (const TWeakObjectPtr<UInventoryManagerTransferItemsFragment>& Looter : LootersToRemove)
	{
		Looter->TrySetLootInventory(nullptr);

		Looters.Remove(Looter);
	}
}
