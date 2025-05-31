// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ActivitySpots/Bed.h"

#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "Objects/Subsystems/WorldSubsystems/BedsManagementSubsystem.h"

ABed::ABed()
{
	PlayerOwnershipComponent = CreateDefaultSubobject<UPlayerOwnershipComponent>(TEXT("PlayerOwnershipComponent"));
}

void ABed::OccupySpot(AEscapeChroniclesCharacter* Character)
{
	Super::OccupySpot(Character);
	
	UBedsManagementSubsystem* BedsManagementSubsystem = GetWorld()->GetSubsystem<UBedsManagementSubsystem>();

	if (!ensureAlways(IsValid(BedsManagementSubsystem)))
	{
		return;
	}
	
	BedsManagementSubsystem->IncreaseSleepingPlayersNumber();
}

void ABed::UnoccupySpot(AEscapeChroniclesCharacter* Character)
{
	Super::UnoccupySpot(Character);

	UBedsManagementSubsystem* BedsManagementSubsystem = GetWorld()->GetSubsystem<UBedsManagementSubsystem>();

	if (!ensureAlways(IsValid(BedsManagementSubsystem)))
	{
		return;
	}

	BedsManagementSubsystem->DecreaseSleepingPlayersNumber();
}
