// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "ToggleCrouchGameplayAbility.generated.h"

/**
 * When activated, calls the Crouch function on the owning character if he isn't crouching or UnCrouch if he is
 * crouching.
 */
UCLASS()
class ESCAPECHRONICLES_API UToggleCrouchGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

public:
	UToggleCrouchGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};