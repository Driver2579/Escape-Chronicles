// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/Saveable.h"
#include "Common/Structs/UniquePlayerID.h"
#include "EscapeChroniclesGameMode.generated.h"

// TODO: Create ScheduleEventManagerComponent here

UCLASS(MinimalAPI)
class AEscapeChroniclesGameMode : public AGameModeBase, public ISaveable
{
	GENERATED_BODY()

public:
	AEscapeChroniclesGameMode();

	FUniquePlayerIdManager& GetUniquePlayerIdManager() { return UniquePlayerIdManager; }

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	virtual void OnLoadGameCalled();

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
		const FString& Options, const FString& Portal = L"") override;

private:
	// A global FUniquePlayerIdManager for generating FUniquePlayerIDs in PlayerState
	FUniquePlayerIdManager UniquePlayerIdManager;

	FDelegateHandle OnGameLoadedDelegateHandle;
	FDelegateHandle OnFailedToLoadGameDelegateHandle;

	// Whether the game has finished the loading regardless of whether it was successful or not
	bool bInitialGameLoadFinishedOrFailed = false;

	// Called when the game finished loading regardless of whether the loading was successful or not
	void OnInitialGameLoadFinishedOrFailed();

	/**
	 * Players in this list are the players that joined the game before the game has finished its loading. They are
	 * going to be loaded when the loading is finished regardless of whether the loading was successful or not.
	 */
	TArray<TWeakObjectPtr<APlayerController>> PlayersWaitingToBeLoaded;

	/**
	 * Attempts to load the player now if he already has a pawn. If the player doesn't have a pawn yet, then he will be
	 * loaded once he gets it.
	 */
	void LoadPlayerNowOrWhenPawnIsPossessed(APlayerController* PlayerController) const;

	/**
	 * Called after the player has possessed a pawn for the first time in case we tried to load the player but failed
	 * because he didn't have a pawn yet.
	 */
	void OnPlayerToLoadPawnChanged(APawn* NewPawn) const;

	// Loads the player from the last save game object that was saved or loaded if any
	void LoadPlayerOrGenerateUniquePlayerIdForPlayer(const APlayerController* PlayerController) const;
};