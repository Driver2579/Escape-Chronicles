// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/InventoryPickupItem.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemInstance.h"
#include "Objects/InventoryItemFragments/PickupInventoryItemFragment.h"

// Sets default values
AInventoryPickupItem::AInventoryPickupItem()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	StaticMeshComponent= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SetRootComponent(StaticMeshComponent);
}

void AInventoryPickupItem::BeginPlay()
{
	Super::BeginPlay();

	check(bItemInstanceIsValid);

	ItemInstance->Initialize();
}

void AInventoryPickupItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// Only update actors on scene
	if (!IsAsset())
	{
		return;
	}

	bItemInstanceIsValid = ApplyChangesFromItemInstance();

	// While bItemInstanceIsValidSet is invalid, set the default settings
	if (!bItemInstanceIsValid)
	{
		SetDefaultSettings();
	}
}

void AInventoryPickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemInstance);
}

bool AInventoryPickupItem::ApplyChangesFromItemInstance() const
{
	if (!ensureAlways(IsValid(StaticMeshComponent)) || !IsValid(ItemInstance))
	{
		return false;
	}
	
	const UPickupInventoryItemFragment* ItemFragment = ItemInstance->GetFragmentByClass<UPickupInventoryItemFragment>();

	if (!IsValid(ItemFragment))
	{
		return false;
	}
	
	UStaticMesh* StaticMesh = ItemFragment->GetStaticMesh();
	
	if (!IsValid(StaticMesh))
	{
		return false;
	}

	StaticMeshComponent->SetStaticMesh(StaticMesh);

	return true;
}

void AInventoryPickupItem::SetDefaultSettings() const
{
	if (!ensureAlways(IsValid(StaticMeshComponent)))
	{
		return;
	}

	const AInventoryPickupItem* DefaultObject = GetClass()->GetDefaultObject<AInventoryPickupItem>();

	if (!ensureAlways(IsValid(DefaultObject)))
	{
		return;
	}

	const UStaticMeshComponent* DefaultObjectStaticMeshComponent = DefaultObject->GetStaticMeshComponent();
	
	if (!ensureAlways(IsValid(DefaultObjectStaticMeshComponent)))
	{
		return;
	}
	
	UStaticMesh* DefaultObjectStaticMesh = DefaultObjectStaticMeshComponent->GetStaticMesh();

	if (!ensureAlways(IsValid(DefaultObjectStaticMesh)))
	{
		return;
	}

	StaticMeshComponent->SetStaticMesh(DefaultObjectStaticMesh);
}

void AInventoryPickupItem::Pickup(UInventoryManagerComponent* InventoryManagerComponent)
{
	if (!ensureAlways(IsValid(InventoryManagerComponent) && IsValid(ItemInstance)))
	{
		return;
	}

	if (InventoryManagerComponent->AddItem(ItemInstance))
	{
		Destroy();
	}
}
