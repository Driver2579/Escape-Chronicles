// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.generated.h"

class UInventoryItemFragment;

/**
 * It is intended to describe items by creating heirs and defining fragments for them
 */
UCLASS(Blueprintable, Const, Abstract)
class INVENTORYSYSTEM_API UInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	const FText& GetDisplayName()
	{
		return DisplayName;
	}
	
	const TArray<TObjectPtr<UInventoryItemFragment>>& GetFragments() const
	{
		return Fragments;
	}

	template<typename T>
	const T* GetFragmentByClass() const
	{
		for (UInventoryItemFragment* Fragment : Fragments)
		{
			T* CastedFragment = Cast<T>(Fragment);
			if (IsValid(CastedFragment))
			{
				return CastedFragment;
			}
		}
		
		return nullptr;
	}
	
protected:
	UPROPERTY(EditDefaultsOnly)
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UInventoryItemFragment>> Fragments;
};
