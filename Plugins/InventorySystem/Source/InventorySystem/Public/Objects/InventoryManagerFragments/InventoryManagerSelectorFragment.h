// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Objects/InventoryManagerFragment.h"
#include "InventorySystemGameplayTags.h"
#include "InventoryManagerSelectorFragment.generated.h"

class AInventoryPickupItem;

// Stores the slot selector in the inventory array and allows to offset it
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerSelectorFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	FGameplayTag GetSelectableSlotsType() const { return SelectableSlotsTypeTag; }
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_OffsetCurrentSlotIndex(const int32 Offset);

protected:
	void LogCurrentSlotIndex() const;

private:
	// The type of slot array for which want to add a selector
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag SelectableSlotsTypeTag = InventorySystemGameplayTags::InventoryTag_MainSlotType;

	// Index of the currently selected slot
	UPROPERTY(ReplicatedUsing="OnRep_SelectedSlotIndex")
	int32 CurrentSlotIndex;

	UFUNCTION()
	void OnRep_SelectedSlotIndex();

	// Whether log CurrentSlotIndex when it changes 
	UPROPERTY(EditDefaultsOnly)
	bool bLogCurrentSlotIndex = false;
};
