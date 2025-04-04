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
	UStaticMeshComponent* GetStaticMeshComponent() const
	{
		return StaticMeshComponent;
	}
	
	AInventoryPickupItem();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void Pickup(UInventoryManagerComponent* InventoryManagerComponent);
	
private:
	// Set a mesh to this actor from the specified ItemInstance (ItemInstance must have a UPickupInventoryItemFragment)
	bool ApplyChangesFromItemInstance() const;

	// Set the same mesh as CDO
	void SetDefaultStaticMesh() const;

	UPROPERTY()
	bool bItemInstanceIsValid;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditInstanceOnly, Instanced, Replicated)
	TObjectPtr<class UInventoryItemInstance> ItemInstance;
};
