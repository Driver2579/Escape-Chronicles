// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/InventoryPickupItem.h"
#include "EscapeChroniclesInventoryPickupItem.generated.h"

class UInteractionManagerComponent;
class UInteractableComponent;

/**
 * Pickup item in the game
 */
UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesInventoryPickupItem : public AInventoryPickupItem
{
	GENERATED_BODY()

public:
	AEscapeChroniclesInventoryPickupItem();
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInteractableComponent> InteractableComponent;

	void InteractHandler (UInteractionManagerComponent* InteractionManagerComponent);
};
