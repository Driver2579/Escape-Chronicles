// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/PlayerControllers/EscapeChroniclesPlayerController.h"

#include "EnhancedInputComponent.h"
#include "Common/DataAssets/InputConfig.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

UAbilitySystemComponent* AEscapeChroniclesPlayerController::GetAbilitySystemComponent() const
{
	const AEscapeChroniclesPlayerState* EscapeChroniclesPlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		PlayerState, ECastCheckedType::NullAllowed);

	return IsValid(EscapeChroniclesPlayerState) ? EscapeChroniclesPlayerState->GetAbilitySystemComponent() : nullptr;
}

UEscapeChroniclesAbilitySystemComponent*
	AEscapeChroniclesPlayerController::GetEscapeChroniclesAbilitySystemComponent() const
{
	const AEscapeChroniclesPlayerState* EscapeChroniclesPlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		PlayerState, ECastCheckedType::NullAllowed);

	return IsValid(EscapeChroniclesPlayerState) ?
		EscapeChroniclesPlayerState->GetEscapeChroniclesAbilitySystemComponent() : nullptr;
}

void AEscapeChroniclesPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	// Bind all input configs
	for (const TObjectPtr<UInputConfig>& InputConfig : InputConfigs)
	{
		BindInputConfig(EnhancedInputComponent, InputConfig);
	}
}

void AEscapeChroniclesPlayerController::BindInputConfig(UEnhancedInputComponent* EnhancedInputComponent,
	const UInputConfig* InputConfig)
{
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent = GetEscapeChroniclesAbilitySystemComponent();

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return;
	}

	const TMap<TObjectPtr<const UInputAction>, FInputConfigActionSettings>& AbilityInputActions =
		InputConfig->GetAbilityInputActions();

	for (auto& InputActionPair : AbilityInputActions)
	{
		TObjectPtr<const UInputAction> InputAction = InputActionPair.Key;

		if (!ensureAlways(InputAction))
		{
			continue;
		}

		const FInputConfigActionSettings& InputConfigActionSettings = InputActionPair.Value;

		// Register the input tag inside the ability system component as required
		AbilitySystemComponent->RegisterInputTag(InputConfigActionSettings.InputTag);

		// Always bind an action to the Triggered event
		EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this,
			&ThisClass::InputTriggered, InputConfigActionSettings.InputTag);

		// Bind an action to the Completed event only if needed
		if (InputConfigActionSettings.bEndAbilityOnComplete)
		{
			EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this,
				&ThisClass::InputCompleted, InputConfigActionSettings.InputTag);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AEscapeChroniclesPlayerController::InputTriggered(const FInputActionValue& InputActionValue,
	const FGameplayTag InputTag)
{
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent = GetEscapeChroniclesAbilitySystemComponent();

	if (ensureAlways(IsValid(AbilitySystemComponent)))
	{
		AbilitySystemComponent->TryActivateAbilitiesByInputTag(InputTag, InputActionValue);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AEscapeChroniclesPlayerController::InputCompleted(const FGameplayTag InputTag)
{
	const UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent = GetEscapeChroniclesAbilitySystemComponent();

	if (ensureAlways(IsValid(AbilitySystemComponent)))
	{
		AbilitySystemComponent->TryEndAbilitiesByInputTag(InputTag);
	}
}