// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/WorldSubsystem.h"
#include "SaveGameSubsystem.generated.h"

/**
 * TODO: Add documentation to different classes
 * TODO: Add support for saving/loading different save game objects depending on the level
 * TODO: Add support for saving/loading some default dynamically spawned actors (such as Pawns, PlayerStates, etc.)
 */
UCLASS()
class ESCAPECHRONICLES_API USaveGameSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Saves the game to the AutoSaveSlotName
	void SaveGame() { SaveGame(AutoSaveSlotName); }

	// Saves the game to the specified slot name
	void SaveGame(const FString& SlotName);

	// Loads the game from the AutoSaveSlotName
	void TryLoadGame() const { TryLoadGame(AutoSaveSlotName); }

	// Loads the game from the specified slot name
	void TryLoadGame(const FString& SlotName) const;

	FSimpleMulticastDelegate OnGameSaved;
	FSimpleMulticastDelegate OnGameLoaded;

	// Usually happens due to unexpected errors
	FSimpleMulticastDelegate OnFailedToSaveGame;

	// Usually happens when the given slot name doesn't exist
	FSimpleMulticastDelegate OnFailedToLoadGame;

protected:
	static void SaveObjectSaveGameFields(UObject* Object, TArray<uint8>& OutByteData);
	static void LoadObjectSaveGameFields(UObject* Object, const TArray<uint8>& InByteData);

private:
	UPROPERTY(EditDefaultsOnly)
	FString AutoSaveSlotName = TEXT("AutoSave");

	FSimpleMulticastDelegate OnGameSaved_Internal;

	void OnAsyncSavingFinished(const FString& SlotName, int32 UserIndex, bool bSuccess) const;

	void OnAsyncLoadingSaveGameObjectFinished(const FString& SlotName, int32 UserIndex, USaveGame* SaveGameObject) const;
};