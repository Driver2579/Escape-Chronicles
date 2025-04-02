// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryManagerComponent.generated.h"

/* A single slot in an inventory */
USTRUCT()
struct FInventorySlot : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance = nullptr;

	/*UPROPERTY()
	int32 Test = 0;*/
};

/* List of inventory slots */
USTRUCT()
struct FInventorySlotsArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FInventorySlot> Slots;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms);
};

template<>
struct TStructOpsTypeTraits<FInventorySlotsArray> : public TStructOpsTypeTraitsBase2<FInventorySlotsArray>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};

/* A typed list of slots in an inventory */
USTRUCT()
struct FInventorySlotsTypedArray : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FGameplayTag Type;	

	UPROPERTY()
	FInventorySlotsArray List;
};

/* Contain inventory slots by their types */
USTRUCT()
struct FInventorySlotsTypedArrayContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FInventorySlotsTypedArray> TypedLists;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms);
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

	// TODO: Implement these methods
	//void AddItem(UInventoryItemInstance* Item, FGameplayTag Type, int32 SlotIndex);
	//void DeleteItem(FGameplayTag Type, int32 SlotIdx);

	// TODO: Delete Tick
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
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
	
	
	UPROPERTY(Replicated)
	FInventorySlotsTypedArrayContainer TypedInventorySlotsLists;
	
	FOnInventoryInitializedDelegate OnInventoryInitialized;
	void InitializeInventory();

	bool bInventoryInitialized = false;
};
