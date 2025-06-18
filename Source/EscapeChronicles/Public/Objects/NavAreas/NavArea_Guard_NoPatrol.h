// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "NavArea_Guard_NoPatrol.generated.h"

// A navigation area that should be used for zones where guard bots can go but cannot patrol
UCLASS()
class ESCAPECHRONICLES_API UNavArea_Guard_NoPatrol : public UNavArea
{
	GENERATED_BODY()

	UNavArea_Guard_NoPatrol()
	{
		DrawColor = FColor::Orange;
	}
};