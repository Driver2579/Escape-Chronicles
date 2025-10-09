// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Objects/InventoryItemFragment.h"
#include "DoorKeyInventoryItemFragment.generated.h"

class UDurabilityInventoryItemFragment;

// Fragment for key items that can unlock doors/containers
UCLASS()
class ESCAPECHRONICLES_API UDoorKeyInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	const FGameplayTagContainer& GetCompatibleAccessTags() const { return CompatibleAccessTags; }
	
	bool IsUsingDurability() const { return bUseDurability; }

	virtual bool IsValidConfiguration(UInventoryItemDefinition* ItemDefinition) override;

private:
	// Gameplay tags that specify compatible locks/doors (doors' KeyAccessTags)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	FGameplayTagContainer CompatibleAccessTags;

	// Whether using this key consumes durability
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	bool bUseDurability;
};
