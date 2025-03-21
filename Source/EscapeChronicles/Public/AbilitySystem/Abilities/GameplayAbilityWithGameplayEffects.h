// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "GameplayAbilityWithGameplayEffects.generated.h"

/**
 * Gameplay ability that applies gameplay effects to the owner actor when activated and removes them when ended.
 * @remark EndAbility should be called manually by child classes or externally!
 */
UCLASS()
class ESCAPECHRONICLES_API UGameplayAbilityWithGameplayEffects : public UEscapeChroniclesGameplayAbility
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