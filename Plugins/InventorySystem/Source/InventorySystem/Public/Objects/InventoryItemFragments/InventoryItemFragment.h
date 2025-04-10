// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/FragmentationFragment.h"
#include "InventoryItemFragment.generated.h"

class UInventoryItemInstance;

/**
 * Base class for creating fragments of an item that can describe various logic for it
 */
UCLASS(Abstract)
class INVENTORYSYSTEM_API UInventoryItemFragment : public UFragmentationFragment
{
	GENERATED_BODY()

};
