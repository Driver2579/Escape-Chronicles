// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"
#include "Objects/InventoryItemFragment.h"
#include "DoorKeyInventoryItemFragment.generated.h"

// Fragment for key items that can unlock doors/containers
UCLASS()
class ESCAPECHRONICLES_API UDoorKeyInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	const FGameplayTagContainer& GetCompatibleAccessTags() const { return CompatibleAccessTags; }
	bool IsUseDurability() const { return bUseDurability; }

	virtual bool IsValidConfiguration(UInventoryItemDefinition* ItemDefinition) override
	{
		// Non-durability keys are always valid
		if (!bUseDurability)
		{
			return true;
		}

		// Durability keys must include durability fragment
		return ensureAlwaysMsgf(ItemDefinition->GetFragments().FindItemByClass<UDurabilityInventoryItemFragment>(),
			TEXT("If bUseDurability is true, the definition must include UDurabilityInventoryItemFragment"));
	}

private:
	// Gameplay tags that specify compatible locks/doors
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer CompatibleAccessTags;

	// Whether using this key consumes durability
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bUseDurability;
};
