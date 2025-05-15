// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvents/AlertScheduleEvent.h"

#include "GameState/EscapeChroniclesGameState.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void UAlertScheduleEvent::OnEventEnded()
{
	// If the gameplay effect isn't even loaded, then it's for sure isn't added to any player
	if (!WantedGameplayEffectClass.IsValid())
	{
		Super::OnEventEnded();

		return;
	}

	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	// GameState may be invalid at this stage
	if (!IsValid(GameState))
	{
		Super::OnEventEnded();

		return;
	}

	// Remove WantedGameplayEffect from all players that have it
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AEscapeChroniclesPlayerState* CastedPlayerState = Cast<AEscapeChroniclesPlayerState>(PlayerState);

		if (IsValid(CastedPlayerState))
		{
			/**
			 * NULL InstigatorAbilitySystemComponent means that the gameplay effect will be removed without checking if
			 * instigator is the same.
			 */
			CastedPlayerState->GetAbilitySystemComponent()->RemoveActiveGameplayEffectBySourceEffect(
				WantedGameplayEffectClass.Get(), nullptr);
		}
	}

	Super::OnEventEnded();
}