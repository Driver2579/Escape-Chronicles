// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/EscapeChroniclesInventoryPickupItem.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "Subsystems/SaveInventoryPickupItemsSubsystem.h"

AEscapeChroniclesInventoryPickupItem::AEscapeChroniclesInventoryPickupItem()
{
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));

	GetMesh()->ComponentTags.Add(InteractableComponent->GetHintMeshTag());
}

void AEscapeChroniclesInventoryPickupItem::BeginPlay()
{
	Super::BeginPlay();

	USaveInventoryPickupItemsSubsystem* SaveInventoryPickupItemsSubsystem = GetWorld()->GetSubsystem<
		USaveInventoryPickupItemsSubsystem>();

	// Register the pickup item in the subsystem as required
	if (ensureAlways(IsValid(SaveInventoryPickupItemsSubsystem)))
	{
		SaveInventoryPickupItemsSubsystem->RegisterPickupItem(this);
	}

	InteractableComponent->OnInteract.AddUObject(this, &ThisClass::OnInteract);
}

void AEscapeChroniclesInventoryPickupItem::OnInteract(UInteractionManagerComponent* InteractionManagerComponent)
{
#if DO_CHECK
	check(IsValid(InteractionManagerComponent))
#endif

	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(
		InteractionManagerComponent->GetOwner());

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}
	
	UInventoryManagerComponent* InventoryManagerComponent = Character->GetInventoryManagerComponent();

	if (ensureAlways(IsValid(InventoryManagerComponent)))
	{
		Pickup(InventoryManagerComponent);
	}
}