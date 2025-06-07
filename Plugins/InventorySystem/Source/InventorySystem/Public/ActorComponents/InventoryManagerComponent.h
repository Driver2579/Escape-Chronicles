// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "InventorySystemGameplayTags.h"
#include "Common/Structs/FastArraySerializers/InventorySlotsTypedArrayContainer.h"
#include "InventoryManagerComponent.generated.h"

class UInventoryManagerFragment;

/**
 * Core inventory management component that handles:
 * - Multi-slot inventory system with typed containers
 * - Item instance storage and operations (add/remove/find)
 * - Replicated inventory state with change notifications
 * - Extensible through fragment system
 * - Notifies about changes
 */
UCLASS(Blueprintable, Const)
class INVENTORYSYSTEM_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UInventoryManagerComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const FInventorySlotsTypedArrayContainer& GetInventoryContent() { return InventoryContent; }

	// Returns the first fragment of type T, or nullptr if none exists
	template<typename T>
	T* GetFragmentByClass() const;

	/**
	 * Gets item instance from specified inventory slot.
	 * @param SlotIndex Index of the slot to query.
	 * @param SlotTypeTag Type of slots to search (defaults to Main).
	 * @return Pointer to item instance or nullptr if slot is empty/invalid
	 */
	UInventoryItemInstance* GetItemInstance(const int32 SlotIndex,
		const FGameplayTag& SlotTypeTag = InventorySystemGameplayTags::Inventory_Slot_Type_Main) const;

	/**
	 * Adds a DUPLICATE of the given item to the inventory.
	 * @param ItemInstance The item being copied.
	 * @param SlotIndex Index of the slot (if INDEX_NONE, searches for an empty slot).
	 * @param SlotTypeTag Type of the slot.
	 */
	bool AddItem(const UInventoryItemInstance* ItemInstance, int32 SlotIndex = INDEX_NONE,
		const FGameplayTag& SlotTypeTag = InventorySystemGameplayTags::Inventory_Slot_Type_Main);

	/**
	 * Deletes an item from the inventory.
	 * @param SlotIndex Index of the slot.
	 * @param SlotTypeTag Type of the slot.
	 */
	bool DeleteItem(const int32 SlotIndex,
		const FGameplayTag& SlotTypeTag = InventorySystemGameplayTags::Inventory_Slot_Type_Main);

	DECLARE_MULTICAST_DELEGATE(FOnContentChangedDelegate);

	// Called when the contents of inventory slot changed
	FOnContentChangedDelegate OnContentChanged;

protected:
	virtual void BeginPlay() override;

	virtual void ReadyForReplication() override;

	// Executes Action for each valid item instance in inventory
	void ForEachInventoryItemInstance(const TFunctionRef<void(UInventoryItemInstance*)>& Action) const;

#if WITH_EDITORONLY_DATA && !NO_LOGGING
	// Logs an information about the content of the inventory
	void LogInventoryContent() const;
#endif

private:
	/**
	* Settings for the number of slots in different types of inventory slots (doesn't work dynamically, value must be
	* set before BeginPlay).
	* @param FGameplayTag Type of the slots.
	* @param int32 Number of slots.
	*/
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, int32> SlotsNumberByTypes;

	// Fragments that extend the functionality of the inventory instance
	UPROPERTY(EditDefaultsOnly, Instanced, Replicated)
	TArray<TObjectPtr<UInventoryManagerFragment>> Fragments;

	// Inventory storage with typed slots containers
	UPROPERTY(ReplicatedUsing="OnRep_InventoryContent")
	FInventorySlotsTypedArrayContainer InventoryContent;

	UFUNCTION()
	void OnRep_InventoryContent(FInventorySlotsTypedArrayContainer& Test) const;

#if WITH_EDITORONLY_DATA && !NO_LOGGING
	// If true, then when OnInventoryContentChanged is called, the content of the inventory will be logged
	UPROPERTY(EditDefaultsOnly)
	bool bLogInventoryContent = false;
#endif
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
