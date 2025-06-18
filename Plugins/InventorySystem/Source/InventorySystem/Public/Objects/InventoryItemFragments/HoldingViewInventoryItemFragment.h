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
	void StartHolding(UInventoryItemInstance* ItemInstance);
	void StopHolding(UInventoryItemInstance* ItemInstance);

private:
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<AActor> ItemInstanceViewClass;

	UPROPERTY(EditDefaultsOnly)
	FName ComponentToAttachTag;

	UPROPERTY(EditDefaultsOnly)
	FName SocketToAttach;

	UPROPERTY(EditDefaultsOnly)
	FTransform TransformToAttach;

	TMap<TObjectPtr<UInventoryItemInstance>, FHoldingViewDataItem> HoldingViewData;

	void ItemInstanceViewClassLoaded(UInventoryItemInstance* ItemInstance);
};
