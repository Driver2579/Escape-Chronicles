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
	Group,

	// Access to all players
	Public,

	NumberOfModes UMETA(Hidden),
	None UMETA(Hidden)
};

USTRUCT()
struct FTransferItemsData
{
	GENERATED_BODY()

	TWeakObjectPtr<const UInventoryManagerComponent> FromInventoryManager;
	FGameplayTag FromSlotTypeTag;
	int32 FromSlotIndex;

	TWeakObjectPtr<const UInventoryManagerComponent> ToInventoryManager;
	FGameplayTag ToSlotTypeTag;
	int32 ToSlotIndex;
};

//
UCLASS()
class INVENTORYSYSTEM_API UInventoryManagerTransferItemsFragment : public UInventoryManagerFragment
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ol UInventoryManagerComponent::AddItem(const UInventoryItemInstance* ItemInstance, int32 SlotIndex,
	//const FGameplayTag& SlotTypeTag)
	UFUNCTION(Server, Reliable)
	void Server_TransferItems(const FTransferItemsData& TransferItemsData);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAccessChangedDelegate, EInventoryAccess CurrentInventoryAccess);

	FOnAccessChangedDelegate OnAccessChanged;

private:
	// 
	UPROPERTY(ReplicatedUsing="OnRep_InventoryAccess")
	EInventoryAccess InventoryAccess;

	UFUNCTION()
	void OnRep_InventoryAccess() const
	{
		OnAccessChanged.Broadcast(InventoryAccess);
	}
};
