// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/InventoryPickupItem.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemInstance.h"
#include "Objects/InventoryItemFragments/PickupInventoryItemFragment.h"

AInventoryPickupItem::AInventoryPickupItem()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetSimulatePhysics(true);

	SetRootComponent(MeshComponent); 
}

void AInventoryPickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemInstance);
}

bool AInventoryPickupItem::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool bParentResult = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (ItemInstance)
	{
		bParentResult |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
	}
	
	return bParentResult;
}

void AInventoryPickupItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	/**
	 * Apply item changes only on server after the world has begun play (it's the first object that begins play), or
	 * for assets
	 */
	if (HasAuthority() && GetWorld()->HasBegunPlay() || IsAsset())
	{
		TryApplyChangesFromItemInstance();
	}
}

void AInventoryPickupItem::BeginPlay()
{
	Super::BeginPlay();

#if DO_CHECK
	check(ItemInstance)
#endif

	if (HasAuthority() && !ItemInstance->IsInitialized())
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

	// === Set the required mesh ===

	// Gets the UPickupInventoryItemFragment to get a suitable mesh
	const UPickupInventoryItemFragment* PickupInventoryItemFragment =
		ItemInstance->GetFragmentByClass<UPickupInventoryItemFragment>();

	if (!IsValid(PickupInventoryItemFragment))
	{
		return false;
	}

	UStaticMesh* StaticMesh = PickupInventoryItemFragment->GetMesh();

	if (!IsValid(StaticMesh))
	{
		return false;
	}

	MeshComponent->SetStaticMesh(StaticMesh);

	return true;
}

void AInventoryPickupItem::SetDefaultSettings() const
{
	const AInventoryPickupItem* PickupItemCDO = GetClass()->GetDefaultObject<AInventoryPickupItem>();

	if (!ensureAlways(IsValid(PickupItemCDO)))
	{
		return;
	}

	// === Set up a standard mesh ===

	const UStaticMeshComponent* PickupItemCDOMeshComponent = PickupItemCDO->GetMesh();
	
	if (!ensureAlways(IsValid(PickupItemCDOMeshComponent)))
	{
		return;
	}
	
	UStaticMesh* PickupItemCDOMesh = PickupItemCDOMeshComponent->GetStaticMesh();

	if (ensureAlways(IsValid(PickupItemCDOMesh)))
	{
		MeshComponent->SetStaticMesh(PickupItemCDOMesh);
	}
}

void AInventoryPickupItem::BreakItemInstance(UInventoryItemInstance* ItemInstancee)
{
	Destroy();
}

void AInventoryPickupItem::TryApplyChangesFromItemInstance() const
{
	// Try to apply the new settings and fall back to the default ones if failed to apply the new ones
	if (!ApplyChangesFromItemInstance())
	{
		SetDefaultSettings();
	}
}

void AInventoryPickupItem::OnRep_ItemInstance()
{
	TryApplyChangesFromItemInstance();
}

void AInventoryPickupItem::Pickup(UInventoryManagerComponent* InventoryManagerComponent)
{
#if DO_CHECK
	check(ItemInstance)
	check(IsValid(InventoryManagerComponent))
#endif

#if DO_ENSURE
	ensureAlways(HasAuthority());
#endif
	
	if (InventoryManagerComponent->AddItem(ItemInstance))
	{
		Destroy();
	}
}
