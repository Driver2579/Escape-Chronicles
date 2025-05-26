// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

class UInteractPopupWidget;
class UInteractionManagerComponent;

/**
 * Delegate called when interacting with the actor
 * @param InteractionManagerComponent Reference to the manager of the actor that call the event
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FInteractDelegate, UInteractionManagerComponent* InteractionManagerComponent);

// A component that makes an actor interactive
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class INTERACTIONSYSTEM_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	const FName& GetHintMeshTag() const { return HintMeshTag; }
	const FName& GetHintWidgetTag() const { return HintWidgetTag; }
	
	// Calls the interaction delegate (InteractDelegate)
	void Interact(UInteractionManagerComponent* InteractionManagerComponent) const;

	// Adds an interaction event handler (InteractDelegate)
	void AddInteractionHandler(const FInteractDelegate::FDelegate& Delegate);
	
	// Enables/disables the visibility of the interaction hint
	virtual void SetInteractionHintVisibility(const bool bNewVisibility);

	UPROPERTY(EditAnywhere)
	bool bCanInteraction;
	
protected:
	virtual void BeginPlay() override;

private:
	void InitializeHintMeshes();
	void InitializeHintWidget();
	
	// A delegate called when interacting with an actor
	FInteractDelegate InteractDelegate;

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
