// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/Actor.h"
#include "Objects/InventoryItemInstance.h"

#include "InventoryPickupItem.generated.h"

class UInventoryManagerComponent;

// Can be picked up into inventory
UCLASS()
class INVENTORYSYSTEM_API AInventoryPickupItem : public AActor
{
	GENERATED_BODY()

public:
	AInventoryPickupItem();

	/**
	 * Associates an actor with an instance of an item.
	 * @see Must be called before BeginPlay.
	 */
	void SetItemInstance(UInventoryItemInstance* InItemInstance)
	{
		if (ensureAlways(!HasActorBegunPlay()) && ensureAlways(IsValid(InItemInstance)))
		{
			ItemInstance = InItemInstance;
		}
	}

	UStaticMeshComponent* GetMesh() const { return MeshComponent; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
		override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	void Pickup(UInventoryManagerComponent* InventoryManagerComponent);

	/**
	 * Applies a change to this actor based on the current item instance
	 * @return True if all settings are applied correctly
	 */
	virtual bool ApplyChangesFromItemInstance() const;

	// Set the settings  mesh as default object
	virtual void SetDefaultSettings() const;

private:
	// An item instance this actor is associated with
	UPROPERTY(EditInstanceOnly, Instanced, ReplicatedUsing="OnRep_ItemInstance")
	TObjectPtr<UInventoryItemInstance> ItemInstance;

	// The mesh of this component will be replaced with the one associated with the item instance
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UFUNCTION()
	void OnRep_ItemInstance();

	/**
	 * Indicates whether an ItemInstance is valid for creating an AInventoryPickupItem. Updated in OnConstruction. Must
	 * be true before the BeginPlay is called.
	 */
	UPROPERTY()
	bool bValidItemInstance;
};
