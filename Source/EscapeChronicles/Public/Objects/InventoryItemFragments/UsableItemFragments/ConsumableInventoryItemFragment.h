// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UsableItemInventoryItemFragment.h"
#include "ConsumableInventoryItemFragment.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

struct FStreamableHandle;

/**
 * A fragment that applies a gameplay effect to the actor that owns the inventory manager component when the item is
 * used.
 */
UCLASS()
class ESCAPECHRONICLES_API UConsumableInventoryItemFragment : public UUsableItemInventoryItemFragment
{
	GENERATED_BODY()

protected:
	virtual bool OnTriedToUseItem(UInventoryManagerComponent* InventoryManagerComponent,
		UInventoryItemInstance* ItemInstance, const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const override;

private:
	// Gameplay effect that will be applied to the actor using the item
	UPROPERTY(EditAnywhere, Category="Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> GameplayEffectClass;

	// The level of the gameplay effect that will be applied to the actor using the item
	UPROPERTY(EditAnywhere, Category="Gameplay Effects")
	float GameplayEffectLevel = 1;

	// Applies the loaded gameplay effect to the given ASC
	void OnGameplayEffectClassLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
		UAbilitySystemComponent* OwningAbilitySystemComponent) const;
};