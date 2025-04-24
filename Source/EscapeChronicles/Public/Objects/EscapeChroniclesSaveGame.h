// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EscapeChroniclesSaveGame.generated.h"

struct FActorSaveData;

/**
 * An object that stores all the data needed to save an actor and its components. Used only internally by the
 * SaveGameSubsystem.
 */
UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	const FActorSaveData* FindActorSaveData(const FName& ActorName) const
	{
		return StaticSavedActors.Find(ActorName);
	}

	// Should be used only for actors that are created with the level (not dynamically spawned)
	void AddStaticSavedActor(const FName& ActorName, const FActorSaveData& SavedActorData)
	{
		StaticSavedActors.Add(ActorName, SavedActorData);
	}

	void ClearSavedActors()
	{
		StaticSavedActors.Empty();
	}

private:
	/**
	 * Map of saved actors that are created with the level (not dynamically spawned).
	 * @tparam KeyType Name of the saved actor.
	 * @tparam ValueType Save data for the associated actor.
	 */
	UPROPERTY()
	TMap<FName, FActorSaveData> StaticSavedActors;
};