// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/AbilityComponents/ApplyGameplayEffectsGameplayAbilityComponent.h"

#include "AbilitySystemComponent.h"
#include "Engine/AssetManager.h"

void UApplyGameplayEffectsGameplayAbilityComponent::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

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

void UApplyGameplayEffectsGameplayAbilityComponent::OnGameplayEffectsLoaded()
{
	UEscapeChroniclesGameplayAbility* OwningAbility = GetOwner();

#if DO_CHECK
	check(IsValid(OwningAbility));
#endif

	const FGameplayAbilitySpecHandle CurrentSpecHandle = OwningAbility->GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* CurrentActorInfo = OwningAbility->GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo& CurrentActivationInfo = OwningAbility->GetCurrentActivationInfoRef();

	for (const TSoftClassPtr<UGameplayEffect>& GameplayEffect : GameplayEffectsToApplyWhileActive)
	{
		FActiveGameplayEffectHandle AppliedGameplayEffect = ApplyGameplayEffectToOwner(CurrentSpecHandle,
			CurrentActorInfo, CurrentActivationInfo, GameplayEffect->GetDefaultObject<UGameplayEffect>(),
			OwningAbility->GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));

		AppliedGameplayEffects.Add(AppliedGameplayEffect);
	}
}

void UApplyGameplayEffectsGameplayAbilityComponent::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	const UEscapeChroniclesGameplayAbility* OwningAbility = GetOwner();

#if DO_CHECK
	check(IsValid(OwningAbility));
#endif

	// Removing gameplay effects is not allowed to be predicted on the client
	if (OwningAbility->HasAuthority(&ActivationInfo))
	{
		for (const FActiveGameplayEffectHandle& AppliedGameplayEffect : AppliedGameplayEffects)
		{
			ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(AppliedGameplayEffect);
		}
	}

	AppliedGameplayEffects.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}