// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "InteractionManagerComponent.generated.h"

class UInteractableComponent;

/**
 * Handles interaction logic: checking conditions, calling events.
 */
UCLASS()
class INTERACTIONSYSTEM_API UInteractionManagerComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent* GetSelectedInteractableComponent() const
	{
		return SelectedInteractableComponent.Get();
	}

	UInteractionManagerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Sends a request to attempt to interact with the server
	bool TryInteract();
	bool TryInteract(UInteractableComponent* InteractableComponent);
	
protected:
	virtual void BeginPlay() override;

private:
	// Selects the InteractableComponent that is closest to the direction of the view from InteractableComponentsPool
	void SelectInteractableComponent();

	// Add InteractableComponents to InteractableComponentsPool
	UFUNCTION()
	void OnAddToInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Delete InteractableComponents to InteractableComponentsPool
	UFUNCTION()
	void OnDeleteFromInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryInteract(UInteractableComponent* InteractableComponent);

	// The maximum distance the server handles for interaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(AllowPrivateAccess = "true"))
	float MaxInteractionDistance = 500.0f;
	
	TWeakObjectPtr<AController> OwnerController;
	TWeakObjectPtr<UInteractableComponent> SelectedInteractableComponent;
	TArray<TWeakObjectPtr<UInteractableComponent>> InteractableComponentsPool;
};
