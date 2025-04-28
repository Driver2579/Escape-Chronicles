// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/InventoryPickupItem.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemInstance.h"
#include "Objects/InventoryItemFragments/PickupInventoryItemFragment.h"

// Sets default values
AInventoryPickupItem::AInventoryPickupItem()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	
	StaticMeshComponent= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SetRootComponent(StaticMeshComponent);
}

void AInventoryPickupItem::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}
	
	if (!ensureAlways(IsValid(ItemInstance)))
	{
		return;
	}
	
	check(bItemInstanceIsValid);

	if (!ItemInstance->IsInitialized())
	{
		ItemInstance->Initialize();
	}

}

void AInventoryPickupItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!HasAuthority())
	{
		return;
	}
		
	// Only update actors on scene
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

void AInventoryPickupItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AInventoryPickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemInstance);
}

bool AInventoryPickupItem::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
			
	if (IsValid(ItemInstance))
	{
		bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
	}
	
	return bWroteSomething;
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
