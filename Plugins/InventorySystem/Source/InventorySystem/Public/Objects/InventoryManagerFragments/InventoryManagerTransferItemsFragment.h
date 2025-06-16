// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Objects/InventoryManagerFragment.h"
#include "InventoryManagerTransferItemsFragment.generated.h"

class AInventoryPickupItem;

UENUM(BlueprintType)
enum class EInventoryAccess : uint8
{
	// Access only to yourself
	Private,

	// Access via access list (TODO: do handling in fragment)
	//Group,

	// Access to all players
	Public,

	NumberOfModes UMETA(Hidden),
	None UMETA(Hidden)
};

USTRUCT()
struct FTransferItemsData
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UInventoryManagerComponent> FromInventoryManager;
	
	UPROPERTY()
	FGameplayTag FromSlotTypeTag;

	UPROPERTY()
	int32 FromSlotIndex;


	UPROPERTY()
	TWeakObjectPtr<UInventoryManagerComponent> ToInventoryManager;

	UPROPERTY()
	FGameplayTag ToSlotTypeTag;

	UPROPERTY()
	int32 ToSlotIndex;
};

//
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerTransferItemsFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	bool HasInventoryAccess(const UInventoryManagerTransferItemsFragment* InInventoryManagerTransferItemsFragment) const;

	EInventoryAccess GetAccess() const { return Access; };

	UInventoryManagerComponent* GetLootInventory() const { return LootInventory.Get(); }

	void SetInventoryAccess(const EInventoryAccess InInventoryAccess) { Access = InInventoryAccess; }

	void TrySetLootInventory(UInventoryManagerComponent* InInventory);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TransferItems(const FTransferItemsData& TransferItemsData);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAccessChangedDelegate, EInventoryAccess CurrentInventoryAccess);

	FOnAccessChangedDelegate OnAccessChanged;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLootInventoryChangedDelegate, UInventoryManagerComponent* InInventory);

	FOnLootInventoryChangedDelegate OnLootInventoryChanged;

private:
	void SetLootInventory(UInventoryManagerComponent* InInventory)
	{
		LootInventory = InInventory;

		OnLootInventoryChanged.Broadcast(LootInventory.Get());
	}

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing="OnRep_Access")
	EInventoryAccess Access;

	UPROPERTY(ReplicatedUsing="OnRep_LootInventory")
	TWeakObjectPtr<UInventoryManagerComponent> LootInventory;

	UFUNCTION()
	void OnRep_Access() const
	{
		OnAccessChanged.Broadcast(Access);
	}

	UFUNCTION()
	void OnRep_LootInventory() const
	{
		OnLootInventoryChanged.Broadcast(LootInventory.Get());
	}
};
