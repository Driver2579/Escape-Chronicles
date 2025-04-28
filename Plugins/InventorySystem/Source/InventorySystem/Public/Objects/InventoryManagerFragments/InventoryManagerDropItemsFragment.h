// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "InventoryManagerFragment.h"
#include "InventoryManagerDropItemsFragment.generated.h"

class AInventoryPickupItem;

UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerDropItemsFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void Server_DropItem(const int32 SlotIndex, const FGameplayTag SlotsType);

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AInventoryPickupItem> DropItemActorClass;

	UPROPERTY(EditDefaultsOnly)
	FVector ThrowingDirection;
};
