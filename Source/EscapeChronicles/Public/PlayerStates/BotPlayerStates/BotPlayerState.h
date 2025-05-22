// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "BotPlayerState.generated.h"

// A base class for the PlayerState that bots should use
UCLASS()
class ESCAPECHRONICLES_API ABotPlayerState : public AEscapeChroniclesPlayerState
{
	GENERATED_BODY()

protected:
	virtual void OnUniquePlayerIdInitializedOrChanged() override;
};