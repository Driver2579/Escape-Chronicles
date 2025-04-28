// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/AIControllers/EscapeChroniclesAIController.h"

#include "PlayerStates/EscapeChroniclesPlayerState.h"

void AEscapeChroniclesAIController::InitPlayerState()
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
}