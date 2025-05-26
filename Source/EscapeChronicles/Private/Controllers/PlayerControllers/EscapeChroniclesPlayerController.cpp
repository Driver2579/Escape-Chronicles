// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/PlayerControllers/EscapeChroniclesPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "EscapeChroniclesGameplayTags.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/DataAssets/InputConfig.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "Subsystems/SaveGameSubsystem.h"

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

AEscapeChroniclesCharacter* AEscapeChroniclesPlayerController::GetEscapeChroniclesCharacter() const
{
	return CastChecked<AEscapeChroniclesCharacter>(GetPawn(), ECastCheckedType::NullAllowed);
}

void AEscapeChroniclesPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind all input configs if PlayerState is already valid (usually always valid on a server at this stage)
	if (PlayerState)
	{
		BindInputConfigs();
	}
	/**
	 * Otherwise, bind them when PlayerState is initialized (usually always invalid on a client at this stage, and we
	 * need to wait until it's replicated).
	 */
	else
	{
		bBindInputConfigsOnPlayerStateInitialized = true;
	}
}

void AEscapeChroniclesPlayerController::InitPlayerState()
{
	// Initialize the PlayerState with a custom class if it's set
	if (IsValid(PlayerStateClassOverride))
	{
		AEscapeChroniclesPlayerState::InitPlayerStateForController(this, PlayerStateClassOverride);
	}
	// Use the default PlayerState initialization otherwise
	else
	{
		Super::InitPlayerState();
	}

	/**
	 * Bind all input configs once the PlayerState is initialized if they failed to bind in SetupInputComponent.
	 * 
	 * P.S.: Usually this function is called before the SetupInputComponent, if we are the server (this function is never
	 * called on clients), but it's better to be safe than sorry.
	 */
	if (bBindInputConfigsOnPlayerStateInitialized)
	{
		BindInputConfigs();
	}
}

void AEscapeChroniclesPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	/**
	 * Bind all input configs once the PlayerState is replicated if they failed to bind in SetupInputComponent.
	 *
	 * P.S.: Usually it's always the case on clients.
	 */
	if (bBindInputConfigsOnPlayerStateInitialized)
	{
		BindInputConfigs();
	}
}

void AEscapeChroniclesPlayerController::BindInputConfigs()
{
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
	AddMappingContexts(InputConfig);

	BindNativeInputActions(EnhancedInputComponent, InputConfig);
	BindAbilityInputActions(EnhancedInputComponent, InputConfig);
}

void AEscapeChroniclesPlayerController::AddMappingContexts(const UInputConfig* InputConfig) const
{
#if DO_CHECK
	check(IsValid(InputConfig));
#endif

	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (!ensureAlways(IsValid(EnhancedInputSubsystem)))
	{
		return;
	}

	const TArray<TObjectPtr<const UInputMappingContext>>& InputMappingContexts = InputConfig->GetInputMappingContexts();

	for (int i = 0; i < InputMappingContexts.Num(); ++i)
	{
		EnhancedInputSubsystem->AddMappingContext(InputMappingContexts[i], i);
	}
}

void AEscapeChroniclesPlayerController::BindNativeInputActions(UEnhancedInputComponent* EnhancedInputComponent,
	const UInputConfig* InputConfig)
{
#if DO_CHECK
	check(IsValid(EnhancedInputComponent));
	check(IsValid(InputConfig));
#endif

	const TMap<TObjectPtr<const UInputAction>, FGameplayTag>& NativeInputActions = InputConfig->GetNativeInputActions();

	const TObjectPtr<const UInputAction>* MoveInputAction = NativeInputActions.FindKey(
		EscapeChroniclesGameplayTags::InputTag_Move);

	const TObjectPtr<const UInputAction>* LookInputAction = NativeInputActions.FindKey(
		EscapeChroniclesGameplayTags::InputTag_Look);

	if (ensureAlways(MoveInputAction))
	{
		EnhancedInputComponent->BindAction(*MoveInputAction, ETriggerEvent::Triggered, this,
			&ThisClass::MoveActionTriggered);

		EnhancedInputComponent->BindAction(*MoveInputAction, ETriggerEvent::Completed, this,
			&ThisClass::MoveActionCompleted);
	}

	if (ensureAlways(LookInputAction))
	{
		EnhancedInputComponent->BindAction(*LookInputAction, ETriggerEvent::Triggered, this,
			&ThisClass::LookActionTriggered);
	}
}

