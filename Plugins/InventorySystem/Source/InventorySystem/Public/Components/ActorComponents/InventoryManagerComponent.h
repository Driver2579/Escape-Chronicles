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
 * List of inventory slots
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

	void Init(const int32 InSlotsNumber)
	{
		Slots.Init(FInventorySlot(), InSlotsNumber);
		MarkArrayDirty();
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventorySlot, FInventorySlotsArray>( Slots, DeltaParms, *this );
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
 * A typed list of slots in an inventory
 */
USTRUCT()
struct FInventorySlotsTypedArray : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FGameplayTag GetType() const
	{
		return Type;
	}
	
	const FInventorySlotsArray& GetList() const
	{
		return List;
	}

	void SetInstanceIntoSlot(UInventoryItemInstance* Instance, const int32 Index)
	{
		List.SetInstanceIntoSlot(Instance, Index);
	}
	
	void Init(const FGameplayTag InType, const int32 InSlotsNumber)
	{
		Type = InType;
		List.Init(InSlotsNumber);
	}

private:
	UPROPERTY()
	FGameplayTag Type;
	
	UPROPERTY()
	FInventorySlotsArray List;
};

/**
 * Contain inventory slots by their types
 */
USTRUCT()
struct FInventorySlotsTypedArrayContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	const TArray<FInventorySlotsTypedArray>& GetTypedLists() const
	{
		return Lists;
	}

	void SetInstanceIntoSlot(UInventoryItemInstance* Instance, const int32 Index, FGameplayTag Type)
	{
		FInventorySlotsTypedArray* SlotsTypedArray = Lists.FindByPredicate(
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

	void Init(const TMap<FGameplayTag, int32>& InitializationData)
	{
		for (const auto& Pair : InitializationData)
		{
			auto SlotsTypedArray = FInventorySlotsTypedArray();
			SlotsTypedArray.Init(Pair.Key, Pair.Value);
			Lists.Add(SlotsTypedArray);
		}

		MarkArrayDirty();
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventorySlotsTypedArray, FInventorySlotsTypedArrayContainer>( Lists, DeltaParms, *this );
	}

private:
	UPROPERTY()
	TArray<FInventorySlotsTypedArray> Lists;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotsTypedArrayContainer> : public TStructOpsTypeTraitsBase2<FInventorySlotsTypedArrayContainer>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};










/**
 * Adds a custom inventory to an actor
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryInitializedDelegate, UInventoryManagerComponent*);
	
public:	
	UInventoryManagerComponent();

	void CallOrRegisterOnInventoryInitialized(const FOnInventoryInitializedDelegate::FDelegate& Callback);
	
	// TODO: Implement these methods
	//void AddItem(UInventoryItemInstance* Item, int32 Index);
	void AddItem(UInventoryItemInstance* Item, int32 Index, FGameplayTag Type = InventorySystemGameplayTags::InventoryTag_MainSlotType);

protected:
	virtual void BeginPlay() override;

	virtual void ReadyForReplication() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/**
	* Settings for the number of slots in different types of inventory slots.
	* @tparam FGameplayTag Inventory type.
	* @tparam int32 Number of slots .
	*/
	UPROPERTY(EditAnywhere, Category="Inventory Settings")
	TMap<FGameplayTag, int32> SlotTypesAndQuantities;
	
	UPROPERTY(ReplicatedUsing=OnRep_TypedInventorySlotsLists)
	FInventorySlotsTypedArrayContainer TypedInventorySlotsLists;

	UFUNCTION()
	void OnRep_TypedInventorySlotsLists();
	
	FOnInventoryInitializedDelegate OnInventoryInitialized;
	void InitializeInventory();
	bool bInventoryInitialized = false;
};
