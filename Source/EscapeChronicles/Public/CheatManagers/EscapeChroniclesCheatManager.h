// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "EscapeChroniclesCheatManager.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesCheatManager : public UCheatManager
{
	GENERATED_BODY()

	// Sets the current day and time in the game
	UFUNCTION(Exec)
	void Cheat_SetGameDateTime(const uint64 Day, const uint8 Hour, const uint8 Minute) const;

	// Sets the global time speed
	UFUNCTION(Exec)
	void Cheat_SetTimeDilation(const float TimeSpeed) const;
};