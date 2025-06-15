// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryItemFragment.h"
#include "ClassificationInventoryItemFragment.generated.h"

enum class EItemClassification : uint8;

// A fragment that simply adds a classification to the item (EItemClassification)
UCLASS()
class ESCAPECHRONICLES_API UClassificationInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	EItemClassification GetItemClassification() const { return ItemClassification; }

private:
	UPROPERTY(EditAnywhere)
	EItemClassification ItemClassification;
};