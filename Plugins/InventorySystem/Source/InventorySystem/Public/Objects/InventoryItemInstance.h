// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemInstance.generated.h"

class UInventoryItemDefinition;

/**
 * A specific object whose properties are described by a UInventoryItemDefinition
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	TSubclassOf<UInventoryItemDefinition> GetItemDefinition() const
	{
		return ItemDefinition;
	}
	void SetItemDefinition(const TSubclassOf<UInventoryItemDefinition>& NewItemDefinition)
	{
		ItemDefinition = NewItemDefinition;
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }

private:
	UPROPERTY(EditAnywhere, Replicated)
	TSubclassOf<UInventoryItemDefinition> ItemDefinition;
};
