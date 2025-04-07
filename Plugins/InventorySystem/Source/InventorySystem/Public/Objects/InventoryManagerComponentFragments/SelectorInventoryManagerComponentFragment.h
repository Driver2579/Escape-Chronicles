// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Objects/InventoryItemInstance.h"
#include "Objects/InventoryManagerComponentFragment.h"
#include "SelectorInventoryManagerComponentFragment.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API USelectorInventoryManagerComponentFragment : public UInventoryManagerComponentFragment
{
	GENERATED_BODY()

public:
	int32 GetSelectedItemIndex() const
	{
		return SelectedItemIndex;
	}

	UInventoryItemInstance* GetSelectedItemInstance() const
	{
		return SelectedItemInstance.Get();
	}
	
private:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag SlotsArrayType;
	
	int32 SelectedItemIndex;
	TWeakObjectPtr<UInventoryItemInstance> SelectedItemInstance;
};
