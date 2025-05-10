// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "EscapeChroniclesLevelScriptActor.generated.h"

class UDayNightCycleManagerComponent;

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEscapeChroniclesLevelScriptActor();

	UDayNightCycleManagerComponent* GetDayNightCycleManagerComponent() const
	{
		return DayNightCycleManagerComponent;
	}

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDayNightCycleManagerComponent> DayNightCycleManagerComponent;
};