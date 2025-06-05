// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Objects/InventoryManagerFragment.h"
#include "InventoryManagerDropItemsFragment.generated.h"

class AInventoryPickupItem;

// Allows to customize and call items to be dropped from the inventory
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerDropItemsFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	// Throws the item instance in the ThrowingDirection relative to the actor creating DropItemActorClass
	UFUNCTION(Server, Reliable)
	void Server_DropItem(const int32 SlotIndex, const FGameplayTag SlotsType);

private:
	// Class used for dropping away item instances
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AInventoryPickupItem> DropItemActorClass;

	// Used as an impulse when an object is ejected
	UPROPERTY(EditDefaultsOnly)
	FVector ThrowingDirection;
};
