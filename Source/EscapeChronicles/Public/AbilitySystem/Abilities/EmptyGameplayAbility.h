// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "EmptyGameplayAbility.generated.h"

// Empty ability (mainly intended for BP using only components)
UCLASS()
class ESCAPECHRONICLES_API UEmptyGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override; 

private:
	UPROPERTY(EditDefaultsOnly)
	bool bAutoEndAbility = true;

	UPROPERTY(EditDefaultsOnly)
	bool bReplicateCancelAbility = true;

	UPROPERTY(EditDefaultsOnly)
	bool bReplicateEndAbility = true;
};
