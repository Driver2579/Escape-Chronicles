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

#if DO_CHECK
	check(PlayerState);
	check(PlayerState->IsA<AEscapeChroniclesPlayerState>());
#endif

	// Broadcast the PlayerState initialization event and clear its subscribers as it will never be called again
	OnPlayerStateInitialized.Broadcast(CastChecked<AEscapeChroniclesPlayerState>(PlayerState));
	OnPlayerStateInitialized.Clear();
}

void AEscapeChroniclesAIController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

#if DO_CHECK
	check(PlayerState);
	check(PlayerState->IsA<AEscapeChroniclesPlayerState>());
#endif

	// Broadcast the PlayerState initialization event and clear its subscribers as it will never be called again
	OnPlayerStateInitialized.Broadcast(CastChecked<AEscapeChroniclesPlayerState>(PlayerState));
	OnPlayerStateInitialized.Clear();
}

void AEscapeChroniclesAIController::CallOrRegister_OnPlayerStateInitialized(
	const FOnPlayerStateInitializedDelegate::FDelegate& Callback)
{
	// Execute the given callback now if the PlayerState is already initialized
	if (PlayerState)
	{
#if DO_CHECK
		check(PlayerState.IsA<AEscapeChroniclesPlayerState>());
#endif

		Callback.Execute(CastChecked<AEscapeChroniclesPlayerState>(PlayerState));
	}
	// Otherwise, register the callback to be executed when the PlayerState is initialized
	else
	{
		OnPlayerStateInitialized.Add(Callback);
	}
}

void AEscapeChroniclesAIController::OnBotInitialized(AEscapeChroniclesPlayerState* InitializedPlayerState,
	const bool bLoaded) const
{
	if (InitializedPlayerState != PlayerState)
	{
		return;
	}

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