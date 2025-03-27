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
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INTERACTIONSYSTEM_API UInteractionManagerComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent* GetSelectedInteractableComponent() const
	{
		return SelectedInteractableComponent.Get();
	}
	
	float GetDistanceToSelectedInteractableComponent() const
	{
		return FVector::Distance(SelectedInteractableComponent.Get()->GetOwner()->GetActorLocation(),
			OwnerController->GetOwner()->GetActorLocation());
	}

	UInteractionManagerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	bool TryInteract(UInteractableComponent* Interactable);
	
protected:
	virtual void BeginPlay() override;

private:	
	void TraceForInteractables();

	UFUNCTION()
	void OnAddToInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDeleteFromInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryInteract(UInteractableComponent* Interactable);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(AllowPrivateAccess = "true"))
	float InteractionDistance = 500.0f;

	TWeakObjectPtr<AController> OwnerController;
	TWeakObjectPtr<UInteractableComponent> SelectedInteractableComponent;
	TArray<TWeakObjectPtr<UInteractableComponent>> InteractableComponentsPool;
};
