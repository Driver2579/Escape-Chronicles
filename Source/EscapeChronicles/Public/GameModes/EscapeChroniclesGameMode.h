// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/Saveable.h"
#include "Common/Structs/UniquePlayerID.h"
#include "EscapeChroniclesGameMode.generated.h"

class AEscapeChroniclesPlayerState;
class UBotSpawnerComponent;
class UScheduleEventManagerComponent;

struct FScheduleEventData;

UCLASS(MinimalAPI)
class AEscapeChroniclesGameMode : public AGameModeBase, public ISaveable
{
	GENERATED_BODY()

public:
	AEscapeChroniclesGameMode();

	UBotSpawnerComponent* GetBotSpawnerComponent() const { return BotSpawnerComponent; }

	UScheduleEventManagerComponent* GetScheduleEventManagerComponent() const
	{
		return ScheduleEventManagerComponent;
	}

	FUniquePlayerIdManager& GetUniquePlayerIdManager() { return UniquePlayerIdManager; }

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;
	virtual void RestartPlayerAtTransform(AController* NewPlayer, const FTransform& SpawnTransform) override;

	/**
	 * Loads the bot and initializes it after it's loaded or failed to be loaded. This will be delayed until the initial
	 * game loading is finished or failed if it isn't already.
	 * @remark Players are loaded and initialized automatically, but bots MUST call this manually after they are
	 * spawned!
	 * @remark See LoadAndInitPlayer_Implementation for more details.
	 */
	void LoadAndInitBot(AEscapeChroniclesPlayerState* PlayerState);

	/**
	 * @param PlayerState The PlayerState of the player or bot that was initialized.
	 * @param bLoaded Whether the player or bot was loaded successfully or not before the final initialization. If
	 * false, then it's the first time the player or bot has joined this game.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerOrBotInitializedDelegate, AEscapeChroniclesPlayerState* PlayerState,
		const bool bLoaded);

	/**
	 * @param UniquePlayerID FUniquePlayerID of the player or bot that you want to know whether he's initialized or not.
	 * @param bOutLoaded Whether the player or bot with the given UniquePlayerID was loaded at least once.
	 * @return Whether the player or bot with the given UniquePlayerID is fully initialized, meaning that the
	 * OnPlayerOrBotInitialized delegate was called for this player or bot.
	 */
	bool IsPlayerOrBotFullyInitialized(const FUniquePlayerID& UniquePlayerID, bool& bOutLoaded) const;

	// Called when the player or bot is fully initialized
	FOnPlayerOrBotInitializedDelegate OnPlayerOrBotInitialized;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerOrBotLogoutDelegate, AEscapeChroniclesPlayerState* PlayerState)

	// Called when the player or bot logs out
	FOnPlayerOrBotLogoutDelegate OnPlayerOrBotLogout;

protected:
	virtual void OnLoadGameCalled();

	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
		const FString& Options, const FString& Portal = L"") override;

	/**
	 * Called after the player or bot is loaded from the last save game object that was saved or loaded (if any) or
	 * failed to be loaded. This is the place where final initialization of the player or bot happens.
	 * @param PlayerState PlayerState of the player or bot that was loaded or failed to be loaded.
	 * @param bLoaded Whether the player or bot was loaded successfully or not. If false, then it's the first time the
	 * player of bot has joined this game.
	 */
	virtual void PostLoadInitPlayerOrBot(AEscapeChroniclesPlayerState* PlayerState, const bool bLoaded);

	virtual void BeginPlay() override;

	virtual void Logout(AController* Exiting) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBotSpawnerComponent> BotSpawnerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UScheduleEventManagerComponent> ScheduleEventManagerComponent;

	// A global FUniquePlayerIdManager for generating FUniquePlayerIDs in PlayerState
	UPROPERTY(Transient, SaveGame)
	FUniquePlayerIdManager UniquePlayerIdManager;

	FDelegateHandle OnGameLoadedDelegateHandle;
	FDelegateHandle OnFailedToLoadGameDelegateHandle;

	// Whether the game has finished the loading regardless of whether it was successful or not
	bool bInitialGameLoadFinishedOrFailed = false;

	// Called when the game finished loading regardless of whether the loading was successful or not
	void OnInitialGameLoadFinishedOrFailed();

	/**
	 * Players in this list are the players that joined the game before the game has finished its loading. They are
	 * going to be loaded and initialized when the loading is finished regardless of whether the loading was successful
	 * or not.
	 */
	TArray<TWeakObjectPtr<APlayerController>> PlayersWaitingToBeLoadedAndInitialized;

	/**
	 * Bots in this list are the bots that called LoadAndInitBot before the game has finished its loading. They
	 * are going to be initialized when the loading is finished regardless of whether the loading was successful or not.
	 */
	TSet<TWeakObjectPtr<AEscapeChroniclesPlayerState>> BotsWaitingToBeLoadedAndInitialized;

	/**
	 * Attempts to load and initialized the player now if he already has a pawn. If the player doesn't have a pawn yet,
	 * then he will be loaded and initialized once he gets it.
	 */
	void LoadAndInitPlayerNowOrWhenPawnIsPossessed(APlayerController* PlayerController);

	/**
	 * Called after the player has possessed a pawn for the first time in case we tried to load the player but failed
	 * because he didn't have a pawn yet.
	 */
	void OnPlayerToLoadPawnChanged(APawn* NewPawn);

	/**
	 * Attempts to load and initialize the player now if the pawn has already begin play. If the pawn hasn't begun play
	 * yet, then he will be loaded and initialized once he begins play.
	 * @remark You should never call this function before the pawn is possessed!
	 */
	void LoadAndInitPlayerNowOrWhenPawnBeginsPlay(const APlayerController* PlayerController);

	FDelegateHandle OnPlayerToLoadPawnBegunPlayDelegateHandle;

	/**
	 * Called when the pawn of the player that we want to load has begun play in case we tried to load the player but
	 * failed because his pawn hadn't begun play yet.
	 * @remark This is never called before the pawn is possessed.
	 */
	void OnPlayerToLoadPawnBegunPlay(APawn* Pawn);

	/**
	 * Loads the player from the last save game object that was saved or loaded if any, or if failed to load, then
	 * generates the new FUniquePlayerID for this player. After that, it calls PostLoadInitPlayerOrBot for the player.
	 */
	void LoadAndInitPlayer(const APlayerController* PlayerController);

	/**
	 * Loads the bot from the last save game object that was saved or loaded if any, or if failed to load, then
	 * generates the new FUniquePlayerID for this bot. After that, it calls PostLoadInitPlayerOrBot for the bot.
	 */
	void LoadAndInitBot_Implementation(AEscapeChroniclesPlayerState* PlayerState);

	/**
	 * List of players or bots that were fully initialized, meaning that the PostLoadInitPlayerOrBot was called for
	 * them.
	 * @param KeyType The FUniquePlayerID of the player or bot that was initialized.
	 * @param ValueType Whether this player or bot was loaded at least once or not.
	 */
	TMap<FUniquePlayerID, bool> FullyInitializedPlayersOrBots;
};