// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeChronicles/Public/GameModes/EscapeChroniclesGameMode.h"

#include "Controllers/PlayerControllers/EscapeChroniclesPlayerController.h"
#include "EscapeChronicles/Public/Characters/EscapeChroniclesCharacter.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "Subsystems/SaveGameSubsystem.h"

AEscapeChroniclesGameMode::AEscapeChroniclesGameMode()
{
	DefaultPawnClass = AEscapeChroniclesCharacter::StaticClass();
	PlayerControllerClass = AEscapeChroniclesPlayerController::StaticClass();
	PlayerStateClass = AEscapeChroniclesPlayerState::StaticClass();
}

void AEscapeChroniclesGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	if (!ensureAlways(SaveGameSubsystem))
	{
		return;
	}

	// Automatically try to load the game when it has started
	SaveGameSubsystem->OnGameLoaded.AddUObject(this, &AEscapeChroniclesGameMode::OnInitialGameLoadFinished);
	SaveGameSubsystem->TryLoadGame();
}

FString AEscapeChroniclesGameMode::InitNewPlayer(APlayerController* NewPlayerController,
	const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	const FString ParentResult = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	if (!ensureAlways(NewPlayerController->PlayerState))
	{
		return ParentResult;
	}

	const USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	/**
	 * Try to load the joined player from the last save game object that was saved or loaded if any, but only if the
	 * game was already loaded. If it wasn't loaded yet, then just skip loading the player because he's going to be
	 * loaded automatically by the SaveGameSubsystem when the game is loaded.
	 */
	if (ensureAlways(SaveGameSubsystem) && ensureAlways(NewPlayerController->PlayerState) && bInitialGameLoadFinished)
	{
#if DO_ENSURE
		ensureAlways(IsValid(NewPlayerController->GetPawn()));
#endif

		SaveGameSubsystem->TryLoadPlayerFromCurrentSaveGameObject(
			CastChecked<AEscapeChroniclesPlayerState>(NewPlayerController->PlayerState));
	}

	return ParentResult;
}

void AEscapeChroniclesGameMode::Logout(AController* Exiting)
{
	USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	// TODO: Pawn is invalid here. Maybe try notifying the GameMode in EndPlay of the controller?
	if (ensureAlways(SaveGameSubsystem))
	{
		// Save the game synchronously because we may close the game before the async save is finished
		SaveGameSubsystem->SaveGame(false);
	}

	Super::Logout(Exiting);
}