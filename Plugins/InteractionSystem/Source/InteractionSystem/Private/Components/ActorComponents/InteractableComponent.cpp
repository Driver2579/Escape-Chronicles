// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionSystem/Public/Components/ActorComponents/InteractableComponent.h"

#include "Components/WidgetComponent.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	// Filling HintMeshes array
	TArray<UActorComponent*> TaggedMeshes = GetOwner()->GetComponentsByTag(UMeshComponent::StaticClass(), HintMeshesTag);

	for (UActorComponent* TaggedMesh : TaggedMeshes)
	{
		UMeshComponent* CastedMesh = Cast<UMeshComponent>(TaggedMesh);

#if DO_ENSURE
		if (!ensureAlways(IsValid(CastedMesh)))
		{
			continue;
		}

		ensureAlwaysMsgf(CastedMesh->GetGenerateOverlapEvents(),
			TEXT("CastedMesh must set GenerateOverlapEvents to true, otherwise it will not be included in "
			"UInteractionManagerComponent::InteractableComponentsPool."));
#endif
		
		HintMeshes.Add(CastedMesh);
	}

	// Find widget
	UActorComponent* TaggedWidget = GetOwner()->FindComponentByTag(UWidgetComponent::StaticClass(), HintWidgetTag);

	if (!IsValid(TaggedWidget))
	{
		return;
	}

	UWidgetComponent* CastedWidget = Cast<UWidgetComponent>(TaggedWidget);

	if (!ensureAlways(IsValid(CastedWidget)))
	{
		return;
	}

	HintWidget = CastedWidget;

	if (!ensureAlways(IsValid(HintWidget->GetWidget())))
	{
		return;
	}
	
	HintWidget->GetWidget()->SetVisibility(ESlateVisibility::Hidden);
}

void UInteractableComponent::Interact(UInteractionManagerComponent* InteractionManagerComponent) const
{
	OnInteract.Broadcast(InteractionManagerComponent);
}

void UInteractableComponent::AddInteractHandler(const FOnInteractDelegate::FDelegate& Callback)
{
	OnInteract.Add(Callback);
}

void UInteractableComponent::SetHintOverlayMaterialForHintMeshes(const bool Value)
{
	if (!ensureAlways(IsValid(HintOverlayMaterial)))
	{
		return;
	}
	
	UMaterialInterface* CurrentOverlayMaterial = Value ? HintOverlayMaterial : nullptr;
	
	for (TWeakObjectPtr<UMeshComponent> Mesh : HintMeshes)
	{
		if (!ensureAlways(Mesh.IsValid()))
		{
			continue;
		}
		
		Mesh->SetOverlayMaterial(CurrentOverlayMaterial);
	}
}

void UInteractableComponent::ShowInteractionHint()
{
	if (HintWidget.IsValid())
	{
		HintWidget->GetWidget()->SetVisibility(ESlateVisibility::Visible);
	}
	
	SetHintOverlayMaterialForHintMeshes(true);
}

void UInteractableComponent::HideInteractionHint()
{
	if (HintWidget.IsValid())
	{
		HintWidget->GetWidget()->SetVisibility(ESlateVisibility::Hidden);
	}
	
	SetHintOverlayMaterialForHintMeshes(false);
}
