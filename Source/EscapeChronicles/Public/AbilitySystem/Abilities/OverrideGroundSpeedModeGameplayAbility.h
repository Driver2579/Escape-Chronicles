// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeChroniclesGameplayAbility.h"
#include "OverrideGroundSpeedModeGameplayAbility.generated.h"

enum class EGroundSpeedMode : uint8;

/**
 * Calls the OverrideGroundSpeedMode function with the selected GroundSpeedMode passed on the owning character when
 * activated.\n
 * Calls the ResetGroundSpeedMode function on the owning character when ended.\n
 * Automatically ends when the input is released.
 */
UCLASS()
class ESCAPECHRONICLES_API UOverrideGroundSpeedModeGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

public:
	UOverrideGroundSpeedModeGameplayAbility();

protected:
	// GroundSpeedMode to set on the owning character when this ability is activated
	UPROPERTY(EditDefaultsOnly)
	EGroundSpeedMode GroundSpeedMode;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	FDelegateHandle OnAnyInstanceActivatedHandle;
};