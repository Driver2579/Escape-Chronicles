// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/EscapeChroniclesInventoryPickupItem.h"

#include "Components/ActorComponents/InteractableComponent.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "Components/ActorComponents/InventoryManagerComponent.h"
#include "GameFramework/PlayerState.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AEscapeChroniclesInventoryPickupItem::AEscapeChroniclesInventoryPickupItem()
{
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));

	GetStaticMeshComponent()->ComponentTags.Add(InteractableComponent->GetMeshesHintTag());
}

void AEscapeChroniclesInventoryPickupItem::BeginPlay()
{
	Super::BeginPlay();

	InteractableComponent->AddInteractHandler(FOnInteractDelegate::FDelegate::CreateUObject(this, &AEscapeChroniclesInventoryPickupItem::Test));
}

void AEscapeChroniclesInventoryPickupItem::Test(UInteractionManagerComponent* InteractionManagerComponent)
{
	const APawn* Pawn = Cast<APawn>(InteractionManagerComponent->GetOwner());

	if (!ensureAlways(IsValid(Pawn)))
	{
		return;
	}

	APlayerState* PlayerState = Pawn->GetPlayerState();

	if (!ensureAlways(IsValid(PlayerState)))
	{
		return;
	}

	AEscapeChroniclesPlayerState* EscapeChroniclesPlayerState = Cast<AEscapeChroniclesPlayerState>(PlayerState);

	if (!ensureAlways(IsValid(EscapeChroniclesPlayerState)))
	{
		return;
	}
	
	UInventoryManagerComponent* InventoryManagerComponent =  EscapeChroniclesPlayerState->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(InventoryManagerComponent)))
	{
		return;
	}
	
	Pickup(InventoryManagerComponent);
}
