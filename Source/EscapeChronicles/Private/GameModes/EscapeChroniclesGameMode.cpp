// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeChronicles/Public/GameModes/EscapeChroniclesGameMode.h"
#include "EscapeChronicles/Public/Characters/EscapeChroniclesCharacter.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "UObject/ConstructorHelpers.h"

AEscapeChroniclesGameMode::AEscapeChroniclesGameMode()
{
	DefaultPawnClass = AEscapeChroniclesCharacter::StaticClass();
	PlayerStateClass = AEscapeChroniclesPlayerState::StaticClass();
}