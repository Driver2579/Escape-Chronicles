// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "UInteractGameplayAbility.generated.h"

/**
 * Allows you to interact with AActor that have UInteractableComponent through own UInteractionManagerComponent
 */
UCLASS() 
class INTERACTIONSYSTEM_API UUInteractGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UUInteractGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override; 
};
