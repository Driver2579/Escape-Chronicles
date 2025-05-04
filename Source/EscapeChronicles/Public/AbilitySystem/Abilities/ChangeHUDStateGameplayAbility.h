// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "ChangeHUDStateGameplayAbility.generated.h"

// Opens and closes the inventory menu
UCLASS()
class ESCAPECHRONICLES_API UChangeHUDStateGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()
		
public:
	UChangeHUDStateGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag StateTag;
};
