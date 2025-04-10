// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GameplayAbilityWithGameplayEffects.h"

#include "AbilitySystemComponent.h"
#include "Engine/AssetManager.h"

void UGameplayAbilityWithGameplayEffects::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}

	TArray<FSoftObjectPath> GameplayEffectsToLoad;
	GameplayEffectsToLoad.Reserve(GameplayEffectsToApplyWhileActive.Num());

	for (const TSoftClassPtr<UGameplayEffect>& GameplayEffect : GameplayEffectsToApplyWhileActive)
	{
		if (ensureAlways(!GameplayEffect.IsNull()))
		{
			GameplayEffectsToLoad.Add(GameplayEffect.ToSoftObjectPath());
		}
	}

	if (GameplayEffectsToApplyWhileActive.Num() > 0)
	{
		UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(GameplayEffectsToLoad,
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameplayEffectsLoaded));
	}
}

void UGameplayAbilityWithGameplayEffects::OnGameplayEffectsLoaded()
{
	for (const TSoftClassPtr<UGameplayEffect>& GameplayEffect : GameplayEffectsToApplyWhileActive)
	{
		FActiveGameplayEffectHandle AppliedGameplayEffect = ApplyGameplayEffectToOwner(CurrentSpecHandle,
			CurrentActorInfo, CurrentActivationInfo, GameplayEffect->GetDefaultObject<UGameplayEffect>(),
			GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));

		AppliedGameplayEffects.Add(AppliedGameplayEffect);
	}
}

void UGameplayAbilityWithGameplayEffects::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	// Removing gameplay effects is not allowed to be predicted on the client
	if (HasAuthority(&ActivationInfo))
	{
		for (const FActiveGameplayEffectHandle& AppliedGameplayEffect : AppliedGameplayEffects)
		{
			ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(AppliedGameplayEffect);
		}
	}

	AppliedGameplayEffects.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}