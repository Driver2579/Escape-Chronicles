// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "DropItemGameplayAbility.generated.h"

// Throws an item out of inventory (using UInventoryManagerDropItemsFragment and UInventoryManagerSelectorFragment)
UCLASS()
class ESCAPECHRONICLES_API UDropItemGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDropItemGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
