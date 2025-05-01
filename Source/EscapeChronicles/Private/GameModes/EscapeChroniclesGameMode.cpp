// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeChronicles/Public/GameModes/EscapeChroniclesGameMode.h"

#include "Controllers/PlayerControllers/EscapeChroniclesPlayerController.h"
#include "EscapeChronicles/Public/Characters/EscapeChroniclesCharacter.h"
#include "HUDs/EscapeChroniclesHUD.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AEscapeChroniclesGameMode::AEscapeChroniclesGameMode()
{
	DefaultPawnClass = AEscapeChroniclesCharacter::StaticClass();
	PlayerControllerClass = AEscapeChroniclesPlayerController::StaticClass();
	PlayerStateClass = AEscapeChroniclesPlayerState::StaticClass();
	HUDClass = AEscapeChroniclesHUD::StaticClass();
}