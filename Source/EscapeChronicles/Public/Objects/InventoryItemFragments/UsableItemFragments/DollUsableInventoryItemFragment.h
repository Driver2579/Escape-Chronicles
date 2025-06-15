// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UsableItemInventoryItemFragment.h"
#include "DollUsableInventoryItemFragment.generated.h"

class ADollOccupyingActivitySpot;
class AActivitySpot;

/**
 * A doll can be used to put it into the activity spot of the specified class. The activity spot will be taken from the
 * UInteractionManagerComponent::GetSelectedInteractableComponent. A UInteractionManagerComponent will be taken from the
 * character that owns the UInventoryManagerComponent this item is in.
 */
UCLASS()
class ESCAPECHRONICLES_API UDollUsableInventoryItemFragment : public UUsableItemInventoryItemFragment
{
	GENERATED_BODY()

protected:
	virtual bool OnTriedToUseItem(UInventoryManagerComponent* InventoryManager, UInventoryItemInstance* ItemInstance,
		const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const override;

private:
	/**
	 * Activity spot classes that this doll can be used with. If no classes are specified, the doll can be used with any
	 * activity spot.
	 */
	UPROPERTY(EditAnywhere)
	TSet<TSoftClassPtr<AActivitySpot>> AllowedActivitySpotClasses;

	// A class of the doll that will be spawned when the doll is used in the activity spot
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<ADollOccupyingActivitySpot> DollActorClass;

	// If true, the DollActorClass will be spawned at the custom transform specified in DollActorSpawnTransform
	UPROPERTY(EditAnywhere)
	bool bUseCustomSpawnTransform = false;

	// A relative transform that will be used to spawn the DollActorClass when the doll is used in the activity spot
	UPROPERTY(EditAnywhere, meta=(EditCondition="bUseCustomSpawnTransform"))
	FTransform DollActorSpawnTransform;
};