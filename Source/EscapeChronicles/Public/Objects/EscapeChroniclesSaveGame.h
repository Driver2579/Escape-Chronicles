// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Common/Structs/UniquePlayerID.h"
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
	const FString& GetLevelName() const { return LevelName; }
	void SetLevelName(const FString& NewLevelName) { LevelName = NewLevelName; }

	/**
	 * Finds the save data for the world subsystem of the given class. It doesn't support finding the child or parent
	 * class. The class should be exact.
	 */
	const FSaveData* FindWorldSubsystemSaveData(const TSoftClassPtr<UWorldSubsystem>& WorldSubsystemClass) const
	{
		return WorldSubsystemsSaveData.Find(WorldSubsystemClass);
	}

	void AddWorldSubsystemSaveData(const TSoftClassPtr<UWorldSubsystem>& WorldSubsystemClass,
		const FSaveData& SavedWorldSubsystemData)
	{
		WorldSubsystemsSaveData.Add(WorldSubsystemClass, SavedWorldSubsystemData);
	}

	void ClearSavedWorldSubsystems()
	{
		WorldSubsystemsSaveData.Empty();
	}

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

	const FPlayerSaveData* FindOfflinePlayerSaveData(const FUniquePlayerID& UniquePlayerID) const
	{
		return OfflinePlayersSaveData.Find(UniquePlayerID);
	}

	/**
	 * @param LocalPlayerID LocalPlayerID from FUniquePlayerID of the player to find.
	 * @param OutPlayerSaveData Save data for the found player.
	 * @param OutPlayerIdForUniquePlayerID PlayerID that was associated with the given UniquePlayerID's LocalPlayerID.
	 * @return True if the player was found. False otherwise.
	 */
	bool FindOfflinePlayerSaveDataAndPlayerIdByLocalPlayerID(const int32 LocalPlayerID,
		const FPlayerSaveData*& OutPlayerSaveData, uint64& OutPlayerIdForUniquePlayerID) const;

	// Should be used only for the offline standalone player (without NetID)
	void OverrideOfflineStandalonePlayerSaveData(const FUniquePlayerID& UniquePlayerID,
		const FPlayerSaveData& SavedPlayerData);

	/**
	 * This should be called when players from OfflinePlayersSaveData connect online. This is a move function instead of
	 * the clear function because there could be a save data for another local player that isn't currently playing.
	 */
	void MoveOfflinePlayersSaveDataToOnlinePlayersSaveData()
	{
		OnlinePlayersSaveData.Append(OfflinePlayersSaveData);
		OfflinePlayersSaveData.Empty();
	}

	const TMap<FUniquePlayerID, FPlayerSaveData>& GetBotsSaveData() const { return BotsSaveData; }

	FPlayerSaveData* FindBotSaveData(const FUniquePlayerID& UniquePlayerID)
	{
		return BotsSaveData.Find(UniquePlayerID);
	}

	// Should be used only for bots (without NetID)
	void AddBotSaveData(const FUniquePlayerID& UniquePlayerID, const FPlayerSaveData& SavedBotData);

	void ClearBotsSaveData()
	{
		BotsSaveData.Empty();
	}

private:
	// Name of the level associated with this save game object
	UPROPERTY()
	FString LevelName;

	/**
	 * Map of saved WorldSubsystems.
	 * @tparam KeyType Class of the saved WorldSubsystem.
	 * @tparam ValueType Save data for the associated WorldSubsystem.
	 */
	UPROPERTY()
	TMap<TSoftClassPtr<UWorldSubsystem>, FSaveData> WorldSubsystemsSaveData;

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
	 * Save data of the players that are playing offline (players without NetID (PIE players or split-screen players)).
	 * Once the player with one of these ID connects online, you should move him from here to OnlinePlayersSaveData.
	 * @tparam KeyType ID of the offline player. NetID should remain empty here.
	 * @tparam Save data of the offline player.
	 */
	UPROPERTY()
	TMap<FUniquePlayerID, FPlayerSaveData> OfflinePlayersSaveData;

	/**
	 * Map of saved bots.
	 * @tparam KeyType Unique ID of the bot (NetID should be empty).
	 * @tparam ValueType Save data for the associated bot.
	 */
	UPROPERTY()
	TMap<FUniquePlayerID, FPlayerSaveData> BotsSaveData;
};