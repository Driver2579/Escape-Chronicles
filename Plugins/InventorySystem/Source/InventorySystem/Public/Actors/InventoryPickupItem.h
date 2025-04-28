// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/Actor.h"
#include "Objects/InventoryItemInstance.h"

#include "InventoryPickupItem.generated.h"

class UInventoryManagerComponent;

// Can be picked up in inventory
UCLASS()
class INVENTORYSYSTEM_API AInventoryPickupItem : public AActor
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
	
	UStaticMeshComponent* GetStaticMeshComponent() const
	{
		return StaticMeshComponent;
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
		override;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void Tick(float DeltaSeconds) override;
	
	void Pickup(UInventoryManagerComponent* InventoryManagerComponent);
	
private:
	/**
	 * Applies a change to this actor based on the current Instance
	 * @return True if all settings are applied correctly
	 */
	bool ApplyChangesFromItemInstance() const;

	// Set the same mesh as default object
	void SetDefaultSettings() const;

	UPROPERTY()
	bool bItemInstanceIsValid;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditInstanceOnly, Instanced, ReplicatedUsing="OnRep_ItemInstance")
	TObjectPtr<UInventoryItemInstance> ItemInstance;

	UFUNCTION()
	void OnRep_ItemInstance();
};
