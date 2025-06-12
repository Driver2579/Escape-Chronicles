// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "CarryCharacterGameplayAbility.generated.h"

class AActivitySpot;
struct FStreamableHandle;
class AEscapeChroniclesCharacter;

//
UCLASS()
class ESCAPECHRONICLES_API UCarryCharacterGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly)
	float CarryPickupDistance = 1;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActivitySpot> ActivitySpotToDropClass;
};
