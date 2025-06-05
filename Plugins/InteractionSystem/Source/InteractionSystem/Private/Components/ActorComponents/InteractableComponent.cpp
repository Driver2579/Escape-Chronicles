// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionSystem/Public/Components/ActorComponents/InteractableComponent.h"

#include "Components/WidgetComponent.h"
#include "Widgets/InteractPopupWidget.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeHintMeshes();
	InitializeHintWidget();
}

void UInteractableComponent::InitializeHintMeshes()
{
	TInlineComponentArray<UMeshComponent*> MeshComponents;
	GetOwner()->GetComponents(UMeshComponent::StaticClass(), MeshComponents);
	
	bool HasMeshWithGenerateOverlapEvents = false;
	
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent->ComponentHasTag(HintMeshTag))
		{
			continue;
		}
		
		if (MeshComponent->GetGenerateOverlapEvents())
		{
			HasMeshWithGenerateOverlapEvents = true;
		}
		
		HintMeshes.Add(MeshComponent);
	}

#if DO_ENSURE
	ensureAlwaysMsgf(HasMeshWithGenerateOverlapEvents,
		TEXT("CastedMesh must be set GenerateOverlapEvents to true, otherwise it will not be included in "
			"UInteractionManagerComponent::InteractableComponentsPool!"));
#endif
}

void UInteractableComponent::InitializeHintWidget()
{
	const UWidgetComponent* WidgetComponent = GetOwner()->FindComponentByTag<UWidgetComponent>(HintWidgetTag);
	
	if (!IsValid(WidgetComponent))
	{
		return;
	}

	UInteractPopupWidget* Widget = Cast<UInteractPopupWidget>(WidgetComponent->GetWidget());

	if (ensureAlways(IsValid(Widget)))
	{
		HintWidget = Widget;
	}
}

void UInteractableComponent::Interact(UInteractionManagerComponent* InteractionManagerComponent) const
{
	InteractDelegate.Broadcast(InteractionManagerComponent);
}

void UInteractableComponent::AddInteractHandler(const FOnInteractDelegate::FDelegate& Delegate)
{
	InteractDelegate.Add(Delegate);
}

void UInteractableComponent::SetInteractionHintVisibility(const bool bNewVisibility)
{
	// === Set widget hint ===
	
	if (HintWidget.IsValid())
	{
		bNewVisibility ? HintWidget->ShowPopup() : HintWidget->HidePopup();
	}

	// === Set mesh hint ===
	
	if (HintMeshes.Num() == 0 || !ensureAlways(IsValid(OverlayMaterialHint)))
	{
		return;
	}
	
	UMaterialInterface* CurrentOverlayMaterial = bNewVisibility ? OverlayMaterialHint : nullptr;
	
	for (TWeakObjectPtr<UMeshComponent> Mesh : HintMeshes)
	{
		if (ensureAlways(Mesh.IsValid()))
		{
			Mesh->SetOverlayMaterial(CurrentOverlayMaterial);
		}
	}
}
