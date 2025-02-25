// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/DataAssets/AbilitySystemSet.h"

#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"

void UAbilitySystemSet::GiveToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent,
	UObject* SourceObject) const
{
#if DO_CHECK
	check(AbilitySystemComponent);
#endif

	if (!AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets
		return;
	}

	// Grant the attribute sets
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FAbilitySystemSet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!ensureAlways(IsValid(SetToGrant.AttributeSet)))
		{
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwner(),
			SetToGrant.AttributeSet);

		AbilitySystemComponent->AddAttributeSetSubobject(NewSet);
	}

	// Grant the gameplay abilities
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FAbilitySystemSet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!ensureAlways(IsValid(AbilityToGrant.Ability)))
		{
			continue;
		}

		UGameplayAbility* AbilityCDO = AbilityToGrant.Ability.GetDefaultObject();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);

		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}

	// Grant the gameplay effects
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FAbilitySystemSet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!ensureAlways(IsValid(EffectToGrant.GameplayEffect)))
		{
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();

		AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel,
			AbilitySystemComponent->MakeEffectContext());
	}
}
