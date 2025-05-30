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

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerOrBotInitializedOrLogoutDelegate,
		AEscapeChroniclesPlayerState* PlayerState)

	// Called when the player or bot is fully initialized
	FOnPlayerOrBotInitializedOrLogoutDelegate OnPlayerOrBotInitialized;

	// Called when the player or bot logs out
	FOnPlayerOrBotInitializedOrLogoutDelegate OnPlayerOrBotLogout;

protected:
	virtual void OnLoadGameCalled();

	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
		const FString& Options, const FString& Portal = L"") override;

	virtual void PostLoadInitPlayerOrBot(AEscapeChroniclesPlayerState* PlayerState);

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
	 * Loads the player from the last save game object that was saved or loaded if any, or if failed to load, then
	 * generates the new FUniquePlayerID for this player. After that, it calls PostLoadInitPlayerOrBot for the player.
	 */
	void LoadAndInitPlayer(const APlayerController* PlayerController);

	/**
	 * Loads the bot from the last save game object that was saved or loaded if any, or if failed to load, then
	 * generates the new FUniquePlayerID for this bot. After that, it calls PostLoadInitPlayerOrBot for the bot.
	 */
	void LoadAndInitBot_Implementation(AEscapeChroniclesPlayerState* PlayerState);
};