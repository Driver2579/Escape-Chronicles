// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/PlayerStarts/EscapeChroniclesPlayerStart.h"

#include "AbilitySystemComponent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AEscapeChroniclesPlayerStart::AEscapeChroniclesPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AEscapeChroniclesPlayerStart::CanSpawnPawn(const AEscapeChroniclesPlayerState* PlayerState) const
{
	// We can't spawn a pawn here if its PlayerState is invalid
	if (!IsValid(PlayerState))
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

	// If we failed to get the AbilitySystemComponent, then spawn the pawn only if there are no required gameplay tags
	if (!IsValid(AbilitySystemComponent))
	{
		return RequiredGameplayTagsToSpawn.IsEmpty();
	}

	// Check if the pawn has all the required gameplay tags to spawn at this PlayerStart
	return AbilitySystemComponent->HasAllMatchingGameplayTags(RequiredGameplayTagsToSpawn);
}