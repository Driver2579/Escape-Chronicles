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

	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;

	MeshComponent= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	MeshComponent->SetSimulatePhysics(true);

	SetRootComponent(MeshComponent); 
}

void AInventoryPickupItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Process only on the server and not during editing in blueprint
	if (!HasAuthority() || !GetWorld()->HasBegunPlay() && !IsAsset())
	{
		return;
	}

	bValidItemInstance = ApplyChangesFromItemInstance();

	// While bValidItemInstance is invalid, set the default settings
	if (!bValidItemInstance)
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

	if (ensureAlways(bValidItemInstance) && !ItemInstance->IsInitialized())
	{
		ItemInstance->Initialize();
	}
	
	AddReplicatedSubObject(ItemInstance);
}

bool AInventoryPickupItem::ApplyChangesFromItemInstance() const
{
	if (!ItemInstance)
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

	MeshComponent->SetStaticMesh(StaticMesh);

	return true;
}

void AInventoryPickupItem::SetDefaultSettings() const
{
	const AInventoryPickupItem* DefaultObject = GetClass()->GetDefaultObject<AInventoryPickupItem>();

	if (!ensureAlways(IsValid(DefaultObject)))
	{
		return;
	}

	const UStaticMeshComponent* DefaultObjectStaticMeshComponent = DefaultObject->GetMesh();
	
	if (!ensureAlways(IsValid(DefaultObjectStaticMeshComponent)))
	{
		return;
	}
	
	UStaticMesh* DefaultObjectStaticMesh = DefaultObjectStaticMeshComponent->GetStaticMesh();

	if (ensureAlways(IsValid(DefaultObjectStaticMesh)))
	{
		MeshComponent->SetStaticMesh(DefaultObjectStaticMesh);
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
	if (!ensureAlways(ItemInstance))
	{
		return;
	}

	bValidItemInstance = ApplyChangesFromItemInstance();

	// While bItemInstanceIsValidSet is invalid, set the default settings
	if (!bValidItemInstance)
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
	const bool bSuccess = ensureAlways(IsValid(InventoryManagerComponent)) && ensureAlways(ItemInstance) &&
		InventoryManagerComponent->AddItem(ItemInstance);

	if (bSuccess)
	{
		Destroy();
	}
}
