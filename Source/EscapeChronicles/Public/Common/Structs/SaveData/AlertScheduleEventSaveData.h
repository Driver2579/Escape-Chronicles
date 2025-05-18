// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Common/Structs/UniquePlayerID.h"
#include "AlertScheduleEventSaveData.generated.h"

USTRUCT()
struct FAlertScheduleEventSaveData
{
	GENERATED_BODY()

	UPROPERTY(Transient, SaveGame)
	TSet<FUniquePlayerID> WantedPlayers;
};