// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/PunchGameplayAbility.h"
#include "Objects/InventoryItemFragments/HoldingViewInventoryItemFragment.h"
#include "WeaponInventoryItemFragment.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UWeaponInventoryItemFragment : public UHoldingViewInventoryItemFragment
{
	GENERATED_BODY()

public:
	const TSoftObjectPtr<UAnimMontage>& GetAnimMontage() const { return AnimMontage; }
	const FName& GetDamageCollisionTag() const { return DamageCollisionTag; }

	virtual void EffectHit(UInventoryItemInstance* ItemInstance) {}

private:
	// Playable animation for the realization of this punch
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> AnimMontage;

	// A component with this tag will be written to DamageCollision (its overlaps generate punch)
	UPROPERTY(EditDefaultsOnly)
	FName DamageCollisionTag;
};
