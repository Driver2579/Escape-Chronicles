// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/AIControllers/EscapeChroniclesAIController.h"

#include "Components/StateTreeAIComponent.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AEscapeChroniclesAIController::AEscapeChroniclesAIController()
{
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("State Tree AI"));
	StateTreeAIComponent->SetStartLogicAutomatically(false);

	bWantsPlayerState = true;
}

void AEscapeChroniclesAIController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AEscapeChroniclesGameMode* GameMode = GetWorld()->GetAuthGameMode<AEscapeChroniclesGameMode>();

	if (IsValid(GameMode))
	{
		GameMode->OnPlayerOrBotInitialized.AddUObject(this, &ThisClass::OnBotInitialized);
	}
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

void AEscapeChroniclesAIController::OnBotInitialized(AEscapeChroniclesPlayerState* InitializedPlayerState)
{
#if DO_ENSURE
	ensureAlways(HasActorBegunPlay());
	ensureAlways(IsValid(GetPawn()));
#endif

	/**
	 * Start the State Tree logic once the bot is fully initialized. This is the place where the controller should've
	 * already called the BeginPlay and OnPossessed functions.
	 */
	StateTreeAIComponent->StartLogic();
}