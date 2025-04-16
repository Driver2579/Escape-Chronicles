// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/EscapeChroniclesInventoryPickupItem.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AEscapeChroniclesInventoryPickupItem::AEscapeChroniclesInventoryPickupItem()
{
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));

	GetStaticMeshComponent()->ComponentTags.Add(InteractableComponent->GetMeshesHintTag());
}

void AEscapeChroniclesInventoryPickupItem::BeginPlay()
{
	Super::BeginPlay();

	InteractableComponent->AddInteractHandler(FOnInteractDelegate::FDelegate::CreateUObject(this,
		&AEscapeChroniclesInventoryPickupItem::InteractHandler));
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void AEscapeChroniclesInventoryPickupItem::InteractHandler(UInteractionManagerComponent* InteractionManagerComponent)
{
	if (!ensureAlways(IsValid(InteractableComponent)))
	{
		return;
	}
	
	const APawn* Pawn = Cast<APawn>(InteractionManagerComponent->GetOwner());

	if (!ensureAlways(IsValid(Pawn)))
	{
		return;
	}

	const AEscapeChroniclesPlayerState* PlayerState = Pawn->GetPlayerState<AEscapeChroniclesPlayerState>();

	if (!ensureAlways(IsValid(PlayerState)))
	{
		return;
	}
	
	UInventoryManagerComponent* InventoryManagerComponent =  PlayerState->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(InventoryManagerComponent)))
	{
		return;
	}
	
	Pickup(InventoryManagerComponent);
}
