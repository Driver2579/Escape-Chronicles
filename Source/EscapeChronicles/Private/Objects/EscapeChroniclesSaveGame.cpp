// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/EscapeChroniclesSaveGame.h"

void UEscapeChroniclesSaveGame::OverridePlayerSaveData(const FUniqueNetIdRepl& PlayerNetID,
	const FPlayerSaveData& SavedActorData)
{
#if DO_CHECK
	check(PlayerNetID.IsValid());
#endif

	const FString PlayerNetIDString = PlayerNetID->ToString();

	// Remove the old data if it exists
	PlayerSpecificSavedActors.Remove(PlayerNetIDString);

	// Add the new data
	PlayerSpecificSavedActors.Add(PlayerNetIDString, SavedActorData);
}