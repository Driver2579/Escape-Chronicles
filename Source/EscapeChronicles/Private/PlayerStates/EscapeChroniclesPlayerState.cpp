// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStates/EscapeChroniclesPlayerState.h"

#include "Common/DataAssets/AbilitySystemSet.h"
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
	// TODO: Add attributes initializations here
}