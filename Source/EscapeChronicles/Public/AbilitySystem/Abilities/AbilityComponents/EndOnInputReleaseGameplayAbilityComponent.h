// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AbilityComponents/GameplayAbilityComponent.h"
#include "EndOnInputReleaseGameplayAbilityComponent.generated.h"

UCLASS(DisplayName="End On Input Release")
class ESCAPECHRONICLES_API UEndOnInputReleaseGameplayAbilityComponent : public UGameplayAbilityComponent
{
	GENERATED_BODY()

protected:
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	bool bReplicateEndAbility = true;
};
