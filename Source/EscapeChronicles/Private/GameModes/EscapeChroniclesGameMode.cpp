// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeChronicles/Public/GameModes/EscapeChroniclesGameMode.h"

#include "EngineUtils.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Actors/PlayerStarts/EscapeChroniclesPlayerStart.h"
#include "Components/ActorComponents/BotSpawnerComponent.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "Components/ActorComponents/ScheduleEventManagerComponent.h"
#include "Controllers/PlayerControllers/EscapeChroniclesPlayerController.h"
#include "Engine/PlayerStartPIE.h"
#include "EscapeChronicles/Public/Characters/EscapeChroniclesCharacter.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "Subsystems/SaveGameSubsystem.h"

AEscapeChroniclesGameMode::AEscapeChroniclesGameMode()
{
	GameStateClass = AEscapeChroniclesGameState::StaticClass();
	DefaultPawnClass = AEscapeChroniclesCharacter::StaticClass();
	PlayerControllerClass = AEscapeChroniclesPlayerController::StaticClass();
	PlayerStateClass = AEscapeChroniclesPlayerState::StaticClass();

	BotSpawnerComponent = CreateDefaultSubobject<UBotSpawnerComponent>(TEXT("Bot Spawner"));

	ScheduleEventManagerComponent = CreateDefaultSubobject<UScheduleEventManagerComponent>(
		TEXT("Schedule Event Manager"));
}

void AEscapeChroniclesGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	if (!ensureAlways(IsValid(SaveGameSubsystem)))
	{
		return;
	}

	/**
	 * We should reset the loading state each time the loading is called to make sure new joined players don't load the
	 * old data if the game is currently asynchronously loading.
	 */
	SaveGameSubsystem->OnLoadGameCalled.AddUObject(this, &ThisClass::OnLoadGameCalled);

	// Automatically try to load the game when it has started
	SaveGameSubsystem->LoadGameAndInitializeUniquePlayerIDs();
}

AActor* AEscapeChroniclesGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
#if DO_CHECK
	check(IsValid(Player));
#endif

	/**
	 * The code below is based on the default implementation of ChoosePlayerStart in AGameModeBase. It was mostly copied
	 * with some small modifications. The main difference here is that we check if we can spawn
	 * EscapeChroniclesPlayerStart.
	 */

	APlayerStart* FoundPlayerStart = nullptr;

	// Try to get the pawn from the controller
	const APawn* PawnToFit = Player->GetPawn();

	// If no pawn was found, they use the default pawn
	if (!IsValid(PawnToFit))
	{
		const UClass* PawnClass = GetDefaultPawnClassForController(Player);
		PawnToFit = IsValid(PawnClass) ? PawnClass->GetDefaultObject<APawn>() : nullptr;
	}

	const AEscapeChroniclesPlayerState* PlayerState = Cast<AEscapeChroniclesPlayerState>(Player->PlayerState);

	TArray<APlayerStart*> UnOccupiedStartPoints;
	TArray<APlayerStart*> OccupiedStartPoints;

	UWorld* World = GetWorld();

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		// Always prefer the first "Play from Here" PlayerStart if we find one while in PIE mode
		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			FoundPlayerStart = PlayerStart;

			break;
		}

		const AEscapeChroniclesPlayerStart* EscapeChroniclesPlayerStart = Cast<AEscapeChroniclesPlayerStart>(
			PlayerStart);

		/**
		 * Don't spawn the player at this PlayerStart if it's an AEscapeChroniclesPlayerStart and it doesn't allow to
		 * spawn a pawn for the given controller here.
		 */
		if (EscapeChroniclesPlayerStart && !EscapeChroniclesPlayerStart->CanSpawnPawn(PlayerState))
		{
			continue;
		}

		FVector ActorLocation = PlayerStart->GetActorLocation();
		const FRotator ActorRotation = PlayerStart->GetActorRotation();

		if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
		{
			UnOccupiedStartPoints.Add(PlayerStart);
		}
		else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
		{
			OccupiedStartPoints.Add(PlayerStart);
		}
	}

	if (!FoundPlayerStart)
	{
		if (UnOccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}

	return FoundPlayerStart;
}

