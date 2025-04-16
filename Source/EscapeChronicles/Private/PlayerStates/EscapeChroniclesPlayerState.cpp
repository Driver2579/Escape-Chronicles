// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStates/EscapeChroniclesPlayerState.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Common/DataAssets/AbilitySystemSet.h"

AEscapeChroniclesPlayerState::AEscapeChroniclesPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UEscapeChroniclesAbilitySystemComponent>(
		TEXT("AbilitySystemComponent"));

	InventoryManagerComponent = CreateDefaultSubobject<UInventoryManagerComponent>(TEXT("InventoryManagerComponent"));
	
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AEscapeChroniclesPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/**
	 * Give AbilitySystemSets to the AbilitySystemComponent. We do this here because SetupInputComponent is called
	 * before BeginPlay.
	 */
	for (const UAbilitySystemSet* AbilitySystemSet : AbilitySystemSets)
	{
		if (AbilitySystemSet)
		{
			AbilitySystemSet->GiveToAbilitySystem(AbilitySystemComponent);
		}
	}
}