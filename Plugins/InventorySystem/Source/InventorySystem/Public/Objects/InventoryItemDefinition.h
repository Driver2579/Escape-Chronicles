// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.generated.h"

class UInventoryItemFragment;

// Describes inventory item instances by creating UInventoryItemFragment for them
UCLASS(Blueprintable, Const, Abstract)
class INVENTORYSYSTEM_API UInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	const FText& GetName() { return DisplayName; }

	const TArray<UInventoryItemFragment*>& GetFragments() const { return Fragments; }

private:
	UPROPERTY(EditDefaultsOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UInventoryItemFragment>> Fragments;
};
