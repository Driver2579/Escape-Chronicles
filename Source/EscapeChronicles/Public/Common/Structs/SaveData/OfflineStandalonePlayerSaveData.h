// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PlayerSaveData.h"
#include "Common/Structs/UniquePlayerID.h"
#include "OfflineStandalonePlayerSaveData.generated.h"

USTRUCT()
struct FOfflineStandalonePlayerSaveData
{
	GENERATED_BODY()

	// ID of the offline standalone player. NetID should remain empty here.
	UPROPERTY()
	FUniquePlayerID UniquePlayerID;

	// Save data of the offline host player
	UPROPERTY()
	FPlayerSaveData PlayerSaveData;
};