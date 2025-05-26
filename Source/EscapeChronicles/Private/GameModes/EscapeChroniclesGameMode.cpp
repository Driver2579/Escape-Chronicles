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
	/**
	 * The code below is based on the default implementation of ChoosePlayerStart in AGameModeBase. It was mostly copied
	 * with some small modifications. The only difference here is that we check if we can spawn
	 * EscapeChroniclesPlayerStart.
	 */

	APlayerStart* FoundPlayerStart = nullptr;

	const UClass* PawnClass = GetDefaultPawnClassForController(Player);
	const APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;

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

void AEscapeChroniclesGameMode::OnInitialGameLoadFinishedOrFailed()
{
	bInitialGameLoadFinishedOrFailed = true;

	for (const TWeakObjectPtr<APlayerController>& PlayerController : PlayersWaitingToBeLoadedAndInitialized)
	{
		if (PlayerController.IsValid())
		{
			LoadAndInitPlayerNowOrWhenPawnIsPossessed(PlayerController.Get());
		}
	}
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

void AEscapeChroniclesGameMode::PostLoadInitPlayerOrBot(AEscapeChroniclesPlayerState* PlayerState)
{
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