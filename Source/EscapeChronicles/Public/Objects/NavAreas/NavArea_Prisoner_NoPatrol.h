// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "NavArea_Prisoner_NoPatrol.generated.h"

// A navigation area that should be used for zones where prisoner bots can go but cannot patrol
UCLASS()
class ESCAPECHRONICLES_API UNavArea_Prisoner_NoPatrol : public UNavArea
{
	GENERATED_BODY()

	UNavArea_Prisoner_NoPatrol()
	{
		DrawColor = FColor::Blue;
	}
};