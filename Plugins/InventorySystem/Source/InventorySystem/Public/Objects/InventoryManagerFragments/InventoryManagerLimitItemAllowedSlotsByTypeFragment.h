// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryManagerFragment.h"
#include "InventoryManagerLimitItemAllowedSlotsByTypeFragment.generated.h"

// A simple wrapper for an array of item fragments classes
USTRUCT()
struct FInventoryItemFragmentsClassesContainer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TSoftClassPtr<UInventoryItemFragment>> ItemFragmentsClasses;
};

// A fragment that limits the items that can be placed in slots of a specific type based on their fragments
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerLimitItemAllowedSlotsByTypeFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	virtual bool CanEquipItemInSlot(const UInventoryItemInstance* Item, const FGameplayTag& SlotTypeTag,
		const int32 SlotIndex) const override;

private:
	/**
	 * Slot types in this map will only allow receiving items that have all the specified item fragments. Slot types
	 * that are not present in this map will allow all items.
	 * @tparam KeyType A tag of the slot type.
	 * @tparam ValueType Fragments that the item must have to be added to the slot of the specified type.
	 */
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, FInventoryItemFragmentsClassesContainer> AllowedItemFragmentsForSlotTypes;
};