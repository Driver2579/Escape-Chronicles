// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/InventoryPickupItem.h"

#include "NaniteSceneProxy.h"
#include "Components/ActorComponents/InventoryManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemDefinition.h"
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

	ensureAlways(bItemInstanceIsValid);
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

	// Until the valid parameters for installing a personal mesh are selected, install the standard one
	if (!bItemInstanceIsValid)
	{
		SetDefaultStaticMesh();
	}
}

void AInventoryPickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemInstance);
}

bool AInventoryPickupItem::ApplyChangesFromItemInstance() const
{
	if (!IsValid(StaticMeshComponent) || !IsValid(ItemInstance))
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

void AInventoryPickupItem::SetDefaultStaticMesh() const
{
	if (!IsValid(StaticMeshComponent))
	{
		return;
	}

	const AInventoryPickupItem* CDO = Cast<AInventoryPickupItem>(GetClass()->GetDefaultObject());

	if (!IsValid(CDO))
	{
		return;
	}

	const UStaticMeshComponent* StaticMeshComponentCDO = CDO->GetStaticMeshComponent();
	
	if (!IsValid(StaticMeshComponentCDO))
	{
		return;
	}
	
	UStaticMesh* StaticMeshCDO = StaticMeshComponentCDO->GetStaticMesh();

	if (!IsValid(StaticMeshCDO))
	{
		return;
	}

	StaticMeshComponent->SetStaticMesh(StaticMeshCDO);
}

// TODO: Refactor
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
