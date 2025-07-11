// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "ActiveGameplayEffectHandle.h"
#include "ApplyGameplayEffectZone.generated.h"

class AEscapeChroniclesCharacter;
class UGameplayEffect;
class UAbilitySystemComponent;

struct FStreamableHandle;

// Applies a gameplay effect to the actor when it overlaps the zone and removes it when it leaves the zone
UCLASS()
class ESCAPECHRONICLES_API AApplyGameplayEffectZone : public ATriggerBox
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AApplyGameplayEffectZone();

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

private:
	TMap<TWeakObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandle> ActorsWithActiveGameplayEffects;

	// Gameplay Effect that will be applied to the player when he enters the zone
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> GameplayEffectClass;

	TSharedPtr<FStreamableHandle> LoadGameplayEffectClassHandle;

	// Calls ApplyGameplayEffectToActor for all overlapping characters
	void OnGameplayEffectClassLoaded();

	/**
	 * Applies gameplay effect to an actor. If no ASC is found on the given actor and the actor is a character, then it
	 * will wait for the PlayerState to be set on the character to find an ASC on it and then apply the gameplay effect
	 * to ASC if it was found.
	 * @remark GameplayEffectClass must be valid when this function is called!
	 */
	void ApplyGameplayEffectToActor(AActor* Actor);

	TMap<TWeakObjectPtr<AEscapeChroniclesCharacter>, FDelegateHandle> OnPlayerStateChangedDelegateHandles;

	// Tries to get the ASC from the NewPlayerState and apply the gameplay effect to it
	void OnCharacterPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState);

	// Applies gameplay effect to a valid ASC
	void ApplyGameplayEffectChecked(UAbilitySystemComponent* AbilitySystemComponent);
};