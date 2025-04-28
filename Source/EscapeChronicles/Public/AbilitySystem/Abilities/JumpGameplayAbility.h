// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeChroniclesGameplayAbility.h"
#include "JumpGameplayAbility.generated.h"

/**
 * Calls the Jump function on the owning character when activated.\n
 * Calls the StopJumping function on the owning character when ended.\n
 * Doesn't automatically end itself when activated. Instead, it ends when the input is released.
 */
UCLASS()
class ESCAPECHRONICLES_API UJumpGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

public:
	UJumpGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};