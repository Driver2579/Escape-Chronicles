// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/StoringItemInstances.h"
#include "Objects/InventoryItemInstance.h"
#include "InventoryPickupItem.generated.h"

class UInventoryManagerComponent;

/**
 * Physical representation of inventory item instance in game world (ItemInstance must be set before BeginPlay)
 * - Spawnable pickup actor that holds ItemInstance data.
 * - Automatically updates visual representation (mesh) from item data.
 * - Handles pickup interaction and inventory transfer.
 */
UCLASS()
class INVENTORYSYSTEM_API AInventoryPickupItem : public AActor, public IStoringItemInstances
{
	GENERATED_BODY()

public:
	AInventoryPickupItem();

	UInventoryItemInstance* GetItemInstance() const { return ItemInstance; }
	UStaticMeshComponent* GetMesh() const { return MeshComponent; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	// Associates an actor with an instance of an item. Must be called before BeginPlay.
	void SetItemInstance(UInventoryItemInstance* InItemInstance)
	{
#if DO_ENSURE
		ensureAlways(!HasActorBegunPlay());
		ensureAlways(IsValid(InItemInstance));
#endif

		ItemInstance = InItemInstance;
	}

	// Transfers item to inventory and destroys actor
	void Pickup(UInventoryManagerComponent* InventoryManagerComponent);

	virtual void BreakItemInstance(UInventoryItemInstance* ItemInstancee) override;
	
protected:
	virtual void BeginPlay() override;

	/**
	 * Applies a change to this actor based on the current item instance (sets mesh as an item instance. But can be
	 * overriden).
	 * @return True if all settings are applied correctly.
	 */
	virtual bool ApplyChangesFromItemInstance() const;

	// Reverts settings to CDO (opposite of ApplyChangesFromItemInstance)
	virtual void SetDefaultSettings() const;

	// Like ApplyChangesFromItemInstance, but at false additionally applies SetDefaultSettings
	void TryApplyChangesFromItemInstance() const;

private:
	// An item instance this actor is associated with
	UPROPERTY(EditInstanceOnly, Instanced, ReplicatedUsing="OnRep_ItemInstance")
	TObjectPtr<UInventoryItemInstance> ItemInstance;

	// The mesh of this component will be replaced with the one associated with the item instance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UFUNCTION()
	void OnRep_ItemInstance();
};
