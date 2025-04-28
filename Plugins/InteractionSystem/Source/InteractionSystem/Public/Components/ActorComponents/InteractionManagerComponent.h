// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "InteractionManagerComponent.generated.h"

class UInteractableComponent;

/**
 * Handles pawn interaction logic by detecting, selecting, and processing interactions with nearby interactable objects.
 * Uses overlap detection to maintain a pool of nearby interactables and automatically selects the best candidate.
 * Must attach at least one UPrimitiveComponent for selection.
 */
UCLASS()
class INTERACTIONSYSTEM_API UInteractionManagerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UInteractionManagerComponent();
	
	UInteractableComponent* GetSelectedInteractableComponent() const
	{
		return SelectedInteractableComponent.Get();
	}

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Attempts interaction with currently selected interactable
	 * @return True if interaction was successful
	 */
	bool TryInteract();

	/**
	 * Attempts interaction with specific interactable component
	 * @param InteractableComponent Target component to interact with
	 * @return True if interaction was successful
	 */
	bool TryInteract(UInteractableComponent* InteractableComponent);
	
protected:
	virtual void BeginPlay() override;

private:
	/**
	 * Checks if there are obstacles between this component and target interactable
	 * @param InteractableComponent Target interactable to check
	 * @return True if path is obstructed
	 */
	bool IsPathObstructed(const UInteractableComponent* InteractableComponent) const;

	/**
	 * Selects the most suitable interactable component from the current pool
	 * Considers view direction and obstacles
	 */
	void SelectInteractableComponent();
	
	UFUNCTION()
	void OnAddToInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDeleteFromInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
	 * Server-side interaction validation and execution
	 * @param InteractableComponent Component to interact with
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryInteract(UInteractableComponent* InteractableComponent);

	// Maximum distance at which interaction is possible
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(AllowPrivateAccess = "true", ClampMin=0,
		UIMin=0, ForceUnits="cm"))
	float MaxInteractionDistance = 500.0f;

	// Controller that owns this interaction component 
	TWeakObjectPtr<AController> OwnerController;

	// Pool of all interactable components currently in detection range 
	TArray<TWeakObjectPtr<UInteractableComponent>> InteractableComponentsPool;

	// Currently selected/focused interactable component (the best candidate from pool) 
	TWeakObjectPtr<UInteractableComponent> SelectedInteractableComponent;

	bool bIsLocallyControlled = false;
};