void AEscapeChroniclesPlayerController::BindAbilityInputActions(UEnhancedInputComponent* EnhancedInputComponent,
	const UInputConfig* InputConfig)
{
#if DO_CHECK
	check(IsValid(EnhancedInputComponent));
	check(IsValid(InputConfig));
#endif

	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent = GetEscapeChroniclesAbilitySystemComponent();

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return;
	}

	const TMap<TObjectPtr<const UInputAction>, FGameplayTag>& AbilityInputActions =
		InputConfig->GetAbilityInputActions();

	for (auto& InputActionPair : AbilityInputActions)
	{
		TObjectPtr<const UInputAction> InputAction = InputActionPair.Key;

		if (!ensureAlways(InputAction))
		{
			continue;
		}

		// Register the input tag inside the ability system component as required
		AbilitySystemComponent->RegisterInputTag(InputActionPair.Value);

		// Always bind an action to ETriggerEvent::Started
		EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this,
			&ThisClass::AbilityInputStarted, InputActionPair.Value);

		// Bind an action to the Completed event
		EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this,
			&ThisClass::AbilityInputCompleted, InputActionPair.Value);
	}
}

// ReSharper disable CppMemberFunctionMayBeConst
void AEscapeChroniclesPlayerController::AbilityInputStarted(const FGameplayTag InputTag)
{
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent = GetEscapeChroniclesAbilitySystemComponent();

	if (ensureAlways(IsValid(AbilitySystemComponent)))
	{
		AbilitySystemComponent->PressInputTag(InputTag);
	}
}

void AEscapeChroniclesPlayerController::AbilityInputCompleted(const FGameplayTag InputTag)
{
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent = GetEscapeChroniclesAbilitySystemComponent();

	if (ensureAlways(IsValid(AbilitySystemComponent)))
	{
		AbilitySystemComponent->ReleaseInputTag(InputTag);
	}
}

void AEscapeChroniclesPlayerController::MoveActionTriggered(const FInputActionValue& Value)
{
#if DO_ENSURE
	ensureAlways(Value.GetValueType() == EInputActionValueType::Axis3D);
#endif

	AEscapeChroniclesCharacter* EscapeChroniclesCharacter = GetEscapeChroniclesCharacter();

	if (ensureAlways(IsValid(EscapeChroniclesCharacter)))
	{
		EscapeChroniclesCharacter->Move(Value.Get<FVector>());
	}
}

void AEscapeChroniclesPlayerController::MoveActionCompleted()
{
	AEscapeChroniclesCharacter* EscapeChroniclesCharacter = GetEscapeChroniclesCharacter();

	if (ensureAlways(IsValid(EscapeChroniclesCharacter)))
	{
		EscapeChroniclesCharacter->StopMoving();
	}
}

void AEscapeChroniclesPlayerController::LookActionTriggered(const FInputActionValue& Value)
{
#if DO_ENSURE
	ensureAlways(Value.GetValueType() == EInputActionValueType::Axis2D);
#endif

	AEscapeChroniclesCharacter* EscapeChroniclesCharacter = GetEscapeChroniclesCharacter();

	if (ensureAlways(IsValid(EscapeChroniclesCharacter)))
	{
		EscapeChroniclesCharacter->Look(Value.Get<FVector2D>());
	}
}
// ReSharper restore CppMemberFunctionMayBeConst

void AEscapeChroniclesPlayerController::OnUnPossess()
{
	if (!HasAuthority())
	{
		Super::OnUnPossess();

		return;
	}

	/**
	 * Save the player when the pawn is unpossessed. This is needed to save the game before the player leaves the game.
	 * It would be better to save the player in AEscapeChroniclesGameMode::Logout(), but it's called already after the
	 * Pawn is unpossessed. This is the last place where we can be sure that the Pawn is still valid. The only problem
	 * is that this might be called even due to other reasons than leaving the game, which causes to extra save calls,
	 * but unfortunutelly we don't have any place where we can be sure that the player is leaving the game and where the
	 * Pawn is still valid.
	 */

	USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	if (ensureAlways(IsValid(SaveGameSubsystem)))
	{
		SaveGameSubsystem->SavePlayerOrBot(CastChecked<AEscapeChroniclesPlayerState>(PlayerState));
	}

	Super::OnUnPossess();
}