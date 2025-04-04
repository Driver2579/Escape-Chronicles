// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/InventoryPickupItem.h"

#include "Components/ActorComponents/InventoryManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemInstance.h"
#include "Objects/InventoryItemFragments/PickupInventoryItemFragment.h"

// Sets default values
AInventoryPickupItem::AInventoryPickupItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	StaticMeshComponent= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SetRootComponent(StaticMeshComponent);
}

void AInventoryPickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemInstance);
}

void AInventoryPickupItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateMesh();
}

void AInventoryPickupItem::Pickup(UInventoryManagerComponent* InventoryManagerComponent)
{
	if (!ensureAlways(IsValid(InventoryManagerComponent)))
	{
		return;
	}

	if (!ensureAlways(IsValid(ItemInstance)))
	{
		return;
	}

	// TODO: Make an addition without explicitly specifying a tag and index
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Inventory.SlotTypes.Main"));
	
	InventoryManagerComponent->AddItem(ItemInstance, Tag, 0);
	
	Destroy();
}

void AInventoryPickupItem::UpdateMesh() const
{
	if (!IsValid(StaticMeshComponent))
	{
		return;
	}
	
	if (!IsValid(ItemInstance))
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
		return;
	}
	
	TSubclassOf<UInventoryItemDefinition> ItemDefinition = ItemInstance->GetItemDefinition();

	if (!IsValid(ItemDefinition))
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
		return;
	}

	const UInventoryItemDefinition* ItemDefinitionCDO = ItemDefinition->GetDefaultObject<UInventoryItemDefinition>();

	if (!IsValid(ItemDefinitionCDO))
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
		return;
	}

	const UPickupInventoryItemFragment* ItemFragment = ItemDefinitionCDO->GetFragmentByClass<UPickupInventoryItemFragment>();

	if (!IsValid(ItemFragment))
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
		return;
	}
	
	UStaticMesh* StaticMesh = ItemFragment->GetStaticMesh();
	
	if (!IsValid(StaticMesh))
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
		return;
	}

	StaticMeshComponent->SetStaticMesh(StaticMesh);
}

