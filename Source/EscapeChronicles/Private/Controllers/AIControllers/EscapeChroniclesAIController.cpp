// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/AIControllers/EscapeChroniclesAIController.h"

#include "Components/StateTreeAIComponent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AEscapeChroniclesAIController::AEscapeChroniclesAIController()
{
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("State Tree AI"));
	StateTreeAIComponent->SetStartLogicAutomatically(false);

	bWantsPlayerState = true;
}

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

void AEscapeChroniclesAIController::BeginPlay()
{
	Super::BeginPlay();

	// Start the StateTree logic if the pawn is already possessed (it will be started in OnPossess otherwise)
	if (IsValid(GetPawn()))
	{
		StateTreeAIComponent->StartLogic();
	}
}

void AEscapeChroniclesAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	/**
	 * Start the StateTree logic when the pawn is possessed if the game has already begun (it will be started in
	 * BeginPlay otherwise).
	 */
	if (HasActorBegunPlay())
	{
		StateTreeAIComponent->StartLogic();
	}
}