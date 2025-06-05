// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UAbilitySystemComponent;
struct FActiveGameplayEffectHandle;

struct FCombatEvents
{
	/**
	 * Broadcasted when a punch successfully hits a target.
	 * @param Instigator The ability system component of the actor who performed the punch.
	 * @param Target The ability system component of the actor who was hit by the punch.
	 * @param TargetAppliedEffectHandle Handle to the gameplay effect applied to the target as a result of the punch.
	 */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPunchHit, UAbilitySystemComponent* Instigator,
		UAbilitySystemComponent* Target, FActiveGameplayEffectHandle TargetAppliedEffectHandle);

	static FOnPunchHit OnPunchHit;
};
