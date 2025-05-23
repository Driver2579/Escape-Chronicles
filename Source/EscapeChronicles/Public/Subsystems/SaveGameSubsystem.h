// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Common/Structs/UniquePlayerID.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameSubsystem.generated.h"

class AEscapeChroniclesPlayerState;
class UEscapeChroniclesSaveGame;

struct FActorSaveData;
struct FPlayerSaveData;

/**
 * A subsystem that handles saving and loading the game. It saves/loads all actors, all their components, and all world
 * subsystems that implement the Saveable interface, and that can be currently saved/loaded, except  it doesn't save
 * dynamically spawned actors which classes were not added in AllowedDynamicallySpawnedActorsClasses. Player-specific
 * actors (e.g., PlayerState, Pawn, PlayerController, etc.) are being saved/loaded separately by player's
 * FUniquePlayerID, which loading or generating this subsystem is also responsible for. Most of the data's
 * saving/loading is done by serializing the UPROPERTY(SaveGame) fields of ISaveable actors/components/subsystems.
 */
UCLASS()
class ESCAPECHRONICLES_API USaveGameSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USaveGameSubsystem();

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	bool IsGameSavingInProgress() const { return bGameSavingInProgress; }

	// Saves the game to the autosave slot
	void SaveGame(const bool bAsync = true)
	{
		SaveGame(AutoSaveSlotName, bAsync);
	}

	/**
	 * Saves the game to the specified slot name. The name of the level will be appended to this name.
	 * @remark It's recommended if you load the game first.
	 */
	virtual void SaveGame(FString SlotName, const bool bAsync = true);

	/**
	 * Saves all player-specific actors (e.g., Pawn, PlayerState, PlayerController, etc.) associated with the given
	 * PlayerState to the current save game object. If there is no save game object yet, then it will be created.
	 * @remark It doesn't save the save game object to the file. You need to use the SaveGame function for that.
	 */
	void SavePlayerOrBot(AEscapeChroniclesPlayerState* PlayerState)
	{
		SavePlayerOrBotChecked(GetOrCreateSaveGameObjectChecked(), PlayerState);
	}

	bool IsGameLoadingInProgress() const { return bGameLoadingInProgress; }

	// Loads the game from the AutoSaveSlotName. Also, initializes FUniquePlayerIDs for all connected players.
	void LoadGameAndInitializeUniquePlayerIDs(const bool bAsync = true)
	{
		LoadGameAndInitializeUniquePlayerIDs(AutoSaveSlotName, bAsync);
	}

	/**
	 * Loads the game from the specified slot name. The name of the level will be appended to this name. Also,
	 * initializes FUniquePlayerIDs for all connected players.
	 */
	virtual void LoadGameAndInitializeUniquePlayerIDs(FString SlotName, const bool bAsync = true);

	/**
	 * Tries to load the given player from the last save game object that was saved or loaded if any. Could fail in case
	 * there is no save game object or the player doesn't have anything to load. This function will also load
	 * FUniquePlayerID for the given PlayerState, or if it failed to load, then it's going to generate a new one in case
	 * it wasn't generated before.
	 * @return True if the player was loaded.
	 */
	bool LoadPlayerOrGenerateUniquePlayerID(AEscapeChroniclesPlayerState* PlayerState) const;

	/**
	 * This MUST be called when the bot gets the UniquePlayerID initialized. This is needed to remember the newly
	 * generated UniquePlayerID as taken to not allow other bots that don't have a UniquePlayerID yet to load the save
	 * data for this UniquePlayerID and take this UniquePlayerID.
	 */
	void RegisterBotUniquePlayerID(const FUniquePlayerID& UniquePlayerID)
	{
		RegisteredBotsUniquePlayerIDs.Add(UniquePlayerID);
	}

	/**
	 * Tries to load the given bot from the last save game object that was saved or loaded if any. Could fail in case
	 * there is no save game object or the bot doesn't have anything to load. This function will also load
	 * FUniquePlayerID for the given PlayerState, or if it failed to load, then it's going to generate a new one in case
	 * it wasn't generated before.
	 * @return True if the player was loaded.
	 */
	bool LoadBotOrGenerateUniquePlayerID(AEscapeChroniclesPlayerState* PlayerState) const;

	// Called right before the game is about to be saved
	FSimpleMulticastDelegate OnSaveGameCalled;

	// Called right before the game is about to be loaded
	FSimpleMulticastDelegate OnLoadGameCalled;

	// Called after the game was successfully saved
	FSimpleMulticastDelegate OnGameSaved;

	// Called after the game was successfully loaded
	FSimpleMulticastDelegate OnGameLoaded;

	// Usually happens due to unexpected errors (I have no idea what could happen, LOL)
	FSimpleMulticastDelegate OnFailedToSaveGame;

	// Usually happens when the given slot name doesn't exist
	FSimpleMulticastDelegate OnFailedToLoadGame;

