// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "CarryCharacterGameplayAbility.generated.h"

class AActivitySpot;
struct FStreamableHandle;
class AEscapeChroniclesCharacter;

/**
 * Gameplay Ability that handles picking up and dropping characters. Uses raycasting to detect characters and activity
 * spots for interaction.
 */
UCLASS()
class ESCAPECHRONICLES_API UCarryCharacterGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

protected:
	// Main execution point for the ability
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	// Maximum distance for picking up/dropping characters
	UPROPERTY(EditDefaultsOnly)
	float CarryPickupDistance = 1;

	// Class of ActivitySpot where characters can be dropped
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActivitySpot> ActivitySpotToDropClass;
};
