// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/DataAssets/AbilitySystemSet.h"

#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"

void UAbilitySystemSet::GiveToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent,
	UObject* SourceObject)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to give or take ability sets
	if (!AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return;
	}

	GiveAttributesToAbilitySystem_Internal(AbilitySystemComponent);
	GiveAbilitiesToAbilitySystem_Internal(AbilitySystemComponent, SourceObject);
	GiveEffectsToAbilitySystem_Internal(AbilitySystemComponent);
	ApplyBlockingAttributesByTags_Internal(AbilitySystemComponent);
}

void UAbilitySystemSet::TakeFromAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to give or take ability sets
	if (!AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return;
	}

	RemoveBlockingAttributesByTags_Internal(AbilitySystemComponent);
	TakeEffectsFromAbilitySystem_Internal(AbilitySystemComponent);
	TakeAbilitiesFromAbilitySystem_Internal(AbilitySystemComponent);
	TakeAttributesFromAbilitySystem_Internal(AbilitySystemComponent);
}

void UAbilitySystemSet::GiveAttributesToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to give or take ability sets
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		GiveAttributesToAbilitySystem_Internal(AbilitySystemComponent);
	}
}

void UAbilitySystemSet::GiveAttributesToAbilitySystem_Internal(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	for (int32 SetIndex = 0; SetIndex < AttributeSetsToGrant.Num(); ++SetIndex)
	{
		const FAbilitySystemSet_AttributeSet& SetToGrant = AttributeSetsToGrant[SetIndex];

		if (!ensureAlways(IsValid(SetToGrant.AttributeSet)))
		{
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwner(),
			SetToGrant.AttributeSet);

		AbilitySystemComponent->AddAttributeSetSubobject(NewSet);
		GrantedAttributeSets.Add(NewSet);
	}
}

void UAbilitySystemSet::TakeAttributesFromAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to give or take ability sets
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		TakeAttributesFromAbilitySystem_Internal(AbilitySystemComponent);
	}
}

void UAbilitySystemSet::TakeAttributesFromAbilitySystem_Internal(
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	for (int32 SetIndex = GrantedAttributeSets.Num() - 1; SetIndex >= 0; --SetIndex)
	{
		TWeakObjectPtr<UAttributeSet> SetToRemove = GrantedAttributeSets[SetIndex];

		if (SetToRemove.IsValid())
		{
			AbilitySystemComponent->RemoveSpawnedAttribute(SetToRemove.Get());
		}
	}

	GrantedAttributeSets.Empty();
}

void UAbilitySystemSet::GiveAbilitiesToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent,
	UObject* SourceObject)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to give or take ability sets
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		GiveAbilitiesToAbilitySystem_Internal(AbilitySystemComponent, SourceObject);
	}
}

void UAbilitySystemSet::GiveAbilitiesToAbilitySystem_Internal(
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent, UObject* SourceObject)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	for (int32 AbilityIndex = 0; AbilityIndex < GameplayAbilitiesToGrant.Num(); ++AbilityIndex)
	{
		const FAbilitySystemSet_GameplayAbility& AbilityToGrant = GameplayAbilitiesToGrant[AbilityIndex];

		if (!ensureAlways(IsValid(AbilityToGrant.Ability)))
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityToGrant.Ability, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;

		const bool bInputTagValid = AbilityToGrant.InputTag.IsValid();

		// Add input tag and input ID only if the input tag is valid
		if (bInputTagValid)
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);
			AbilitySpec.InputID = AbilitySystemComponent->GenerateInputId();
		}

		FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);

		if (ensureAlways(AbilitySpecHandle.IsValid()))
		{
			GrantedGameplayAbilities.Add(AbilitySpecHandle);
		}
	}
}

void UAbilitySystemSet::TakeAbilitiesFromAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to give or take ability sets
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		TakeAbilitiesFromAbilitySystem_Internal(AbilitySystemComponent);
	}
}

