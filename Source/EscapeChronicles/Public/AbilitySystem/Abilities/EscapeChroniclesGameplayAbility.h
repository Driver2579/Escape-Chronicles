// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InputActionValue.h"
#include "EscapeChroniclesGameplayAbility.generated.h"

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

	// Should be used only by the ability system component to set the InputActionValue that triggered this ability
	void SetInputActionValue(const FInputActionValue& InInputActionValue) { InputActionValue = InInputActionValue; }

	const FInputActionValue& GetInputActionValue() const { return InputActionValue; }

private:
	/**
	 * The InputActionValue of InputAction that triggered this ability, if any.\n
	 * This exists only locally, so if you want to use it, you need to set NetSecurityPolicy to LocalOnly for this
	 * ability.
	 */
	FInputActionValue InputActionValue;
};