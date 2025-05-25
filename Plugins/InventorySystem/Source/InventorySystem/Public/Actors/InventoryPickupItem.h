// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/StoringItemInstances.h"
#include "Objects/InventoryItemInstance.h"

#include "InventoryPickupItem.generated.h"

class UInventoryManagerComponent;

// Can be picked up in inventory
UCLASS()
class INVENTORYSYSTEM_API AInventoryPickupItem : public AActor, public IStoringItemInstances
{
	GENERATED_BODY()
	
public:
	AInventoryPickupItem();

	void SetItemInstance(UInventoryItemInstance* InItemInstance)
	{
		if (!ensureAlways(!HasActorBegunPlay()))
		{
			return;
		}

		if (ensureAlways(IsValid(InItemInstance)))
		{
			ItemInstance = InItemInstance;
		}
	}
	
	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

	virtual void BreakItemInstance(UInventoryItemInstance* ItemInstancee) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
	void Pickup(UInventoryManagerComponent* InventoryManagerComponent);

	/**
	 * Applies a change to this actor based on the current Instance
	 * @return True if all settings are applied correctly
	 */
	virtual bool ApplyChangesFromItemInstance() const;

	// Set the settings  mesh as default object
	virtual void SetDefaultSettings() const;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditInstanceOnly, Instanced, ReplicatedUsing="OnRep_ItemInstance")
	TObjectPtr<UInventoryItemInstance> ItemInstance;

	UFUNCTION()
	void OnRep_ItemInstance();

	UPROPERTY()
	bool bItemInstanceIsValid;
};
