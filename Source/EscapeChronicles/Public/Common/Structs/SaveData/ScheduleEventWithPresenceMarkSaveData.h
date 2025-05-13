// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Common/Structs/UniquePlayerID.h"
#include "ScheduleEventWithPresenceMarkSaveData.generated.h"

USTRUCT()
struct FScheduleEventWithPresenceMarkSaveData
{
	GENERATED_BODY()

	UPROPERTY(Transient, SaveGame)
	TArray<FUniquePlayerID> CheckedInPlayers;
};