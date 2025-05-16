// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Triggers/ApplyGameplayEffectZone.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/AssetManager.h"

AApplyGameplayEffectZone::AApplyGameplayEffectZone()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AApplyGameplayEffectZone::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

#if DO_ENSURE
	ensureAlways(!GameplayEffectClass.IsNull());
#endif

	LoadGameplayEffectClassHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		GameplayEffectClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameplayEffectClassLoaded));
}

void AApplyGameplayEffectZone::OnGameplayEffectClassLoaded()
{
#if DO_ENSURE
	ensureAlways(GameplayEffectClass.IsValid());
#endif

	TSet<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);

	/**
	 * Apply gameplay effect to all actors that are currently overlapping with the zone once the gameplay effect class
	 * is loaded.
	 */
	for (const AActor* OverlappingActor : OverlappingActors)
	{
		ApplyGameplayEffectToActor(OverlappingActor);
	}
}

void AApplyGameplayEffectZone::ApplyGameplayEffectToActor(const AActor* Actor)
{
#if DO_CHECK
	check(GameplayEffectClass.IsValid());
#endif

	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor,
		false);

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	// Apply gameplay effect to the actor
	const FActiveGameplayEffectHandle ActiveGameplayEffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(
		GameplayEffectClass->GetDefaultObject<UGameplayEffect>(), 1, AbilitySystemComponent->MakeEffectContext(),
		AbilitySystemComponent->ScopedPredictionKey);

	/**
	 * If we successfully applied the gameplay effect and got a valid handle, then remember it (it will always be
	 * invalid on clients).
	 */
	if (ActiveGameplayEffectHandle.IsValid())
	{
		ActorsWithActiveGameplayEffects.Add(AbilitySystemComponent, ActiveGameplayEffectHandle);
	}
}

void AApplyGameplayEffectZone::NotifyActorEndOverlap(AActor* OtherActor)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(
		OtherActor, false);

	if (!IsValid(AbilitySystemComponent))
	{
		Super::NotifyActorEndOverlap(OtherActor);

		return;
	}

	// Try to find a gameplay effect handle for the actor
	const FActiveGameplayEffectHandle* ActiveGameplayEffectHandle = ActorsWithActiveGameplayEffects.Find(
		AbilitySystemComponent);

	// Check if the handle was found
	if (!ActiveGameplayEffectHandle)
	{
		Super::NotifyActorEndOverlap(OtherActor);

		return;
	}

	/**
	 * Remove gameplay effect from the actor (the handle is never valid on clients, so we don't need to check if we have
	 * an authority because we never get here otherwise).
	 */
	AbilitySystemComponent->RemoveActiveGameplayEffect(*ActiveGameplayEffectHandle);

	// Forget about a gameplay effect handle
	ActorsWithActiveGameplayEffects.Remove(AbilitySystemComponent);

	// Unload the gameplay effect class if there are no actors with active gameplay effects anymore
	if (ActorsWithActiveGameplayEffects.IsEmpty() && LoadGameplayEffectClassHandle.IsValid())
	{
		LoadGameplayEffectClassHandle->ReleaseHandle();
		LoadGameplayEffectClassHandle.Reset();
	}
}