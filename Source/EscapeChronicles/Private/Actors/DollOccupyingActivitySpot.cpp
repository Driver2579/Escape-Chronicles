// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/DollOccupyingActivitySpot.h"

ADollOccupyingActivitySpot::ADollOccupyingActivitySpot()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);

	bReplicates = true;
}