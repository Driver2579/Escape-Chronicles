// Fill out your copyright notice in the Description page of Project Settings.

#include "CheatManagers/EscapeChroniclesCheatManager.h"

#include "GameState/EscapeChroniclesGameState.h"

void UEscapeChroniclesCheatManager::Cheat_SetGameDateTime(const uint64 Day, const uint8 Hour, const uint8 Minute) const
{
	const UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	AEscapeChroniclesGameState* GameState = World->GetGameState<AEscapeChroniclesGameState>();

	if (IsValid(GameState))
	{
		GameState->SetCurrentGameDateTime(FGameplayDateTime(Day, Hour, Minute));
	}
}

void UEscapeChroniclesCheatManager::Cheat_SetTimeDilation(const float TimeSpeed) const
{
	const UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	AWorldSettings* WorldSettings = World->GetWorldSettings();

	if (IsValid(WorldSettings))
	{
		WorldSettings->SetTimeDilation(TimeSpeed);
	}
}