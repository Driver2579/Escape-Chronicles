// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/InventoryPickupItem.h"
#include "EscapeChroniclesInventoryPickupItem.generated.h"

class UInteractionManagerComponent;
class UInteractableComponent;

// Character can interact with this item to pick it up in the inventory
UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesInventoryPickupItem : public AInventoryPickupItem
{
	GENERATED_BODY()

public:
	AEscapeChroniclesInventoryPickupItem();
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInteractableComponent> InteractableComponent;

	void OnInteract(UInteractionManagerComponent* InteractionManagerComponent);
};
