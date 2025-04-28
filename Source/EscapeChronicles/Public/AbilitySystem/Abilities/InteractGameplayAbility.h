// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "InteractGameplayAbility.generated.h"

// Allows you to interact with actor that have UInteractableComponent through own UInteractionManagerComponent
UCLASS()
class ESCAPECHRONICLES_API UInteractGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()
	
public:
	UInteractGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override; 
};
