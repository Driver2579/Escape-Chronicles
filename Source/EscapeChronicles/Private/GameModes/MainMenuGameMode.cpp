// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/MainMenuGameMode.h"

#include "GameInstances/EscapeChroniclesGameInstance.h"

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	UEscapeChroniclesGameInstance* GameInstance = GetWorld()->GetGameInstance<UEscapeChroniclesGameInstance>();

	// Destroy the session each time we travel to the main menu to ensure clients break their connection to the session
	if (ensureAlways(IsValid(GameInstance)))
	{
		GameInstance->DestroySession();
	}
}
