// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Triggers/ApplyGameplayEffectZone.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/AssetManager.h"
#include "GameFramework/PlayerState.h"

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

	// Apply gameplay effect to an actor if the gameplay effect is already loaded
	if (GameplayEffectClass.IsValid())
	{
		ApplyGameplayEffectToActor(OtherActor);
	}
	// Otherwise, start loading the gameplay effect class but only if we didn't already start loading it before
	else if (!LoadGameplayEffectClassHandle.IsValid())
	{
		LoadGameplayEffectClassHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			GameplayEffectClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameplayEffectClassLoaded));
	}
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
	for (AActor* OverlappingActor : OverlappingActors)
	{
		ApplyGameplayEffectToActor(OverlappingActor);
	}
}

void AApplyGameplayEffectZone::ApplyGameplayEffectToActor(AActor* Actor)
{
#if DO_CHECK
	check(GameplayEffectClass.IsValid());
#endif

	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor,
		false);

	// Apply gameplay effect already if we found the ASC
	if (IsValid(AbilitySystemComponent))
	{
		ApplyGameplayEffectChecked(AbilitySystemComponent);

		return;
	}

	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(Actor);

	/**
	 * If we didn't find the ASC now, then it might be a character that has no PlayerState with ASC yet. Wait for
	 * PlayerState to be initialized in this case.
	 */
	if (IsValid(Character))
	{
		const FDelegateHandle DelegateHandle = Character->OnPlayerStateChangedDelegate.AddUObject(this,
			&ThisClass::OnCharacterPlayerStateChanged);

		// Remember the delegate handle for this character to unsubscribe from this delegate later
		OnPlayerStateChangedDelegateHandles.Add(Character, DelegateHandle);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void AApplyGameplayEffectZone::OnCharacterPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	// Don't do anything if we didn't get the new valid PlayerState
	if (!IsValid(NewPlayerState))
	{
		return;
	}

	// Try to get the ASC from a valid PlayerState this time
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(
		NewPlayerState);

	// If we found an ASC, then apply the gameplay effect to it
	if (IsValid(AbilitySystemComponent))
	{
		ApplyGameplayEffectChecked(AbilitySystemComponent);
	}

	/**
	 * The character now has a valid PlayerState, so we don't need to listen for the PlayerState changes for this
	 * character anymore. We can be sure the character is valid here because this function is called by the delegate
	 * that was broadcast from this character.
	 */
	OnPlayerStateChangedDelegateHandles.Remove(
		CastChecked<AEscapeChroniclesCharacter>(NewPlayerState->GetPawn()));
}

void AApplyGameplayEffectZone::ApplyGameplayEffectChecked(UAbilitySystemComponent* AbilitySystemComponent)
{
	// Apply gameplay effect
	const FActiveGameplayEffectHandle ActiveGameplayEffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(
		GameplayEffectClass->GetDefaultObject<UGameplayEffect>(), 1, AbilitySystemComponent->MakeEffectContext(),
		AbilitySystemComponent->GetPredictionKeyForNewAction());

	// If we successfully applied the gameplay effect and got a valid handle, then remember it
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

	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	/**
	 * If an actor that stopped overlapping the zone is a character, then MAYBE we subscribed to its PlayerState changes
	 * when we failed to find ASC from this character, so now we need to unsubscribe from it.
	 */
	if (IsValid(Character))
	{
		const FDelegateHandle* OnPlayerStateChangedDelegateHandle = OnPlayerStateChangedDelegateHandles.Find(
			Character);

		// Check if we actually subscribed to the delegate
		if (OnPlayerStateChangedDelegateHandle)
		{
			// Unsubscribe from the delegate
			Character->OnPlayerStateChangedDelegate.Remove(*OnPlayerStateChangedDelegateHandle);

			// Forget about the delegate handle for this character
			OnPlayerStateChangedDelegateHandles.Remove(Character);
		}
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
	 * Remove gameplay effect from the actor, but first check if we are the server because removing gameplay effects
	 * predictively on clients is not supported 🤡.
	 */
	if (HasAuthority())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(*ActiveGameplayEffectHandle);
	}

	// Forget about a gameplay effect handle
	ActorsWithActiveGameplayEffects.Remove(AbilitySystemComponent);

	// Unload the gameplay effect class if there are no actors with active gameplay effects anymore
	if (ActorsWithActiveGameplayEffects.IsEmpty() && LoadGameplayEffectClassHandle.IsValid())
	{
		LoadGameplayEffectClassHandle->ReleaseHandle();
		LoadGameplayEffectClassHandle.Reset();
	}
}