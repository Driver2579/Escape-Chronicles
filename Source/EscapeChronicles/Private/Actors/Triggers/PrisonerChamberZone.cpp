// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Triggers/PrisonerChamberZone.h"

#include "Components/BoxComponent.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"

APrisonerChamberZone::APrisonerChamberZone()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	OuterZoneBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Outer Zone"));
	OuterZoneBoxComponent->SetupAttachment(RootComponent);

	InnerZoneBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Inner Zone"));
	InnerZoneBoxComponent->SetupAttachment(RootComponent);

	PlayerOwnershipComponent = CreateDefaultSubobject<UPlayerOwnershipComponent>(TEXT("Player Ownership"));
}