void AEscapeChroniclesGameMode::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	/**
	 * The code below is based on the default implementation of RestartPlayerAtPlayerStart in AGameModeBase. It was
	 * mostly copied with some small modifications. The main difference here is that we reset the pawn's location and
	 * rotation here at the ones of the StartSpot even if there is already a pawn existing, which the default
	 * implementation doesn't do in this case.
	 */

	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}

	if (!StartSpot)
	{
#if !NO_LOGGING
		UE_LOG(LogGameMode, Warning, TEXT("RestartPlayerAtPlayerStart: Player start not found"));
#endif

		return;
	}

#if !NO_LOGGING
	UE_LOG(LogGameMode, Verbose, TEXT("RestartPlayerAtPlayerStart %s"),
		(NewPlayer && NewPlayer->PlayerState) ? *NewPlayer->PlayerState->GetPlayerName() : TEXT("Unknown"));
#endif

	if (MustSpectate(Cast<APlayerController>(NewPlayer)))
	{
#if !NO_LOGGING
		UE_LOG(LogGameMode, Verbose,
			TEXT("RestartPlayerAtPlayerStart: Tried to restart a spectator-only player!"));
#endif

		return;
	}

	// Try to create a pawn to use of the default class for this player if it doesn't already have one
	if (!NewPlayer->GetPawn() && GetDefaultPawnClassForController(NewPlayer) != nullptr)
	{
		APawn* NewPawn = SpawnDefaultPawnFor(NewPlayer, StartSpot);

		if (IsValid(NewPawn))
		{
			NewPlayer->SetPawn(NewPawn);
		}
	}
	
	if (!IsValid(NewPlayer->GetPawn()))
	{
		FailedToRestartPlayer(NewPlayer);
	}
	else
	{
		// Tell the start spot it was used
		InitStartSpot(StartSpot, NewPlayer);

		const FRotator SpawnRotation = StartSpot->GetActorRotation();

		// Reset the pawn's location and rotation to the StartSpot's ones
		NewPlayer->GetPawn()->SetActorLocation(StartSpot->GetActorLocation());
		NewPlayer->GetPawn()->SetActorRotation(SpawnRotation);

		FinishRestartPlayer(NewPlayer, SpawnRotation);
	}
}

void AEscapeChroniclesGameMode::RestartPlayerAtTransform(AController* NewPlayer, const FTransform& SpawnTransform)
{
	/**
	 * The code below is based on the default implementation of RestartPlayerAtPlayerStart in AGameModeBase. It was
	 * mostly copied with some small modifications. The main difference here is that we reset the pawn's location and
	 * rotation here at the ones of the SpawnTransform even if there is already a pawn existing, which the default
	 * implementation doesn't do in this case.
	 */

	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}

#if !NO_LOGGING
	UE_LOG(LogGameMode, Verbose, TEXT("RestartPlayerAtTransform %s"),
		(NewPlayer && NewPlayer->PlayerState) ? *NewPlayer->PlayerState->GetPlayerName() : TEXT("Unknown"));
