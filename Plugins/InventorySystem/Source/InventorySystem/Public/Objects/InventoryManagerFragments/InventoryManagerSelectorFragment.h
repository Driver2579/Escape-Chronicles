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
	
	// 
	void SelectNextItem();

	// 
	void SelectPrevItem();

	//
	void DropSelectedItem(const FVector ThrowingDirection);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	UFUNCTION(Server, Reliable)
	void Server_OffsetCurrentSlotIndex(const int32 Offset);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DropItem(const int32 Offset, const FVector ThrowingDirection);
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag SelectableSlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AInventoryPickupItem> DropItemActorClass;
	
	UPROPERTY(Replicated)
	int32 CurrentSlotIndex;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxTrowingPower;
};
