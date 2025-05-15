// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Common/Structs/GameplayDateTime.h"
#include "BedtimeScheduleEventSaveData.generated.h"

USTRUCT()
struct FBedtimeScheduleEventSaveData
{
	GENERATED_BODY()

	UPROPERTY(Transient, SaveGame)
	FGameplayDateTime EventStartDateTime;
};