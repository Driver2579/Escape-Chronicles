// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Objects/InventoryManagerFragment.h"
#include "InventoryManagerDropItemsFragment.generated.h"

class AInventoryPickupItem;

// Allows an item to be dropped from the inventory
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerDropItemsFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	// Throws the item instance in the ThrowingDirection relative to the actor creating DropItemActorClass
	UFUNCTION(Server, Reliable)
	void Server_DropItem(const int32 SlotIndex, const FGameplayTag& SlotsType);

private:
	// Class used for dropping away item instances
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AInventoryPickupItem> DropItemActorClass;

	// Impulse of the item when it's dropped. It will be multiplied with owner's rotation
	UPROPERTY(EditDefaultsOnly)
	FVector ThrowingImpulse;
};
