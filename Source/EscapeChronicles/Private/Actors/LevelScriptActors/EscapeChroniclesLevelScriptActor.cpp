// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/LevelScriptActors/EscapeChroniclesLevelScriptActor.h"

#include "Components/ActorComponents/DayNightCycleManagerComponent.h"

AEscapeChroniclesLevelScriptActor::AEscapeChroniclesLevelScriptActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DayNightCycleManagerComponent = CreateDefaultSubobject<UDayNightCycleManagerComponent>(
		TEXT("Day-Night Cycle Manager"));
}