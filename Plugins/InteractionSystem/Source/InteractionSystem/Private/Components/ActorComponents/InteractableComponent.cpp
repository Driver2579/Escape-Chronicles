// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionSystem/Public/Components/ActorComponents/InteractableComponent.h"

#include "Components/Widget.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	
	HintMesh = Owner->FindComponentByClass<UMeshComponent>();

	ensureAlwaysMsgf(HintMesh->GetGenerateOverlapEvents(),
		TEXT("The owner of this class must set GenerateOverlapEvents to true, otherwise it will not be "
	   "included in UInteractionManagerComponent::InteractableComponentsPool."));
}

void UInteractableComponent::Interact(UInteractionManagerComponent* InteractionManagerComponent) const
{
	OnInteract.Broadcast(InteractionManagerComponent);
}

void UInteractableComponent::AddInteractHandler(const FOnInteractDelegate::FDelegate& Callback)
{
	OnInteract.Add(Callback);
}

void UInteractableComponent::ShowInteractionHint()
{
	if (IsValid(HintWidget))
	{
		HintWidget->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (ensureAlways(HintMesh.IsValid()) && IsValid(HintOverlayMaterial))
	{
		HintMesh->SetOverlayMaterial(HintOverlayMaterial);
	}
}

void UInteractableComponent::HideInteractionHint()
{
	if (IsValid(HintWidget))
	{
		HintWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (ensureAlways(HintMesh.IsValid()))
	{
		HintMesh->SetOverlayMaterial(nullptr);
	}
}
