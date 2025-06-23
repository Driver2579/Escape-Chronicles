// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/WinTrigger.h"

#include "GameState/EscapeChroniclesGameState.h"

void AWinTrigger::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!HasAuthority())
	{
		return;
	}

	const APawn* OtherPawn = Cast<APawn>(OtherActor);

	if (!ensureAlways(IsValid(OtherPawn)))
	{
		return;
	}

	const APlayerController* Controller = OtherPawn->GetController<APlayerController>();

	if (!IsValid(Controller))
	{
		return;
	}

	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	if (ensureAlways(IsValid(GameState)))
	{
		GameState->NetMulticast_WinGame();
	}
}