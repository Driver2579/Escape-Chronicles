// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "InventorySystemGameplayTags.h"
#include "Common/Structs/FastArraySerializers/InventorySlotsTypedArrayContainer.h"
#include "Interfaces/StoringItemInstances.h"

#include "InventoryManagerComponent.generated.h"

class UInventoryManagerFragment;
DECLARE_MULTICAST_DELEGATE(FOnInventoryContentChanged);

// Adds a custom inventory to an actor
UCLASS(Blueprintable, Const)
class INVENTORYSYSTEM_API UInventoryManagerComponent : public UActorComponent, public IStoringItemInstances
{
	GENERATED_BODY()
	
public:
	UInventoryManagerComponent();
	
	const FInventorySlotsTypedArrayContainer& GetTypedInventorySlotsLists() { return TypedInventorySlotsLists; }
	
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

	bool AddItem(const UInventoryItemDefinition* ItemInstance, int32 SlotIndex = -1,
		FGameplayTag SlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType);
	
	/**
	 * Delete an item from inventory
	 * @param SlotIndex Index of the slot
	 * @param SlotsType Slot type tag
	 */
	bool DeleteItem(const int32 SlotIndex,
		const FGameplayTag SlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType);

	/**
	 * Method for obtaining data on item location in inventory
	 * @return true if the search was successful
	 */
	bool GetItemInstanceContainerAndIndex(FGameplayTag& OutSlotsType, int32& OutSlotIndex,
	                                      UInventoryItemInstance* ItemInstance) const;
	
	virtual void BreakItemInstance(UInventoryItemInstance* ItemInstance) override;
	
	void AddInventoryContentChangedHandler(const FOnInventoryContentChanged::FDelegate& Callback);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Do an action on each item in the inventory
	void ForEachInventoryItemInstance(const TFunctionRef<void(UInventoryItemInstance*)>& Action) const;
	
protected:
	virtual void BeginPlay() override;

	virtual void ReadyForReplication() override;

	void LogInventoryContent() const;
	
private:
	// Called when the contents of inventory slot change
	FOnInventoryContentChanged OnInventoryContentChanged;
	
	/**
	* Settings for the number of slots in different types of inventory slots
	* (Doesn't work dynamically, value must be set before BeginPlay)
	* @tparam FGameplayTag Inventory slots type;
	* @tparam int32 Number of slots;
	*/
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, int32> SlotsNumberByTypes;

	UPROPERTY(EditDefaultsOnly, Instanced, Replicated)
	TArray<TObjectPtr<UInventoryManagerFragment>> Fragments;
	
	UPROPERTY(ReplicatedUsing=OnRep_TypedInventorySlotsLists)
	FInventorySlotsTypedArrayContainer TypedInventorySlotsLists;

	UFUNCTION()
	void OnRep_TypedInventorySlotsLists();

	UPROPERTY(EditDefaultsOnly)
	bool bLogInventoryContentWhenChanges = false;
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