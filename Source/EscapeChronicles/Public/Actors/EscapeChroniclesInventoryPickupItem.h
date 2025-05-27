// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/InventoryPickupItem.h"
#include "EscapeChroniclesInventoryPickupItem.generated.h"

class UInteractionManagerComponent;
class UInteractableComponent;

// You can interact with this item to pick it up in your inventory
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
