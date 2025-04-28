// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "DropItemGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPECHRONICLES_API UDropItemGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDropItemGameplayAbility();

protected:
	UPROPERTY(EditDefaultsOnly)
	FVector TrowingDirection;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

};
