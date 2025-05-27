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
	bReplicateUsingRegisteredSubObjectList = true;
	
	StaticMeshComponent= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SetRootComponent(StaticMeshComponent);
	StaticMeshComponent->SetSimulatePhysics(true);
}

void AInventoryPickupItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!HasAuthority())
	{
		return;
	}
		
	// Do not process during editing in blueprint
	if (!GetWorld()->HasBegunPlay() && !IsAsset())
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

void AInventoryPickupItem::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}
	
	check(bItemInstanceIsValid);

	if (!ItemInstance->IsInitialized())
	{
		ItemInstance->Initialize();
	}
	
	AddReplicatedSubObject(ItemInstance);
}

bool AInventoryPickupItem::ApplyChangesFromItemInstance() const
{
	if (!ensureAlways(IsValid(StaticMeshComponent)) || !IsValid(ItemInstance))
	{
		return false;
	}
	
	const UPickupInventoryItemFragment* PickupInventoryItemFragment =
		ItemInstance->GetFragmentByClass<UPickupInventoryItemFragment>();

	if (!IsValid(PickupInventoryItemFragment))
	{
		return false;
	}
	
	UStaticMesh* StaticMesh = PickupInventoryItemFragment->GetStaticMesh();
	
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

	if (ensureAlways(IsValid(DefaultObjectStaticMesh)))
	{
		StaticMeshComponent->SetStaticMesh(DefaultObjectStaticMesh);
	}
}

void AInventoryPickupItem::BreakItemInstance(UInventoryItemInstance* ItemInstancee)
{
	Destroy();
}

void AInventoryPickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemInstance);
}

void AInventoryPickupItem::OnRep_ItemInstance()
{
	bItemInstanceIsValid = ApplyChangesFromItemInstance();

	// While bItemInstanceIsValidSet is invalid, set the default settings
	if (!bItemInstanceIsValid)
	{
		SetDefaultSettings();
	}

	if (!ItemInstance->IsInitialized())
	{
		ItemInstance->Initialize();
	}
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
