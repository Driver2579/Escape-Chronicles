// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "NavArea_MedicsZone.generated.h"

// A navigation area that should be used for zones where medics walk around
UCLASS()
class ESCAPECHRONICLES_API UNavArea_MedicsZone : public UNavArea
{
	GENERATED_BODY()

	UNavArea_MedicsZone()
	{
		DefaultCost = 2;
		DrawColor = FColor::Cyan;
	}
};