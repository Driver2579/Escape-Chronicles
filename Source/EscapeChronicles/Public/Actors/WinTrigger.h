// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "WinTrigger.generated.h"

UCLASS()
class ESCAPECHRONICLES_API AWinTrigger : public ATriggerBox
{
	GENERATED_BODY()

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};