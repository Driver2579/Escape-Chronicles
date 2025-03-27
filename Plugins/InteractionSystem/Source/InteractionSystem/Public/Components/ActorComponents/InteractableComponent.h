// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

class UInteractionManagerComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractDelegate, UInteractionManagerComponent*);

/**
 * Added to objects that can react to interaction (doors, objects, items, etc).
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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
	
	// Set as overlay material when showing hint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hint", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> HintOverlayMaterial;

	// Displayed when showing hint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hint", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<class UWidget> HintWidget;

	// Mesh to which HintOverlayMaterial is set when showing a hint
	UPROPERTY()
	TWeakObjectPtr<UMeshComponent> HintMesh;
};
