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

	MeshesHintInitialize();
	WidgetHintInitialize();
}

void UInteractableComponent::MeshesHintInitialize()
{
	TInlineComponentArray<UMeshComponent*> MeshComponents;
	GetOwner()->GetComponents(UMeshComponent::StaticClass(), MeshComponents);
	
	bool HasMeshWithGenerateOverlapEvents = false;
	
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent->ComponentHasTag(MeshesHintTag))
		{
			continue;
		}
		
		if (MeshComponent->GetGenerateOverlapEvents())
		{
			HasMeshWithGenerateOverlapEvents = true;
		}
		
		MeshHints.Add(MeshComponent);
	}

#if DO_ENSURE
	ensureAlwaysMsgf(HasMeshWithGenerateOverlapEvents,
		TEXT("CastedMesh must be set GenerateOverlapEvents to true, otherwise it will not be included in "
		"UInteractionManagerComponent::InteractableComponentsPool."));
#endif
}

void UInteractableComponent::WidgetHintInitialize()
{
	const UWidgetComponent* WidgetComponent = GetOwner()->FindComponentByTag<UWidgetComponent>(WidgetHintTag);
	
	if (!IsValid(WidgetComponent))
	{
		return;
	}

	UWidget* Widget = WidgetComponent->GetWidget();
	
	if (!ensureAlways(IsValid(Widget)))
	{
		return;
	}
	
	UInteractPopupWidget* CastedWidget = Cast<UInteractPopupWidget>(Widget);

	if (!ensureAlways(CastedWidget))
	{
		return;
	}

	WidgetHint = CastedWidget;
}

void UInteractableComponent::Interact(UInteractionManagerComponent* InteractionManagerComponent) const
{
	OnInteract.Broadcast(InteractionManagerComponent);
}

void UInteractableComponent::AddInteractHandler(const FOnInteractDelegate::FDelegate& Callback)
{
	OnInteract.Add(Callback);
}

void UInteractableComponent::SetInteractionHintVisible(const bool Value)
{
	// Set widget hint
	if (WidgetHint.IsValid())
	{
		Value ? WidgetHint->ShowPopup() : WidgetHint->HidePopup();
	}

	//Set mesh hint
	if (MeshHints.Num() == 0)
	{
		return;
	}
	
	if (!ensureAlways(IsValid(OverlayMaterialHint)))
	{
		return;
	}
	
	UMaterialInterface* CurrentOverlayMaterial = Value ? OverlayMaterialHint : nullptr;
	
	for (TWeakObjectPtr<UMeshComponent> Mesh : MeshHints)
	{
		if (!ensureAlways(Mesh.IsValid()))
		{
			continue;
		}
		
		Mesh->SetOverlayMaterial(CurrentOverlayMaterial);
	}
}
