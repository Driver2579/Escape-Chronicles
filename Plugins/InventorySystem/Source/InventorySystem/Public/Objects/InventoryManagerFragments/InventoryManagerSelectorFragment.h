// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "InventoryManagerFragment.h"
#include "InventorySystemGameplayTags.h"
#include "InventoryManagerSelectorFragment.generated.h"

class AInventoryPickupItem;

// Adds selector for a single typed array
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerSelectorFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	virtual void OnManagerInitialized(UInventoryManagerComponent* Inventory) override;
	
	FGameplayTag GetSelectableSlotsType() const { return SelectableSlotsType; }
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Server, Reliable)
	void Server_OffsetCurrentSlotIndex(const int32 Offset);

protected:
	void LogCurrentSlotIndex() const;
	
private:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag SelectableSlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType;

	UPROPERTY(ReplicatedUsing="OnRep_SelectedSlotIndex")
	int32 CurrentSlotIndex;

	UFUNCTION()
	void OnRep_SelectedSlotIndex();
	
	UPROPERTY(EditDefaultsOnly)
	bool bLogCurrentSlotIndex = false;
};
