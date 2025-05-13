// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "EndWhenInputReleasedGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPECHRONICLES_API UEndWhenInputReleasedGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;	
};
