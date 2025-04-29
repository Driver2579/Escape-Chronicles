// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/WorldSubsystem.h"
#include "SaveGameSubsystem.generated.h"

class AEscapeChroniclesPlayerState;
class UEscapeChroniclesSaveGame;

struct FActorSaveData;

/**
 * TODO: Add documentation to different classes
 * TODO: Add support for saving/loading different save game objects depending on the level
 */
UCLASS()
class ESCAPECHRONICLES_API USaveGameSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USaveGameSubsystem();

	// Saves the game to the autosave slot
	void SaveGame(const bool bAsync = true) { SaveGame(AutoSaveSlotName, bAsync); }

	/**
	 * Saves the game to the specified slot name.
	 * @remark It's recommended if you load the game first.
	 */
	virtual void SaveGame(const FString& SlotName, const bool bAsync = true);

	// Loads the game from the AutoSaveSlotName
	void TryLoadGame(const bool bAsync = true) { TryLoadGame(AutoSaveSlotName, bAsync); }

	// Loads the game from the specified slot name
	virtual void TryLoadGame(const FString& SlotName, const bool bAsync = true);

	FSimpleMulticastDelegate OnGameSaved;
	FSimpleMulticastDelegate OnGameLoaded;

	// Usually happens due to unexpected errors (I have no idea what could happen, LOL)
	FSimpleMulticastDelegate OnFailedToSaveGame;

	// Usually happens when the given slot name doesn't exist
	FSimpleMulticastDelegate OnFailedToLoadGame;

	/**
	 * Tries to load the given player from the last save game object that was saved or loaded if any. Could fail in case
	 * there is no save game object or the player doesn't have anything to load.
	 */
	bool TryLoadPlayerFromCurrentSaveGameObject(AEscapeChroniclesPlayerState* PlayerState) const;

protected:
	UEscapeChroniclesSaveGame* GetOrCreateSaveGameObjectChecked();

	static void SaveObjectSaveGameFields(UObject* Object, TArray<uint8>& OutByteData);
	static void LoadObjectSaveGameFields(UObject* Object, const TArray<uint8>& InByteData);

private:
	UPROPERTY(Transient)
	TObjectPtr<UEscapeChroniclesSaveGame> CurrentSaveGameObject;

	UPROPERTY(EditDefaultsOnly)
	FString AutoSaveSlotName = TEXT("AutoSave");

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

	// TODO: Implement saving bots
	/**
	 * Saves all player-specific actors (e.g., Pawn, PlayerState, PlayerController, etc.) associated with the given
	 * PlayerState to the given save game object
	 */
	void SavePlayerToSaveGameObjectChecked(UEscapeChroniclesSaveGame* SaveGameObject,
		AEscapeChroniclesPlayerState* PlayerState);

	/**
	 * Saves an actor to the OutActorSaveData and prepares it to be saved in the given save game object (e.g., calling
	 * interface methods, subscribing it to delegates, etc.).
	 */
	void SaveActorToSaveDataChecked(AActor* Actor, FActorSaveData& OutActorSaveData);

	FSimpleMulticastDelegate OnGameSaved_Internal;

	void OnSavingFinished(const FString& SlotName, int32 UserIndex, bool bSuccess) const;

	void OnLoadingSaveGameObjectFinished(const FString& SlotName, int32 UserIndex, USaveGame* SaveGameObject);

	// TODO: Implement loading bots
	/**
	 * Loads all player-specific actors (e.g., Pawn, PlayerState, PlayerController, etc.) from the given save game
	 * object. Also, generates a UniquePlayerID for the given PlayerState if it doesn't have one.
	 */
	static bool LoadPlayerFromSaveGameObjectChecked(const UEscapeChroniclesSaveGame* SaveGameObject,
		AEscapeChroniclesPlayerState* PlayerState);

	// Loads an actor from the given ActorSaveData and notifies it about the loading by calling interface methods
	static void LoadActorFromSaveDataChecked(AActor* Actor, const FActorSaveData& ActorSaveData);
};