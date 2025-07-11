// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "UnoccupyActivitySpotGameplayAbility.generated.h"

/**
 * Activation unoccupies the character from the activity spot in which the character is using (the activity spot gets
 * from player state)
 */
UCLASS()
class ESCAPECHRONICLES_API UUnoccupyActivitySpotGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

public:
	UUnoccupyActivitySpotGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
