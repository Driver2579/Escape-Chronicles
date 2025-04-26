// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "EscapeChroniclesAIController.generated.h"

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesAIController : public ADetourCrowdAIController
{
	GENERATED_BODY()

public:
	virtual void InitPlayerState() override;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AEscapeChroniclesPlayerState> PlayerStateClassOverride;
};