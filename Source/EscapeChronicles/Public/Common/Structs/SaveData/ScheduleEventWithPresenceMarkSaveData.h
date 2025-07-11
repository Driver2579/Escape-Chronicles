// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Common/Structs/UniquePlayerID.h"
#include "ScheduleEventWithPresenceMarkSaveData.generated.h"

USTRUCT()
struct FScheduleEventWithPresenceMarkSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TSet<FUniquePlayerID> CheckedInPlayers;

	UPROPERTY(SaveGame)
	TSet<FUniquePlayerID> PlayersThatMissedAnEvent;
};