// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryItemFragment.h"
#include "PickupInventoryItemFragment.generated.h"

/**
 * Adds a representation of an item to the world
 */
UCLASS()
class INVENTORYSYSTEM_API UPickupInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	UStaticMesh* GetStaticMesh() const
	{
		return StaticMesh;
	}

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> StaticMesh;
};
