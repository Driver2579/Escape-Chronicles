// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/EscapeChroniclesSaveGame.h"

const FPlayerSaveData* UEscapeChroniclesSaveGame::FindPlayerSaveDataAndUpdatePlayerID(
	FUniquePlayerID& InOutUniquePlayerID) const
{
#if DO_CHECK
	check(InOutUniquePlayerID.IsValid());
#endif

	const FPlayerSaveData* SaveData = PlayersSaveData.Find(InOutUniquePlayerID);

	if (SaveData)
	{
		const FUniquePlayerID* SavedUniquePlayerID = PlayersSaveData.FindKey(*SaveData);

#if DO_CHECK
		// Make sure the NetIDs are equal
		check(SavedUniquePlayerID->NetID == InOutUniquePlayerID.NetID);
#endif

		// Update the PlayerID if it's different from the one in the save data
		InOutUniquePlayerID.PlayerID = SavedUniquePlayerID->PlayerID;
	}

	return SaveData;
}

void UEscapeChroniclesSaveGame::OverridePlayerSaveData(const FUniquePlayerID& UniquePlayerID,
	const FPlayerSaveData& SavedActorData)
{
#if DO_CHECK
	check(UniquePlayerID.IsValid());
#endif

	// Remove the old data if it exists
	PlayersSaveData.Remove(UniquePlayerID);

	// Add the new data
	PlayersSaveData.Add(UniquePlayerID, SavedActorData);
}