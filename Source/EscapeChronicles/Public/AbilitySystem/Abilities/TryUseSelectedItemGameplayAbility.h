// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeChroniclesGameplayAbility.h"
#include "TryUseSelectedItemGameplayAbility.generated.h"

/**
 * An ability that tries to use the currently selected item in the inventory. CommitAbility will be called only if the
 * item was used successfully.
 */
UCLASS()
class ESCAPECHRONICLES_API UTryUseSelectedItemGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

public:
	UTryUseSelectedItemGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};