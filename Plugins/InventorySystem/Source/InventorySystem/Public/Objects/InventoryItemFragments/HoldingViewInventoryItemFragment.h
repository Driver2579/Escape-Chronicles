// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Objects/InventoryItemFragment.h"
#include "HoldingViewInventoryItemFragment.generated.h"

struct FStreamableHandle;

USTRUCT()
struct FHoldingViewDataItem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> SpawnedActor;

	TSharedPtr<FStreamableHandle> StreamableHandle;
};

UCLASS()
class INVENTORYSYSTEM_API UHoldingViewInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	bool IsHoldingItem(const UInventoryItemInstance* ItemInstance) const
	{
		return HoldingViewData.Contains(ItemInstance);
	}
	
	AActor* GetActor(const UInventoryItemInstance* ItemInstance) const
	{
		return HoldingViewData[ItemInstance].SpawnedActor;
	}

	const TMap<TObjectPtr<UInventoryItemInstance>, FHoldingViewDataItem>& GetHoldingViewData() const
	{
		return HoldingViewData;
	}

	virtual void StartHolding(UInventoryItemInstance* ItemInstance) const;
	virtual void StopHolding(UInventoryItemInstance* ItemInstance) const;

private:
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<AActor> ItemInstanceViewClass;

	UPROPERTY(EditDefaultsOnly)
	FName ComponentToAttachTag;

	UPROPERTY(EditDefaultsOnly)
	FName SocketToAttach;

	UPROPERTY(EditDefaultsOnly)
	FTransform TransformToAttach;

	mutable TMap<TObjectPtr<UInventoryItemInstance>, FHoldingViewDataItem> HoldingViewData;

	void ItemInstanceViewClassLoaded(UInventoryItemInstance* ItemInstance) const;
};