#endif

	if (MustSpectate(Cast<APlayerController>(NewPlayer)))
	{
#if !NO_LOGGING
		UE_LOG(LogGameMode, Verbose,
			TEXT("RestartPlayerAtTransform: Tried to restart a spectator-only player!"));
#endif

		return;
	}

	// Try to create a pawn to use of the default class for this player if it doesn't already have one
	if (!NewPlayer->GetPawn() && GetDefaultPawnClassForController(NewPlayer) != nullptr)
	{
		// Try to create a pawn to use of the default class for this player
		APawn* NewPawn = SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform);

		if (IsValid(NewPawn))
		{
			NewPlayer->SetPawn(NewPawn);
		}
	}

	if (!IsValid(NewPlayer->GetPawn()))
	{
		FailedToRestartPlayer(NewPlayer);
	}
	else
	{
		const FRotator SpawnRotation = SpawnTransform.GetRotation().Rotator();

		// Reset the pawn's location and rotation to the SpawnTransform's ones
		NewPlayer->GetPawn()->SetActorLocation(SpawnTransform.GetLocation());
		NewPlayer->GetPawn()->SetActorRotation(SpawnRotation);

		FinishRestartPlayer(NewPlayer, SpawnRotation);
	}
}
void AEscapeChroniclesGameMode::OnLoadGameCalled()
{
	bInitialGameLoadFinishedOrFailed = false;

	USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	if (!ensureAlways(IsValid(SaveGameSubsystem)))
	{
		return;
	}

	// === Subscribe to delegates but make sure we bind the delegates only once ===

	if (!OnGameLoadedDelegateHandle.IsValid())
	{
		OnGameLoadedDelegateHandle = SaveGameSubsystem->OnGameLoaded.AddUObject(this,
			&ThisClass::OnInitialGameLoadFinishedOrFailed);
	}

	if (!OnFailedToLoadGameDelegateHandle.IsValid())
	{
		OnFailedToLoadGameDelegateHandle = SaveGameSubsystem->OnFailedToLoadGame.AddUObject(this,
			&ThisClass::OnInitialGameLoadFinishedOrFailed);
	}
}

FString AEscapeChroniclesGameMode::InitNewPlayer(APlayerController* NewPlayerController,
	const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	const FString ParentResult = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

#if DO_CHECK
	check(IsValid(NewPlayerController));
#endif

	if (!ensureAlways(NewPlayerController->PlayerState))
	{
		return ParentResult;
	}

	/**
	 * We can load the joined player from the last save game object that was saved or loaded (if any) only if the
	 * game was already loaded or failed to be loaded. If the loading hasn't finished yet, then add the player to the
	 * list of players to load. Even though the players are loaded automatically by the SaveGameSubsystem when the game
	 * is loaded, this new joined player may still not be initialized correctly when this happens. FUniquePlayerID for
	 * this player will be generated by the same logic but also even if the game fails to load.
	 */
	if (bInitialGameLoadFinishedOrFailed)
	{
		LoadAndInitPlayerNowOrWhenPawnIsPossessed(NewPlayerController);
	}
	else
	{
		PlayersWaitingToBeLoadedAndInitialized.Add(NewPlayerController);
	}

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	// Set the player name with the nickname from the online subsystem
	if (ensureAlways(OnlineSubsystem))
	{
		const IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();

		if (ensureAlways(IdentityInterface.IsValid()))
		{
			const FString PlayerName = IdentityInterface->GetPlayerNickname(
				*NewPlayerController->PlayerState->GetUniqueId());

			NewPlayerController->PlayerState->SetPlayerName(PlayerName);
		}
	}

	return ParentResult;
}

void AEscapeChroniclesGameMode::LoadAndInitBot(AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	// If the game has already finished or failed the loading, then we can load and initialize the bot immediately
	if (bInitialGameLoadFinishedOrFailed)
	{
		LoadAndInitBot_Implementation(PlayerState);
	}
	// Otherwise, add the bot to the list of bots to be initialized when the game finishes or fails the loading
	else
	{
		BotsWaitingToBeLoadedAndInitialized.Add(PlayerState);
	}
}

void AEscapeChroniclesGameMode::OnInitialGameLoadFinishedOrFailed()
{
	bInitialGameLoadFinishedOrFailed = true;

	// Load and initialize all players that were waiting for the game to finish loading
	for (const TWeakObjectPtr<APlayerController>& PlayerController : PlayersWaitingToBeLoadedAndInitialized)
	{
		if (PlayerController.IsValid())
		{
			LoadAndInitPlayerNowOrWhenPawnIsPossessed(PlayerController.Get());
		}
	}

	// Load and initialize all bots that were waiting for the game to finish loading
	for (const TWeakObjectPtr<AEscapeChroniclesPlayerState>& PlayerState : BotsWaitingToBeLoadedAndInitialized)
	{
		if (PlayerState.IsValid())
		{
			LoadAndInitBot_Implementation(PlayerState.Get());
		}
	}

	// Clear the arrays as we don't need them anymore
	PlayersWaitingToBeLoadedAndInitialized.Empty();
	BotsWaitingToBeLoadedAndInitialized.Empty();
}

