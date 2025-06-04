// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AbilityComponents/GameplayAbilityComponent.h"
#include "RequireEnergyGameplayAbilityComponent.generated.h"

/**
 * Checks if the owner actor isn't out of energy in the CheckCost method. While the ability is active, it listens to the
 * event that the owner actor runs out of energy and cancels the ability if it does.
 */
UCLASS(DisplayName="Require Energy")
class ESCAPECHRONICLES_API URequireEnergyGameplayAbilityComponent : public UGameplayAbilityComponent
{
	GENERATED_BODY()

public:
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void OnEnergyAttributeValueChange(const FOnAttributeChangeData& OnAttributeChangeData) const;

	FDelegateHandle OnEnergyAttributeValueChangeDelegateHandle;
};