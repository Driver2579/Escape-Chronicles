// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "InputActionValue.h"
#include "EscapeChroniclesAbilitySystemComponent.generated.h"

struct FInputActionValue;

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/**
	 * Registers an input tag to be associated with the specified ability in AbilitySystemSet. An associated ability
	 * will be found automatically if it was set in AbilitySystemSet. This is needed to not iterate all abilities every
	 * time we want to find it by input tag and search only for the registered ones in a more optimized way via
	 * TMultiMap.\n
	 * Warning: This MUST be called for each input tag that should trigger an ability when the input action is bound.
	 * Otherwise, any functions related to input tags and abilities activation/ending are not going to work.
	 */
	void RegisterInputTag(const FGameplayTag& InputTag);

	/**
	 * Activates all abilities associated with the input tag. These associations should be set in AbilitySystemSet.\n
	 * Warning: To make this function work, you MUST call RegisterInputTag first.
	 */
	void TryActivateAbilitiesByInputTag(const FGameplayTag& InputTag);

	/**
	 * Ends all abilities associated with the input tag. These associations should be set in AbilitySystemSet.\n
	 * Warning: To make this function work, you MUST call RegisterInputTag first.
	 */
	void TryEndAbilitiesByInputTag(const FGameplayTag& InputTag) const;

private:
	/**
	 * Cached associations between input tags and abilities.
	 * @tparam FGameplayTag Input tag.
	 * @tparam FGameplayAbilitySpecHandle Abilities associated with the input tag.
	 */
	TMultiMap<FGameplayTag, FGameplayAbilitySpecHandle> GameplayAbilitiesAssociatedWithInputTags;
};