void AEscapeChroniclesGameMode::LoadAndInitPlayerNowOrWhenPawnIsPossessed(APlayerController* PlayerController)
{
#if DO_CHECK
	check(IsValid(PlayerController));
#endif

	// If the controller already possesses a pawn, then we can already load the player
	if (IsValid(PlayerController->GetPawn()))
	{
		LoadAndInitPlayer(PlayerController);
	}
	// Otherwise, wait for the pawn to be possessed
	else
	{
		PlayerController->GetOnNewPawnNotifier().AddUObject(this, &ThisClass::OnPlayerToLoadPawnChanged);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void AEscapeChroniclesGameMode::OnPlayerToLoadPawnChanged(APawn* NewPawn)
{
	if (!IsValid(NewPawn))
	{
		return;
	}

	APlayerController* PlayerController = NewPawn->GetController<APlayerController>();

	if (!ensureAlways(IsValid(PlayerController)))
	{
		return;
	}

	// Stop listening for the new pawn possessed event because we needed it only for the first pawn
	PlayerController->GetOnNewPawnNotifier().RemoveAll(this);

	LoadAndInitPlayer(PlayerController);
}

void AEscapeChroniclesGameMode::LoadAndInitPlayer(const APlayerController* PlayerController)
{
#if DO_CHECK
	check(IsValid(PlayerController));
	check(PlayerController->PlayerState);
	check(PlayerController->PlayerState.IsA<AEscapeChroniclesPlayerState>());
#endif

#if DO_ENSURE
	ensureAlways(bInitialGameLoadFinishedOrFailed);
#endif

	const USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		PlayerController->PlayerState);

	if (ensureAlways(IsValid(SaveGameSubsystem)))
	{
		SaveGameSubsystem->LoadPlayerOrGenerateUniquePlayerID(PlayerState);
	}

	PostLoadInitPlayerOrBot(PlayerState);
}

void AEscapeChroniclesGameMode::LoadAndInitBot_Implementation(AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

#if DO_ENSURE
	ensureAlways(bInitialGameLoadFinishedOrFailed);
#endif

	const USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	if (ensureAlways(IsValid(SaveGameSubsystem)))
	{
		SaveGameSubsystem->LoadBotOrGenerateUniquePlayerID(PlayerState);
	}

	PostLoadInitPlayerOrBot(PlayerState);
}

void AEscapeChroniclesGameMode::PostLoadInitPlayerOrBot(AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_ENSURE
	ensureAlwaysMsgf(bInitialGameLoadFinishedOrFailed,
		TEXT("You must call this function only after the initial loading of the game has finished or failed!"));
#endif

#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	// Register the player in the player ownership system because we have a valid UniquePlayerID now
	UPlayerOwnershipComponent::RegisterPlayer(PlayerState);

	OnPlayerOrBotInitialized.Broadcast(PlayerState);
}

void AEscapeChroniclesGameMode::BeginPlay()
{
	Super::BeginPlay();

	BotSpawnerComponent->SpawnBots();
}

void AEscapeChroniclesGameMode::Logout(AController* Exiting)
{
#if DO_CHECK
	check(Exiting->PlayerState);
	check(Exiting->PlayerState->IsA<AEscapeChroniclesPlayerState>());
#endif

	OnPlayerOrBotLogout.Broadcast(CastChecked<AEscapeChroniclesPlayerState>(Exiting->PlayerState));

	Super::Logout(Exiting);
}

void AEscapeChroniclesGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/**
	 * Save the game at the end of the game but only if initial loading was finished or failed because we don't want to
	 * accidentally override the data that wasn't even loaded yet.
	 */
	if (bInitialGameLoadFinishedOrFailed)
	{
		USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

		if (ensureAlways(IsValid(SaveGameSubsystem)))
		{
			// Save the game synchronously because the game is about to be closed
			SaveGameSubsystem->SaveGame(false);
		}
	}

	Super::EndPlay(EndPlayReason);
}