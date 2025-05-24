// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Objects/InventoryItemFragments/InventoryItemFragment.h"
#include "DoorKeyInventoryItemFragment.generated.h"

// This fragment makes the item a key to the door
UCLASS()
class ESCAPECHRONICLES_API UDoorKeyInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	const FGameplayTagContainer& GetCompatibleAccessTags() const { return CompatibleAccessTags; }

	//void UseKey(UInventoryItemInstance* Instance) {}
	
private:
	// Indicates which doors this key can open
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer CompatibleAccessTags;

	// If True, takes away a unit of strength from the item
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bUseDurability;
};
