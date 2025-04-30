// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Common/Structs/SaveData/OfflineStandalonePlayerSaveData.h"
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

	/**
	 * Finds the save data for the dynamically spawned actor of the given class. It doesn't support finding the child or
	 * parent class. The class should be exact.
	 */
	const FActorSaveData* FindDynamicallySpawnedActorSaveData(const TSoftClassPtr<AActor>& ActorClass) const
	{
		return DynamicallySpawnedSavedActors.Find(ActorClass);
	}

	// Should be used only for actors that were dynamically spawned (not created with the level)
	void AddDynamicallySpawnedSavedActor(const TSoftClassPtr<AActor>& ActorClass, const FActorSaveData& SavedActorData)
	{
		DynamicallySpawnedSavedActors.Add(ActorClass, SavedActorData);
	}

	// Clears both StaticSavedActors and DynamicallySpawnedSavedActors
	void ClearSavedActors()
	{
		StaticSavedActors.Empty();
		DynamicallySpawnedSavedActors.Empty();
	}

	/**
	 * Finds the save data for the given FUniquePlayerID and update the PlayerID in the struct if it's different from
	 * the one in the save data.
	 */
	const FPlayerSaveData* FindOnlinePlayerSaveDataAndUpdatePlayerID(FUniquePlayerID& InOutUniquePlayerID) const;

	// Should be used only for players that are connected online (with NetID)
	void OverrideOnlinePlayerSaveData(const FUniquePlayerID& UniquePlayerID, const FPlayerSaveData& SavedPlayerData);

	const FOfflineStandalonePlayerSaveData* GetOfflineStandalonePlayerSaveData() const
	{
		return OfflineStandalonePlayerSaveData.UniquePlayerID.IsValid() ? &OfflineStandalonePlayerSaveData : nullptr;
	}

	// Should be used only for the offline standalone player (without NetID)
	void OverrideOfflineStandalonePlayerSaveData(const FUniquePlayerID& UniquePlayerID,
		const FPlayerSaveData& SavedPlayerData);

	void ClearOfflineStandalonePlayerSaveData()
	{
		OfflineStandalonePlayerSaveData = FOfflineStandalonePlayerSaveData();
	}

	// Should be used only for bots (without NetID)
	void OverrideBotSaveData(const FUniquePlayerID& UniquePlayerID, const FPlayerSaveData& SavedBotData);

	void ClearBotsSaveData()
	{
		BotsSaveData.Empty();
	}

private:
	/**
	 * Map of saved actors that are created with the level (not dynamically spawned).
	 * @tparam KeyType Name of the saved actor.
	 * @tparam ValueType Save data for the associated actor.
	 */
	UPROPERTY()
	TMap<FName, FActorSaveData> StaticSavedActors;

	/**
	 * Map of saved actors that were dynamically spawned (not created with the level).
	 * @tparam KeyType Class of the saved actor.
	 * @tparam ValueType Save data for the associated actor.
	 */
	UPROPERTY()
	TMap<TSoftClassPtr<AActor>, FActorSaveData> DynamicallySpawnedSavedActors;

	/**
	 * Map of saved players that are connected online (with NetID).
	 * @tparam KeyType Unique ID of the player.
	 * @tparam ValueType Save data for the associated player.
	 */
	UPROPERTY()
	TMap<FUniquePlayerID, FPlayerSaveData> OnlinePlayersSaveData;

	/**
	 * Save data of the only player that is playing offline (standalone player without NetID). Once the player with this
	 * ID connects online, you should move it from here to OnlinePlayersSaveData.
	 */
	UPROPERTY()
	FOfflineStandalonePlayerSaveData OfflineStandalonePlayerSaveData;

	/**
	 * Map of saved bots.
	 * @tparam KeyType Unique ID of the bot (NetID should be empty).
	 * @tparam ValueType Save data for the associated bot.
	 */
	UPROPERTY()
	TMap<FUniquePlayerID, FPlayerSaveData> BotsSaveData;
};