void UAbilitySystemSet::TakeAbilitiesFromAbilitySystem_Internal(
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	for (int32 AbilityIndex = GrantedGameplayAbilities.Num() - 1; AbilityIndex >= 0; --AbilityIndex)
	{
		const FGameplayAbilitySpecHandle& AbilityToRemove = GrantedGameplayAbilities[AbilityIndex];

		if (AbilityToRemove.IsValid())
		{
			AbilitySystemComponent->ClearAbility(AbilityToRemove);
		}
	}

	GrantedGameplayAbilities.Empty();
}

void UAbilitySystemSet::GiveEffectsToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to give or take ability sets
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		GiveEffectsToAbilitySystem_Internal(AbilitySystemComponent);
	}
}

void UAbilitySystemSet::GiveEffectsToAbilitySystem_Internal(
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	for (int32 EffectIndex = 0; EffectIndex < GameplayEffectsToGrant.Num(); ++EffectIndex)
	{
		const FAbilitySystemSet_GameplayEffect& EffectToGrant = GameplayEffectsToGrant[EffectIndex];

		if (!ensureAlways(IsValid(EffectToGrant.GameplayEffect)))
		{
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect.GetDefaultObject();

		FActiveGameplayEffectHandle EffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffect,
			EffectToGrant.EffectLevel, AbilitySystemComponent->MakeEffectContext());

		if (EffectHandle.IsValid())
		{
			GrantedGameplayEffects.Add(EffectHandle);
		}
	}
}

void UAbilitySystemSet::TakeEffectsFromAbilitySystem(
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to give or take ability sets
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		TakeEffectsFromAbilitySystem_Internal(AbilitySystemComponent);
	}
}

void UAbilitySystemSet::TakeEffectsFromAbilitySystem_Internal(
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	for (int32 EffectIndex = GrantedGameplayEffects.Num() - 1; EffectIndex >= 0; --EffectIndex)
	{
		const FActiveGameplayEffectHandle& EffectToRemove = GrantedGameplayEffects[EffectIndex];

		if (EffectToRemove.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(EffectToRemove);
		}
	}

	GrantedGameplayEffects.Empty();
}

void UAbilitySystemSet::ApplyBlockingAttributesByTags(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to apply or remove blocking attributes by tags
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		ApplyBlockingAttributesByTags_Internal(AbilitySystemComponent);
	}
}

void UAbilitySystemSet::ApplyBlockingAttributesByTags_Internal(
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	for (const auto& BlockedAttributesPair : AttributesToBlockByTags)
	{
		if (!ensureAlways(BlockedAttributesPair.Key.IsValid()))
		{
			continue;
		}

		for (const FGameplayAttribute& BlockedAttribute : BlockedAttributesPair.Value.Attributes)
		{
			if (ensureAlways(BlockedAttribute.IsValid()))
			{
				AbilitySystemComponent->ApplyBlockingAttributeWhenHasTag(BlockedAttributesPair.Key, BlockedAttribute);
			}
		}
	}
}

void UAbilitySystemSet::RemoveBlockingAttributesByTags(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Must be authoritative to apply or remove blocking attributes by tags
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		RemoveBlockingAttributesByTags_Internal(AbilitySystemComponent);
	}
}

void UAbilitySystemSet::RemoveBlockingAttributesByTags_Internal(
	UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	for (const auto& BlockedAttributesPair : AttributesToBlockByTags)
	{
		if (!ensureAlways(BlockedAttributesPair.Key.IsValid()))
		{
			continue;
		}

		for (const FGameplayAttribute& BlockedAttribute : BlockedAttributesPair.Value.Attributes)
		{
			if (ensureAlways(BlockedAttribute.IsValid()))
			{
				AbilitySystemComponent->RemoveBlockingAttributeWhenHasTag(BlockedAttributesPair.Key, BlockedAttribute);
			}
		}
	}
}