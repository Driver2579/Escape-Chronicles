// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

class UWidgetComponent;
class UInteractionManagerComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractDelegate, UInteractionManagerComponent*);

/**
 * Added to objects that can react to interaction (doors, objects, items, etc.).
 */
UCLASS()
class INTERACTIONSYSTEM_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractableComponent();

	void Interact(UInteractionManagerComponent* InteractionManagerComponent) const;
	void AddInteractHandler(const FOnInteractDelegate::FDelegate& Callback);

	virtual void ShowInteractionHint();
	virtual void HideInteractionHint();
	
protected:
	virtual void BeginPlay() override;

private:
	// Called every time the item is interacted with
	FOnInteractDelegate OnInteract;

	// Meshes with this tag will be a hint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hint", meta=(AllowPrivateAccess = "true"))
	FName HintMeshesTag = "HintMesh";

	// Widgets with this tag will be a hint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hint", meta=(AllowPrivateAccess = "true"))
	FName HintWidgetTag = "HintWidget";
	
	// Set as overlay material when showing hint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hint", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> HintOverlayMaterial;

	// Widget that has the visible parameter set when showing a hint
	TWeakObjectPtr<UWidgetComponent> HintWidget;
	
	// Mesh to which HintOverlayMaterial is set when showing a hint
	TArray<TWeakObjectPtr<UMeshComponent>> HintMeshes;

	// Adds or removes overlay material for showing a hint
	void SetHintOverlayMaterialForHintMeshes(bool Value);
};
