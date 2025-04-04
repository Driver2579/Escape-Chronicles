// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

class UInteractPopupWidget;
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
	FName GetMeshesHintTag() const 
	{
		return MeshesHintTag;
	}

	FName GetWidgetHintTag() const
	{
		return WidgetHintTag;
	}
	
	UInteractableComponent();

	void Interact(UInteractionManagerComponent* InteractionManagerComponent) const;
	void AddInteractHandler(const FOnInteractDelegate::FDelegate& Callback);

	virtual void SetInteractionHintVisible(bool Value);
	
protected:
	virtual void BeginPlay() override;

private:
	void MeshesHintInitialize();
	void WidgetHintInitialize();
	
	// Called every time the item is interacted with
	FOnInteractDelegate OnInteract;

	// Meshes with this tag will be a hint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hint", meta=(AllowPrivateAccess = "true"))
	FName MeshesHintTag = "MeshHint";

	// Widgets with this tag will be a hint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hint", meta=(AllowPrivateAccess = "true"))
	FName WidgetHintTag = "WidgetHint";
	
	// Set as overlay material when showing hint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hint", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> OverlayMaterialHint;

	// Widget that has the visible parameter set when showing a hint
	TWeakObjectPtr<UInteractPopupWidget> WidgetHint;
	
	// Mesh to which OverlayMaterialHint is set when showing a hint
	TArray<TWeakObjectPtr<UMeshComponent>> MeshHints;
};
