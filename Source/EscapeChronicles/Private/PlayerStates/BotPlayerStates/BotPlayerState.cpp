// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStates/BotPlayerStates/BotPlayerState.h"

#include "Subsystems/SaveGameSubsystem.h"

void ABotPlayerState::OnUniquePlayerIdInitializedOrChanged()
{
	Super::OnUniquePlayerIdInitializedOrChanged();

	USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	// This is required by the SaveGameSubsystem to be called
	if (ensureAlways(IsValid(SaveGameSubsystem)))
	{
		SaveGameSubsystem->RegisterBotUniquePlayerID(GetUniquePlayerID());
	}
}