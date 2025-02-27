// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "EscapeChroniclesGameplayAbility.generated.h"

// Base class for all gameplay abilities in this project
UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	/**
	 * I don't like this, but it's the only way to allow our custom AbilitySystemComponent to manually end this ability.
	 * This is what original UGameplayAbility and UAbilitySystemComponent do, and that's disgusting.
	 */
	friend class UEscapeChroniclesAbilitySystemComponent;

public:
	UEscapeChroniclesGameplayAbility();
};