// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Saveable.generated.h"

class UEscapeChroniclesSaveGame;

UINTERFACE()
class USaveable : public UInterface
{
	GENERATED_BODY()
};

class ISaveable
{
	GENERATED_BODY()

public:
	virtual void OnGameSaved() {}
	virtual void OnGameLoaded() = 0;
};