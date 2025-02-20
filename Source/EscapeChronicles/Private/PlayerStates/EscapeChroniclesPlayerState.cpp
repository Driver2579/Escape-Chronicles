// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStates/EscapeChroniclesPlayerState.h"

#include "AbilitySystem/EscapeChroniclesAbilitySystemComponent.h"

AEscapeChroniclesPlayerState::AEscapeChroniclesPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UEscapeChroniclesAbilitySystemComponent>(
		TEXT("AbilitySystemComponent"));

	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}