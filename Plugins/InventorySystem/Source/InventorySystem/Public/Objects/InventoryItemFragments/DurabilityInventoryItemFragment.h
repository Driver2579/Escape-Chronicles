// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Objects/InventoryItemFragments/InventoryItemFragment.h"
#include "DurabilityInventoryItemFragment.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UDurabilityInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	virtual void OnInstanceInitialized(UInventoryItemInstance* Instance) override;
	
	void ReduceDurability(UInventoryItemInstance* Instance, int32 Amount) const;
	
private:
	// 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 InitialDurability;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTag DurabilityDataTag;
};
