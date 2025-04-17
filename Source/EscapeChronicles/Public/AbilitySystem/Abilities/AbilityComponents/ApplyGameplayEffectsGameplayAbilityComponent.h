// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityComponent.h"
#include "ApplyGameplayEffectsGameplayAbilityComponent.generated.h"

/**
 * Gameplay ability component that applies gameplay effects to the owner actor when ability is activated and removes
 * them when ability is ended.
 */
UCLASS(DisplayName="Apply Gameplay Effects")
class ESCAPECHRONICLES_API UApplyGameplayEffectsGameplayAbilityComponent : public UGameplayAbilityComponent
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void OnGameplayEffectsLoaded();

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	/**
	 * Gameplay effects in this array will be automatically applied to the owner actor when the ability is activated and
	 * removed when the ability is ended.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TArray<TSoftClassPtr<UGameplayEffect>> GameplayEffectsToApplyWhileActive;

	TArray<FActiveGameplayEffectHandle> AppliedGameplayEffects;
};