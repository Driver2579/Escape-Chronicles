// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventorySystemGameplayTags.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Objects/InventoryItemInstance.h"
#include "InventoryManagerComponent.generated.h"

class UInventoryItemInstance;

/**
 * A single slot in an inventory
 */
USTRUCT()
struct FInventorySlot : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	UInventoryItemInstance* GetInstance() const
	{
		return Instance;
	}
	
	void SetInstance(UInventoryItemInstance* NewInstance)
	{
		Instance = NewInstance;
	}
	
private:
	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance = nullptr;
};

/**
 * Array of inventory slots
 */
USTRUCT()
struct FInventorySlotsArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()
	
	const TArray<FInventorySlot>& GetSlots() const
	{
		return Slots;
	}

	void SetInstanceIntoSlot(UInventoryItemInstance* Instance, const int32 Index)
	{
		if (!ensureAlways(IsValid(Instance)))
		{
			return;
		}
		
		if (!ensureAlwaysMsgf(Index <= Slots.Num() - 1 && Index >= 0, TEXT("Unavailable slot number")))
		{
			return;
		}
		
		if (!ensureAlwaysMsgf(Slots[Index].GetInstance() == nullptr, TEXT("Slot is not empty")))
		{
			return;
		}
		
		Slots[Index].SetInstance(Instance);
		MarkItemDirty(Slots[Index]);
	}

	FInventorySlotsArray(const int32 InSlotsNumber)
	{
		Slots.Init(FInventorySlot(), InSlotsNumber);
		MarkArrayDirty();
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventorySlot, FInventorySlotsArray>( Slots,
			DeltaParms, *this );
	}
	
private:
	UPROPERTY()
	TArray<FInventorySlot> Slots;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotsArray> : public TStructOpsTypeTraitsBase2<FInventorySlotsArray>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};










/**
 * Array of inventory slots with type
 */
USTRUCT()
struct FInventorySlotsTypedArray : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FGameplayTag GetType() const
	{
		return Type;
	}
	
	const FInventorySlotsArray& GetArray() const
	{
		return Array;
	}

	void SetInstanceIntoSlot(UInventoryItemInstance* Instance, const int32 Index)
	{
		Array.SetInstanceIntoSlot(Instance, Index);
	}
	
	FInventorySlotsTypedArray(const FGameplayTag InType, const int32 InSlotsNumber)
		: Type(InType), Array(InSlotsNumber){}

private:
	UPROPERTY()
	FGameplayTag Type;
	
	UPROPERTY()
	FInventorySlotsArray Array;
};

/**
 * Contain arrays of slots by their types
 */
USTRUCT()
struct FInventorySlotsTypedArrayContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	const TArray<FInventorySlotsTypedArray>& GetArrays() const
	{
		return Arrays;
	}

	void SetInstanceIntoSlot(UInventoryItemInstance* Instance, const int32 Index, FGameplayTag Type)
	{
		// Search for an array that is marked with the corresponding tag
		FInventorySlotsTypedArray* SlotsTypedArray = Arrays.FindByPredicate(
			[Type](const FInventorySlotsTypedArray& List)
			{
				return List.GetType() == Type;
			});

		if (!ensureAlwaysMsgf(SlotsTypedArray, TEXT("Array not found by tag")))
		{
			return;
		}

		SlotsTypedArray->SetInstanceIntoSlot(Instance, Index);
		
		MarkItemDirty(*SlotsTypedArray);
	}
	
	/**
	* @tparam FGameplayTag Inventory slots type;
	* @tparam int32 Number of slots;
	 */
	explicit FInventorySlotsTypedArrayContainer(const TMap<FGameplayTag, int32>& InitializationData)
	{
		for (const auto& Pair : InitializationData)
		{
			Arrays.Add(FInventorySlotsTypedArray(Pair.Key, Pair.Value));
		}
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventorySlotsTypedArray,
			FInventorySlotsTypedArrayContainer>( Arrays, DeltaParms, *this );
	}

private:
	UPROPERTY()
	TArray<FInventorySlotsTypedArray> Arrays;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotsTypedArrayContainer>
	: public TStructOpsTypeTraitsBase2<FInventorySlotsTypedArrayContainer>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};








DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryContentChanged, class UInventoryManagerComponent*);

/**
 * Adds a custom inventory to an actor
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UInventoryManagerComponent();

	void AddOnInventoryContentChanged(const FOnInventoryContentChanged::FDelegate& Callback);
	
	void AddItem(UInventoryItemInstance* Item, int32 Index,
		FGameplayTag Type = InventorySystemGameplayTags::InventoryTag_MainSlotType);

protected:
	virtual void BeginPlay() override;

	virtual void ReadyForReplication() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// Do an action on each item in the inventory
	void ForEachInventoryItemInstance(const TFunctionRef<void(UInventoryItemInstance*)>& Action) const;

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
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, int32> SlotTypesAndQuantities;
	
	UPROPERTY(ReplicatedUsing=OnRep_TypedInventorySlotsLists)
	FInventorySlotsTypedArrayContainer TypedInventorySlotsLists;

	UFUNCTION()
	void OnRep_TypedInventorySlotsLists();

	UPROPERTY(EditDefaultsOnly)
	bool bLogInventoryContentWhenChanges = true;
};
