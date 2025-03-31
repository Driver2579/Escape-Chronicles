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
	TArray<UActorComponent*> TaggedMeshes = GetOwner()->GetComponentsByTag(UMeshComponent::StaticClass(), HintMeshesTag);

	if (!ensureAlways(TaggedMeshes.Num() != 0))
	{
		return;
	}

	bool HasMeshWithGenerateOverlapEvents = false;
	
	for (UActorComponent* TaggedMesh : TaggedMeshes)
	{
		UMeshComponent* CastedMesh = Cast<UMeshComponent>(TaggedMesh);

		if (CastedMesh->GetGenerateOverlapEvents())
		{
			HasMeshWithGenerateOverlapEvents = true;
		}
		
		HintMeshes.Add(CastedMesh);
	}

#if DO_ENSURE
	ensureAlwaysMsgf(HasMeshWithGenerateOverlapEvents,
		TEXT("CastedMesh must be set GenerateOverlapEvents to true, otherwise it will not be included in "
		"UInteractionManagerComponent::InteractableComponentsPool."));
#endif
}

void UInteractableComponent::WidgetHintInitialize()
{
	UActorComponent* TaggedWidgetComponent = GetOwner()->FindComponentByTag(UWidgetComponent::StaticClass(), HintWidgetTag);

	if (!IsValid(TaggedWidgetComponent))
	{
		return;
	}

	const UWidgetComponent* CastedWidgetComponent = Cast<UWidgetComponent>(TaggedWidgetComponent);

	if (!ensureAlways(IsValid(CastedWidgetComponent)))
	{
		return;
	}

	UWidget* Widget = CastedWidgetComponent->GetWidget();
	
	if (!ensureAlways(IsValid(Widget)))
	{
		return;
	}
	
	UInteractPopupWidget* CastedWidget = Cast<UInteractPopupWidget>(Widget);

	if (!ensureAlways(CastedWidget))
	{
		return;
	}

	HintWidget = CastedWidget;
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
	if (HintWidget.IsValid())
	{
		Value ? HintWidget->ShowPopup() : HintWidget->HidePopup();
	}

	//Set mesh hint
	if (HintMeshes.Num() == 0)
	{
		return;
	}
	
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
