// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Common/Structs/SaveData/PlayerSaveData.h"
#include "EscapeChroniclesSaveGame.generated.h"

/**
 * An object that stores all the data needed to save an actor and its components. Used only internally by the
 * SaveGameSubsystem.
 */
UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	const FActorSaveData* FindStaticActorSaveData(const FName& ActorName) const
	{
		return StaticSavedActors.Find(ActorName);
	}

	// Should be used only for actors that are created with the level (not dynamically spawned)
	void AddStaticSavedActor(const FName& ActorName, const FActorSaveData& SavedActorData)
	{
		StaticSavedActors.Add(ActorName, SavedActorData);
	}

	void ClearStaticSavedActors()
	{
		StaticSavedActors.Empty();
	}

	const FPlayerSaveData* FindPlayerSaveData(const FUniqueNetIdRepl& PlayerID) const
	{
#if DO_CHECK
		check(PlayerID.IsValid());
#endif

		return PlayerSpecificSavedActors.Find(PlayerID->ToString());
	}

	void OverridePlayerSaveData(const FUniqueNetIdRepl& PlayerNetID, const FPlayerSaveData& SavedActorData);

private:
	/**
	 * Map of saved actors that are created with the level (not dynamically spawned).
	 * @tparam KeyType Name of the saved actor.
	 * @tparam ValueType Save data for the associated actor.
	 */
	UPROPERTY()
	TMap<FName, FActorSaveData> StaticSavedActors;

	/**
	 * @tparam KeyType Unique ID of the player converted to a string.
	 * @tparam ValueType Save data for the associated player.
	 */
	UPROPERTY()
	TMap<FString, FPlayerSaveData> PlayerSpecificSavedActors;
};