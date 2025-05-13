// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AbilityComponents/GameplayAbilityComponent.h"
#include "AutoCommitGameplayAbilityComponent.generated.h"

UCLASS(DisplayName="Auto Commit")
class ESCAPECHRONICLES_API UAutoCommitGameplayAbilityComponent : public UGameplayAbilityComponent
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
