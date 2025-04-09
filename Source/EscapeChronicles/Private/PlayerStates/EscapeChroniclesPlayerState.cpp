// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStates/EscapeChroniclesPlayerState.h"

#include "AbilitySystem/AttributeSets/MovementAttributeSet.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/DataAssets/AbilitySystemSet.h"
#include "Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "GameFramework/SpectatorPawn.h"

AEscapeChroniclesPlayerState::AEscapeChroniclesPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UEscapeChroniclesAbilitySystemComponent>(
		TEXT("AbilitySystemComponent"));

	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AEscapeChroniclesPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/**
	 * Give AbilitySystemSets to the AbilitySystemComponent. We do this here because SetupInputComponent is called
	 * before BeginPlay. Except we don't want to apply gameplay effects before the pawn is set to be able to initialize
	 * attributes with data from the pawn.
	 */
	for (UAbilitySystemSet* AbilitySystemSet : AbilitySystemSets)
	{
		if (AbilitySystemSet)
		{
			AbilitySystemSet->GiveAttributesToAbilitySystem(AbilitySystemComponent);
			AbilitySystemSet->GiveAbilitiesToAbilitySystem(AbilitySystemComponent);
		}
	}

	OnPawnSet.AddDynamic(this, &ThisClass::OnPawnChanged);
}

void AEscapeChroniclesPlayerState::OnPawnChanged(APlayerState* ThisPlayerState, APawn* NewPawn, APawn* OldPawn)
{
	// Don't do anything if the new pawn is invalid (for example, if it was removed instead of being set)
	if (!IsValid(NewPawn))
	{
		return;
	}

	// Don't reapply effects if the new pawn is a spectator
	if (NewPawn->IsA<ASpectatorPawn>())
	{
		// Remember the last pawn that wasn't a spectator
		LastNotSpectatorPawn = OldPawn;

		return;
	}

	/**
	 * Don't reapply effects if the new pawn is the same as the last pawn that wasn't a spectator (for example, if we
	 * had the pawn, then switched to a spectator, then switched back to the pawn).
	 */
	if (LastNotSpectatorPawn.IsValid() && NewPawn == LastNotSpectatorPawn)
	{
		// Forget about the last pawn that wasn't a spectator
		LastNotSpectatorPawn.Reset();

		return;
	}

	// Initialize attributes BEFORE the gameplay effects are applied. They may want to use attributes.
	InitializeAttributes();

	/**
	 * Reapply (or apply for the first time) effects from AbilitySystemSets to the AbilitySystemComponent when the new
	 * pawn is set.
	 */
	for (UAbilitySystemSet* AbilitySystemSet : AbilitySystemSets)
	{
		if (AbilitySystemSet)
		{
			AbilitySystemSet->TakeEffectsFromAbilitySystem(AbilitySystemComponent);
			AbilitySystemSet->GiveEffectsToAbilitySystem(AbilitySystemComponent);
		}
	}
}

void AEscapeChroniclesPlayerState::InitializeAttributes()
{
	TryInitializeMovementAttributeSet();
}

void AEscapeChroniclesPlayerState::TryInitializeMovementAttributeSet()
{
	const AEscapeChroniclesCharacter* EscapeChroniclesCharacter = Cast<AEscapeChroniclesCharacter>(GetPawn());

	// Might also be a SpectatorPawn
	if (!IsValid(EscapeChroniclesCharacter))
	{
		return;
	}

	const UMovementAttributeSet* MovementAttributeSet = AbilitySystemComponent->GetSet<UMovementAttributeSet>();

	if (!IsValid(MovementAttributeSet))
	{
		return;
	}

	const UCommonLegacyMovementSettings* CommonLegacyMovementSettings =
		EscapeChroniclesCharacter->GetCharacterMoverComponent()->FindSharedSettings<UCommonLegacyMovementSettings>();

	if (!ensureAlways(IsValid(CommonLegacyMovementSettings)))
	{
		return;
	}

	// Set the MaxGroundSpeed attribute to the default max speed of the CharacterMoverComponent
	AbilitySystemComponent->ApplyModToAttribute(MovementAttributeSet->GetMaxGroundSpeedAttribute(),
		EGameplayModOp::Override, CommonLegacyMovementSettings->MaxSpeed);

	MovementAttributeSet->OnMaxGroundSpeedChanged.AddUObject(this, &ThisClass::OnMaxGroundSpeedChanged);
}

void AEscapeChroniclesPlayerState::OnMaxGroundSpeedChanged(AActor* EffectInstigator, AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec, const float EffectMagnitude, const float OldValue,
	const float NewValue) const
{
	const AEscapeChroniclesCharacter* EscapeChroniclesCharacter = Cast<AEscapeChroniclesCharacter>(GetPawn());

	// Might also be a SpectatorPawn
	if (!IsValid(EscapeChroniclesCharacter))
	{
		return;
	}

	UCommonLegacyMovementSettings* CommonLegacyMovementSettings =
		EscapeChroniclesCharacter->GetCharacterMoverComponent()->FindSharedSettings_Mutable<
			UCommonLegacyMovementSettings>();

	// TODO: Скорее всего, это неправильно. С этим система предсказаний на клиенте ломается и получаются лаги при смене скорости
	// TODO: Нужно использовать MovementModifier
	if (ensureAlways(IsValid(CommonLegacyMovementSettings)))
	{
		// Update the MaxSpeed value in CharacterMoverComponent when the MaxGroundSpeed attribute changes
		CommonLegacyMovementSettings->MaxSpeed = NewValue;
	}
}