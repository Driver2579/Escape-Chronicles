// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventoryPickupItem.generated.h"

class UInventoryManagerComponent;

/**
 * Can be picked up in inventory
 */
UCLASS()
class INVENTORYSYSTEM_API AInventoryPickupItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AInventoryPickupItem();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	void Pickup(UInventoryManagerComponent* InventoryManagerComponent);

	UStaticMeshComponent* GetStaticMeshComponent() const
	{
		return StaticMeshComponent;
	}
	
private:
	// Sets a mesh to this actor from the specified ItemInstance (ItemInstance must have a UPickupInventoryItemFragment)
	void UpdateMesh() const;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditAnywhere, Instanced, Replicated)
	TObjectPtr<class UInventoryItemInstance> ItemInstance;
};
