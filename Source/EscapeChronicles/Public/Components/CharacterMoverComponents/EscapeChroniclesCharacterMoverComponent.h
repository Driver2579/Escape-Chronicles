// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "EscapeChroniclesCharacterMoverComponent.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesCharacterMoverComponent : public UCharacterMoverComponent
{
	GENERATED_BODY()

public:
	virtual bool CanCrouch() override { return !IsAirborne(); }
};