// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeChroniclesGameMode.h"
#include "EscapeChroniclesCharacter.h"
#include "UObject/ConstructorHelpers.h"

AEscapeChroniclesGameMode::AEscapeChroniclesGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
