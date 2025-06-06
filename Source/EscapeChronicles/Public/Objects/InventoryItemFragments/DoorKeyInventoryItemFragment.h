// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"
#include "Objects/InventoryItemFragment.h"
#include "DoorKeyInventoryItemFragment.generated.h"

// This fragment makes the item a key to the door
UCLASS()
class ESCAPECHRONICLES_API UDoorKeyInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	const FGameplayTagContainer& GetCompatibleAccessTags() const { return CompatibleAccessTags; }
	bool IsUseDurability() const { return bUseDurability; }

	virtual bool IsValidConfiguration(UInventoryItemDefinition* ItemDefinition) override
	{
		if (!bUseDurability)
		{
			return true;
		}

		return ensureAlwaysMsgf(ItemDefinition->GetFragments().FindItemByClass<UDurabilityInventoryItemFragment>(),
			TEXT("If bUseDurability is true, the definition must include UDurabilityInventoryItemFragment"));
	}
	
private:
	// Indicates which doors this key can open
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer CompatibleAccessTags;

	// If True, takes away a unit of strength from the item
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bUseDurability;
};
