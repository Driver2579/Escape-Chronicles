// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "OffsetCurrentSlotIndexGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPECHRONICLES_API UOffsetCurrentSlotIndexGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()
	
public:
	UOffsetCurrentSlotIndexGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly)
	int32 Offset = 0;
};
