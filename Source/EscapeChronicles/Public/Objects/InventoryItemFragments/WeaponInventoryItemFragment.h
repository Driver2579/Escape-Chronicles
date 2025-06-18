// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Objects/InventoryItemFragments/HoldingViewInventoryItemFragment.h"
#include "WeaponInventoryItemFragment.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UWeaponInventoryItemFragment : public UHoldingViewInventoryItemFragment
{
	GENERATED_BODY()

public:
	virtual void EffectHit() {}
};
