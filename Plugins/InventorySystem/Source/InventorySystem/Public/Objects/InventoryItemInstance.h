// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.generated.h"

class UInventoryItemFragment;

/**
 * A specific object whose properties are described by a UInventoryItemDefinition
 */
UCLASS(EditInlineNew)
class INVENTORYSYSTEM_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	void SetItemDefinition(const TSubclassOf<UInventoryItemDefinition>& NewItemDefinition)
	{
		ItemDefinition = NewItemDefinition;
	}

	template<typename T>
	const T* GetFragmentByClass() const
	{
		if (!IsValid(ItemDefinition))
		{
			return nullptr;
		}
		
		const UInventoryItemDefinition* ItemDefinitionCDO = ItemDefinition->GetDefaultObject<UInventoryItemDefinition>();

		if (!ensureAlways(IsValid(ItemDefinitionCDO)))
		{
			return nullptr;
		}

		return ItemDefinitionCDO->GetFragmentByClass<T>();
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual UInventoryItemInstance* Duplicate(UObject* Outer);
	
private:
	UPROPERTY(EditAnywhere, Replicated)
	TSubclassOf<UInventoryItemDefinition> ItemDefinition;

	// TODO: Create replicated struct FGameplayTag/TUnion for local data
};
