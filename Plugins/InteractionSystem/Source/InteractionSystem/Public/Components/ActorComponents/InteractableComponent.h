// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

class UInteractPopupWidget;
class UInteractionManagerComponent;

// A component that makes an actor interactive
UCLASS()
class INTERACTIONSYSTEM_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	const FName& GetHintMeshTag() const { return HintMeshTag; }
	const FName& GetHintWidgetTag() const { return HintWidgetTag; }

	bool CanInteract() const { return bCanInteract; }
	void SetCanInteract(const bool bInbCanInteract) { bCanInteract = bInbCanInteract; }

	// Calls the interaction delegate (InteractDelegate)
	void Interact(UInteractionManagerComponent* InteractionManagerComponent) const;

	// Enables/disables the visibility of the interaction hint
	virtual void SetInteractionHintVisibility(const bool bNewVisibility);

	/**
	 * Delegate called when interacting with the actor
	 * @param InteractionManagerComponent Reference to the manager of the actor that call the event
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractDelegate, UInteractionManagerComponent* InteractionManagerComponent);

	// A delegate called when interacting with an actor
	FOnInteractDelegate OnInteract;

protected:
	virtual void BeginPlay() override;

private:
	void InitializeHintMeshes();
	void InitializeHintWidget();

	// Whether interaction is possible
	UPROPERTY(EditAnywhere)
	bool bCanInteract;
	
	// Tag to find meshes to hint when the interaction hint visibility is true
	UPROPERTY(EditAnywhere, Category="Hint")
	FName HintMeshTag = TEXT("HintMesh");

	// Tag to find widget that is visible when the interaction hint visibility is true
	UPROPERTY(EditAnywhere, Category="Hint")
	FName HintWidgetTag = TEXT("HintWidget");
	
	// Material applied to the meshes when the interaction hint visibility is true
	UPROPERTY(EditAnywhere, Category="Hint")
	TObjectPtr<UMaterialInterface> OverlayMaterialHint;

	// Widget that is visible when the interaction hint visibility is true
	TWeakObjectPtr<UInteractPopupWidget> HintWidget;
	
	// Meshes to hint when the interaction hint visibility is true
	TArray<TWeakObjectPtr<UMeshComponent>> HintMeshes;
};
