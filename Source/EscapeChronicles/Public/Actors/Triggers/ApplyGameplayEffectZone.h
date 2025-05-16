// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "ActiveGameplayEffectHandle.h"
#include "ApplyGameplayEffectZone.generated.h"

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

	void OnGameplayEffectClassLoaded();

	void ApplyGameplayEffectToActor(const AActor* Actor);
};