// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "InventorySystemGameplayTags.h"
#include "Common/Structs/FastArraySerializers/InventorySlotsTypedArrayContainer.h"

#include "InventoryManagerComponent.generated.h"

class UInventoryManagerFragment;

// Called when the contents of inventory slot change
DECLARE_MULTICAST_DELEGATE(FOnInventoryContentChangedDelegate);

// Adds a custom inventory to an actor
UCLASS(Blueprintable, Const)
class INVENTORYSYSTEM_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UInventoryManagerComponent();
	
	const FInventorySlotsTypedArrayContainer& GetInventoryContent() { return InventoryContent; }

	// Returns the first fragment of type T, or nullptr if none exists
	template<typename T>
	T* GetFragmentByClass() const;

	UInventoryItemInstance* GetItemInstance(const int32 SlotIndex,
		const FGameplayTag SlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType) const;
	
	/**
	 * Add item DUPLICATE to inventory
	 * @param ItemInstance The item being copied
	 * @param SlotIndex Index of the slot (if -1 searches for an empty slot)
	 * @param SlotsType Slot type tag
	 */
	bool AddItem(const UInventoryItemInstance* ItemInstance, int32 SlotIndex = -1,
		FGameplayTag SlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType);

	/**
	 * Delete an item from inventory
	 * @param SlotIndex Index of the slot
	 * @param SlotsType Slot type tag
	 */
	bool DeleteItem(const int32 SlotIndex,
		const FGameplayTag SlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType);

	void AddInventoryContentChangedHandler(const FOnInventoryContentChangedDelegate::FDelegate& Callback);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;

	virtual void ReadyForReplication() override;

	// Do an action on each item in the inventory
	void ForEachInventoryItemInstance(const TFunctionRef<void(UInventoryItemInstance*)>& Action) const;

	// Log information about the contents of the inventory
	void LogInventoryContent() const;
	
private:
	FOnInventoryContentChangedDelegate OnInventoryContentChanged;
	
	/**
	* Settings for the number of slots in different types of inventory slots
	* (Doesn't work dynamically, value must be set before BeginPlay)
	* @tparam FGameplayTag Inventory slots type;
	* @tparam int32 Number of slots;
	*/
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, int32> SlotsNumberByTypes;

	// Fragments that extend the functionality of the inventory instance
	UPROPERTY(EditDefaultsOnly, Instanced, Replicated)
	TArray<TObjectPtr<UInventoryManagerFragment>> Fragments;

	// Arranged by type inventory slots capable of storing item instances
	UPROPERTY(ReplicatedUsing="OnRep_InventoryContent")
	FInventorySlotsTypedArrayContainer InventoryContent;

	UFUNCTION()
	void OnRep_InventoryContent();

	// If true will log the contents of the inventory when calling OnInventoryContentChanged
	UPROPERTY(EditDefaultsOnly)
	bool bLogInventoryContent = false;
};

template<typename T>
T* UInventoryManagerComponent::GetFragmentByClass() const
{
	static_assert(TIsDerivedFrom<T, UInventoryManagerFragment>::Value,
		"T must be inherited from UInventoryManagerFragment!");
		
	for (UInventoryManagerFragment* Fragment : Fragments)
	{
		T* CastedFragment = Cast<T>(Fragment);
            
		if (IsValid(CastedFragment))
		{
			return CastedFragment;
		}
	}

	return nullptr;
}