protected:
	UEscapeChroniclesSaveGame* GetOrCreateSaveGameObjectChecked();

	// Saves all fields marked with "SaveGame" of the given object to the given byte array
	static void SaveObjectSaveGameFields(UObject* Object, TArray<uint8>& OutByteData);

	// Loads all fields marked with "SaveGame" of the given object from the given byte array
	static void LoadObjectSaveGameFields(UObject* Object, const TArray<uint8>& InByteData);

private:
	UPROPERTY(Transient)
	TObjectPtr<UEscapeChroniclesSaveGame> CurrentSaveGameObject;

	// Base name for the auto save slot. The name of the level will be appended to this name.
	UPROPERTY(EditDefaultsOnly, Category="Auto Saves")
	FString AutoSaveSlotName = TEXT("AutoSave");

	// Period in seconds between auto saves
	UPROPERTY(EditDefaultsOnly, Category="Auto Saves", meta=(ClampMin=0.1))
	float AutoSavePeriod = 5;

	// Asynchronously saves the game to the auto save slot. This function exists only to be called from the timer.
	void AutoSaveAsync()
	{
		SaveGame(true);
	}

	/**
	 * The character that is going to separate the slot name from the level name. Example: "SlotName_LevelName" where
	 * "_" is a separator.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Slot Names")
	FString SlotNameSeparator = TEXT("_");

	/**
	 * List of classes that are allowed to be saved even if they were dynamically spawned. Should be used for some
	 * game-specific actors (e.g., GameMode, GameState, etc.).
	 * @note This doesn't mean that actors in this list will be spawned or destroyed by the SaveGameSubsystem when
	 * loading the game. It's expected that the classes in this list are already spawned in the world when loading the
	 * game.
	 */
	TArray<TSubclassOf<AActor>> AllowedDynamicallySpawnedActorsClasses;

	bool IsAllowedDynamicallySpawnedActor(const AActor* Actor) const;

	// List of classes that are saved separately for each player (e.g., Pawn, PlayerState, PlayerController, etc.)
	TArray<TSubclassOf<AActor>> PlayerSpecificClasses;

	/**
	 * @return Whether PlayerSpecificClasses contains the given actor's class. 
	 */
	bool IsPlayerSpecificActor(const AActor* Actor) const;

	/**
	 * Saves all player-specific actors (e.g., Pawn, PlayerState, PlayerController, etc.) associated with the given
	 * PlayerState to the given save game object.
	 */
	void SavePlayerOrBotChecked(UEscapeChroniclesSaveGame* SaveGameObject, AEscapeChroniclesPlayerState* PlayerState);

	/**
	 * Saves an actor to the OutActorSaveData and prepares it to be saved in the given save game object (e.g., calling
	 * interface methods, subscribing it to delegates, etc.).
	 */
	void SaveActorToSaveDataChecked(AActor* Actor, FActorSaveData& OutActorSaveData);

	FSimpleMulticastDelegate OnGameSaved_Internal;

	void OnSavingFinished(const FString& SlotName, int32 UserIndex, bool bSuccess);

	// Whether the whole game is currently being saved
	bool bGameSavingInProgress = false;

	void OnLoadingSaveGameObjectFinished(const FString& SlotName, int32 UserIndex, USaveGame* SaveGameObject);

	/**
	 * Loads all player-specific actors (e.g., Pawn, PlayerState, PlayerController, etc.) from the given save game
	 * object and generates a UniquePlayerID for the given PlayerState if it doesn't have one.
	 * @remark All parameters here must be valid, and PlayerState must implement the ISaveable interface.
	 */
	static bool LoadPlayerOrGenerateUniquePlayerIdChecked(const UEscapeChroniclesSaveGame* SaveGameObject,
		AEscapeChroniclesPlayerState* PlayerState);

	/**
	 * Loads the UniquePlayerID and associated save data for the given PlayerState from the given SaveGameObject. If
	 * failed to load, then it generates a new FUniquePlayerID for the given PlayerState if it wasn't generated before.
	 * @remark All parameters here must be valid, and PlayerState must implement the ISaveable interface.
	 */
	static const FPlayerSaveData* LoadOrGenerateUniquePlayerIdAndLoadSaveDataForPlayer(
		const UEscapeChroniclesSaveGame* SaveGameObject, AEscapeChroniclesPlayerState* PlayerState);

	/**
	 * Finds the save data for the given FUniquePlayerID that should be generated by the PlayerState and updates the
	 * PlayerID in the given FUniquePlayerID if it's different from the one in the save data (only the LocalPlayerID is
	 * used for the search).
	 */
	static const FPlayerSaveData* LoadOfflinePlayerSaveDataAndPlayerID(const UEscapeChroniclesSaveGame* SaveGameObject,
		FUniquePlayerID& InOutUniquePlayerID);

	/**
	 * Loads all player-specific actors (e.g., Pawn, PlayerState, PlayerController, etc.) from the given save game
	 * object and generates a UniquePlayerID for the given PlayerState if it doesn't have one.
	 * @remark All parameters here must be valid, and PlayerState must implement the ISaveable interface.
	 */
	bool LoadBotOrGenerateUniquePlayerIdChecked(const UEscapeChroniclesSaveGame* SaveGameObject,
		AEscapeChroniclesPlayerState* PlayerState) const;

	/**
	 * Loads the UniquePlayerID and associated save data for the given PlayerState from the given SaveGameObject.
	 * This function doesn't really know which data are associated with the given PlayerState if it doesn't already have
	 * a UniquePlayerID, in which case it loads the first save data which associated UniquePlayerID wasn't already taken
	 * by any bot before during this session. If this function fails to load the save data for the given PlayerState,
	 * then it will generate a new FUniquePlayerID for the given PlayerState if it wasn't generated before.
	 * @remark All parameters here must be valid, and PlayerState must implement the ISaveable interface.
	 */
	const FPlayerSaveData* LoadOrGenerateUniquePlayerIdAndLoadSaveDataForBot(
		const UEscapeChroniclesSaveGame* SaveGameObject, AEscapeChroniclesPlayerState* PlayerState) const;

	// UniquePlayerIDs of all the bots that got the UniquePlayerID during this session
	TSet<FUniquePlayerID> RegisteredBotsUniquePlayerIDs;

	/**
	 * Loads all player-specific actors (e.g., Pawn, PlayerState, PlayerController, etc.) from the given save data.
	 * @remark All parameters here must be valid, and PlayerState must implement the ISaveable interface.
	 */
	static void LoadPlayerSpecificActors(const FPlayerSaveData& PlayerOrBotSaveData,
		AEscapeChroniclesPlayerState* PlayerState);

	// Loads an actor from the given ActorSaveData and notifies it about the loading by calling interface methods
	static void LoadActorFromSaveDataChecked(AActor* Actor, const FActorSaveData& ActorSaveData);

#if DO_ENSURE
	void EnsureBotUniquePlayerIdIsRegistered(const FUniquePlayerID& UniquePlayerID) const
	{
		ensureAlwaysMsgf(RegisteredBotsUniquePlayerIDs.Contains(UniquePlayerID),
			TEXT("Bots must manually call USaveGameSubsystem::RegisterBotUniquePlayerID function when their "
				"UniquePlayerID is initialized!"));
	}
#endif

	/**
	 * @return The PlatformUserIndex from the first found ULocalPlayer.
	 */
	int32 GetPlatformUserIndex() const;

	// Whether the whole game is currently being loaded
	bool bGameLoadingInProgress = false;
};