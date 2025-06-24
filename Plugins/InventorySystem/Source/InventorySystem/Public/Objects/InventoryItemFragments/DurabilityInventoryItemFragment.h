// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Objects/InventoryItemFragment.h"
#include "DurabilityInventoryItemFragment.generated.h"

// Item fragment class responsible for durability system functionality
UCLASS()
class INVENTORYSYSTEM_API UDurabilityInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	virtual void OnItemInstanceInitialized(UInventoryItemInstance* Instance) override;

	/**
	 * Function to reduce item's durability.
	 * @param ItemInstance the item instance whose durability is being reduced.
	 * @param Amount how much durability to subtract.
	 */
	void ReduceDurability(UInventoryItemInstance* ItemInstance, int32 Amount = 1) const;

private:
	// Initial durability value for items of this type
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 InitialDurability;

	// Gameplay tag identifying the durability stat in its stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTag DurabilityStatTag;
};
