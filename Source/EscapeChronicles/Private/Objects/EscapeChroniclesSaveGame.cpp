// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/EscapeChroniclesSaveGame.h"

const FPlayerSaveData* UEscapeChroniclesSaveGame::FindOnlinePlayerSaveDataAndUpdatePlayerID(
	FUniquePlayerID& InOutUniquePlayerID) const
{
#if DO_CHECK
	check(InOutUniquePlayerID.IsValid());
#endif

#if DO_ENSURE
	ensureAlwaysMsgf(!InOutUniquePlayerID.NetID.IsEmpty(), TEXT("Online players must contain the NetID!"));
#endif

	const FPlayerSaveData* SaveData = OnlinePlayersSaveData.Find(InOutUniquePlayerID);

	if (!SaveData)
	{
		return nullptr;
	}

	const FUniquePlayerID* SavedUniquePlayerID = OnlinePlayersSaveData.FindKey(*SaveData);

#if DO_ENSURE
	// Make sure the NetID is set because this map should not contain players without NetIDs
	ensureAlways(!SavedUniquePlayerID->NetID.IsEmpty());

	// Make sure the NetIDs are equal
	ensureAlways(SavedUniquePlayerID->NetID == InOutUniquePlayerID.NetID);
#endif

	// Update the PlayerID if it's different from the one in the save data
	InOutUniquePlayerID.PlayerID = SavedUniquePlayerID->PlayerID;

	return SaveData;
}

void UEscapeChroniclesSaveGame::OverrideOnlinePlayerSaveData(const FUniquePlayerID& UniquePlayerID,
	const FPlayerSaveData& SavedPlayerData)
{
#if DO_CHECK
	check(UniquePlayerID.IsValid());
#endif

	// Remove the old data if it exists
	OnlinePlayersSaveData.Remove(UniquePlayerID);

	// Add the new data
	OnlinePlayersSaveData.Add(UniquePlayerID, SavedPlayerData);
}

bool UEscapeChroniclesSaveGame::FindOfflinePlayerSaveDataAndPlayerIdByLocalPlayerID(const int32 LocalPlayerID,
	const FPlayerSaveData*& OutPlayerSaveData, uint64& OutPlayerIdForUniquePlayerID) const
{
	for (const TPair<FUniquePlayerID, FPlayerSaveData>& Pair : OfflinePlayersSaveData)
	{
		if (Pair.Key.LocalPlayerID == LocalPlayerID)
		{
			OutPlayerSaveData = &Pair.Value;
			OutPlayerIdForUniquePlayerID = Pair.Key.PlayerID;

			return true;
		}
	}

	return false;
}

void UEscapeChroniclesSaveGame::OverrideOfflineStandalonePlayerSaveData(const FUniquePlayerID& UniquePlayerID,
	const FPlayerSaveData& SavedPlayerData)
{
#if DO_CHECK
	check(UniquePlayerID.IsValid());
#endif

#if DO_ENSURE
	ensureAlwaysMsgf(UniquePlayerID.NetID.IsEmpty(),
		TEXT("Offline standalone player must NOT contain the NetID!"));
#endif

	// Remove the old data if it exists
	OfflinePlayersSaveData.Remove(UniquePlayerID);

	// Add the new data
	OfflinePlayersSaveData.Add(UniquePlayerID, SavedPlayerData);
}