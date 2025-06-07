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
	virtual void OnManagerInitialized() override;

	const FGameplayTag& GetSelectableSlotsTypeTag() const { return SelectableSlotsTypeTag; }
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Offsets CurrentSlotIndex to the passed value within the specified inventory
	UFUNCTION(Server, Reliable)
	void Server_OffsetCurrentSlotIndex(const int32 Offset);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOffsetCurrentSlotIndexDelegate, int32 CurrentSlotIndex);

	// Called when CurrentSlotIndex is offset
	FOnOffsetCurrentSlotIndexDelegate OnOffsetCurrentSlotIndex;

protected:
#if !NO_LOGGING
	void LogCurrentSlotIndex() const;
#endif

private:
	// The type of the slots array which you want to add a selector for
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag SelectableSlotsTypeTag = InventorySystemGameplayTags::Inventory_SlotType_Main;

	// Index of the currently selected slot
	UPROPERTY(Transient, ReplicatedUsing="OnRep_SelectedSlotIndex")
	int32 CurrentSlotIndex;

	UFUNCTION()
	void OnRep_SelectedSlotIndex();

	// Whether to log the CurrentSlotIndex when it changes
	UPROPERTY(EditDefaultsOnly)
	bool bLogCurrentSlotIndex = false;
};
