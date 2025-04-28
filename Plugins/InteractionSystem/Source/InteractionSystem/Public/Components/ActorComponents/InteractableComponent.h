// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

class UInteractionManagerComponent;

/**
 * Delegate called when interacting with the actor
 * @param InteractionManagerComponent Reference to the manager of the actor that call the event
 */
	
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractDelegate, UInteractionManagerComponent* InteractionManagerComponent);
	
// A component that makes an actor interactive
UCLASS()
class INTERACTIONSYSTEM_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	FName GetHintMeshTag() const
	{
		return HintMeshTag;
	}

	FName GetHintWidgetTag() const
	{
		return HintWidgetTag;
	}
	
	// Calls the interaction delegate (OnInteract)
	void Interact(UInteractionManagerComponent* InteractionManagerComponent) const;

	// Adds an interaction event handler (OnInteract)
	void AddInteractionHandler(const FOnInteractDelegate::FDelegate& Callback);

	// Enables/disables the visibility of the interaction hint
	virtual void SetInteractionHintVisibility(const bool bNewVisibility);
	
protected:
	virtual void BeginPlay() override;

private:
	void InitializeHintMeshes();
	void InitializeHintWidget();
	
	// A delegate called when interacting with an actor
	FOnInteractDelegate OnInteract;

	// Tag to find meshes to hint when the interaction hint visibility is true
	UPROPERTY(EditAnywhere, Category="Hint")
	FName HintMeshTag = "HintMesh";

	// Tag to find widget that is visible when the interaction hint visibility is true
	UPROPERTY(EditAnywhere, Category="Hint")
	FName HintWidgetTag = "HintWidget";
	
	// Material applied to the meshes when the interaction hint visibility is true
	UPROPERTY(EditAnywhere, Category="Hint")
	TObjectPtr<UMaterialInterface> OverlayMaterialHint;

	// Widget that is visible when the interaction hint visibility is true
	TWeakObjectPtr<class UInteractPopupWidget> HintWidget;
	
	// Meshes to hint when the interaction hint visibility is true
	TArray<TWeakObjectPtr<UMeshComponent>> HintMeshes;
};
