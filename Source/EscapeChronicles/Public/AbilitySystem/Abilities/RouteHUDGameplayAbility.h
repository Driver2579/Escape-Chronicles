// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "RouteHUDGameplayAbility.generated.h"

// Opens and closes the selected route
UCLASS()
class ESCAPECHRONICLES_API URouteHUDGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()
		
public:
	URouteHUDGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	// Route the player's HUD will go to when the ability is activated
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag RouteTag;
};
