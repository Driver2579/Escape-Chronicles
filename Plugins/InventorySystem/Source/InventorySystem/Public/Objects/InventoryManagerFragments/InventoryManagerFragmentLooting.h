// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "InventoryManagerFragment.h"
#include "InventoryManagerFragmentLooting.generated.h"

class AInventoryPickupItem;

// TODO: DO DOCS
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerFragmentLooting : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void Server_Swap(UInventoryManagerComponent* Inventory, const int32 FromSlotIndex, const int32 ToSlotIndex,
		const FGameplayTag& FromSlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType,
		const FGameplayTag& ToSlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Loot(UInventoryManagerComponent* Inventory, UInventoryItemInstance* ItemInstance);

private:
	UPROPERTY(EditDefaultsOnly)
	float AllowedLootingDistance = 100;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer RequiredCharacterLootingTags;
};
