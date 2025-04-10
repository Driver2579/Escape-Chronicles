// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/FragmentationDefinition.h"
#include "InventoryItemDefinition.generated.h"

class UInventoryItemFragment;

/**
 * It is intended to describe items by creating heirs and defining fragments for them
 */
UCLASS(Blueprintable, Const, Abstract)
class INVENTORYSYSTEM_API UInventoryItemDefinition : public UFragmentationDefinition
{
	GENERATED_BODY()

public:
	virtual const TArray<TObjectPtr<UFragmentationFragment>>* GetFragments() const override
	{
        return reinterpret_cast<const TArray<TObjectPtr<UFragmentationFragment>>*>(&Fragments);
	}
	
protected:
	UPROPERTY(EditDefaultsOnly)
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UInventoryItemFragment>> Fragments;
};
