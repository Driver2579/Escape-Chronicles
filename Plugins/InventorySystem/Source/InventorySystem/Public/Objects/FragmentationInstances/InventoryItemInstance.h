// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/FragmentationInstance.h"
#include "InventoryItemInstance.generated.h"

class UInventoryItemFragment;

/**
 * A specific object whose properties are described by a UInventoryItemDefinition
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryItemInstance : public UFragmentationInstance
{
	GENERATED_BODY()
	
};
