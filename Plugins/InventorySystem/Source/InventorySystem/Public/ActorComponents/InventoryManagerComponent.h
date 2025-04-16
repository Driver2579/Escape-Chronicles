// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameplayTagContainer.h"
#include "InventorySystemGameplayTags.h"
#include "Common/Structs/FastArraySerializers/InventorySlotsTypedArrayContainer.h"

#include "InventoryManagerComponent.generated.h"

class UInventoryManagerFragment;
DECLARE_MULTICAST_DELEGATE(FOnInventoryContentChanged);

// Adds a custom inventory to an actor
UCLASS(Blueprintable, Const)
class INVENTORYSYSTEM_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	const FInventorySlotsTypedArrayContainer& GetTypedInventorySlotsLists()
	{
		return TypedInventorySlotsLists;
	}

	template<typename T>
	T* GetFragmentByClass() const;
	
	void AddOnInventoryContentChanged(const FOnInventoryContentChanged::FDelegate& Callback);
	
	UInventoryManagerComponent();

	/**
	 * Add item DUPLICATE to inventory
	 * @param Item The item being copied;
	 * @param SlotIndex Index of the slot (if -1 searches for an empty slot)
	 * @param SlotsType Slot type tag
	 */
	bool AddItem(const UInventoryItemInstance* Item, size_t SlotIndex = -1,
		FGameplayTag SlotsType = InventorySystemGameplayTags::InventoryTag_MainSlotType);

protected:
	virtual void BeginPlay() override;

	virtual void ReadyForReplication() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Do an action on each item in the inventory
	void ForEachInventoryItemInstance(const TFunctionRef<void(UInventoryItemInstance*)>& Action) const;

	void LogInventoryContent() const;
	
private:
	// Called when the contents of inventory slot change
	FOnInventoryContentChanged OnInventoryContentChanged;

	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<UInventoryManagerFragment*> Fragments;
	
	/**
	* Settings for the number of slots in different types of inventory slots
	* (Doesn't work dynamically, value must be set before BeginPlay)
	* @tparam FGameplayTag Inventory slots type;
	* @tparam int32 Number of slots;
	*/
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, int32> SlotTypesAndNumber;
	
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