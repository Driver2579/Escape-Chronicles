// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryItemFragments/InventoryItemFragment.h"
#include "IconInventoryItemFragment.generated.h"

// Adds a representation of an item to the UI
UCLASS()
class INVENTORYSYSTEM_API UIconInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	UTexture2D* GetIcon() const { return Icon; }

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Icon;
};
