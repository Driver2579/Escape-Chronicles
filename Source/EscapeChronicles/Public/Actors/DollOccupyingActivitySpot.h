// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DollOccupyingActivitySpot.generated.h"

class UInventoryItemInstance;

/**
 * A doll that can be used to occupy the activity spot. It can be picked up into the inventory when it's occupying the
 * activity spot.
 */
UCLASS()
class ESCAPECHRONICLES_API ADollOccupyingActivitySpot : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADollOccupyingActivitySpot();

	UStaticMeshComponent* GetMesh() const { return MeshComponent; }

	const UInventoryItemInstance* GetItemInstance() const { return ItemInstance; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// An item instance this doll is associated with
	UPROPERTY(EditDefaultsOnly, Instanced)
	TObjectPtr<UInventoryItemInstance> ItemInstance;
};