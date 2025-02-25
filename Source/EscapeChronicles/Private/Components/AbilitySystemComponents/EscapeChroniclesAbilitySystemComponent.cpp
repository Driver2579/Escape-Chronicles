// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"

void UEscapeChroniclesAbilitySystemComponent::RegisterInputTag(const FGameplayTag& InputTag)
{
	// Iterate all activatable abilities to find those that have the specified input tag
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		// Input tags are added to DynamicSpecSourceTags in AbilitySystemSet when the ability is granted
		if (Spec.GetDynamicSpecSourceTags().HasTag(InputTag))
		{
			// Remember the ability's handle for the input tag
			GameplayAbilitiesAssociatedWithInputTags.Add(InputTag, Spec.Handle);
		}
	}
}

void UEscapeChroniclesAbilitySystemComponent::TryActivateAbilitiesByInputTag(const FGameplayTag& InputTag,
	const FInputActionValue& InputActionValue)
{
	// Find all abilities associated with the input tag
	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	GameplayAbilitiesAssociatedWithInputTags.MultiFind(InputTag, AbilitiesToActivate);

	for (const FGameplayAbilitySpecHandle AbilityToActivate : AbilitiesToActivate)
	{
		const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilityToActivate);

		// Activate the found ability
		if (ensureAlways(AbilitySpec))
		{
			LastInputActionValue = InputActionValue;
			TryActivateAbility(AbilityToActivate);
		}
	}
}

void UEscapeChroniclesAbilitySystemComponent::TryEndAbilitiesByInputTag(const FGameplayTag& InputTag) const
{
	// Find all abilities associated with the input tag
	TArray<FGameplayAbilitySpecHandle> AbilitiesToEnd;
	GameplayAbilitiesAssociatedWithInputTags.MultiFind(InputTag, AbilitiesToEnd);

	for (const FGameplayAbilitySpecHandle AbilityToEnd : AbilitiesToEnd)
	{
		const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilityToEnd);

		if (!ensureAlways(AbilitySpec))
		{
			continue;
		}

		/**
		 * Iterate all ability instances of found AbilitySpec to end them all. Usually, there is only once instance
		 * because we use InstancedPerActor abilities, but it's better to follow the interface to be safe.
		 */
		for (UGameplayAbility* Ability : AbilitySpec->GetAbilityInstances())
		{
			UEscapeChroniclesGameplayAbility* EscapeChroniclesAbilityInstance = Cast<UEscapeChroniclesGameplayAbility>(
				Ability);

			// End found ability
			if (ensureAlways(IsValid(EscapeChroniclesAbilityInstance)))
			{
				EscapeChroniclesAbilityInstance->EndAbility(AbilityToEnd, Ability->GetCurrentActorInfo(),
					Ability->GetCurrentActivationInfo(), true, false);
			}
		}
	}
}

void UEscapeChroniclesAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	UEscapeChroniclesGameplayAbility* EscapeChroniclesAbilityInstance = Cast<UEscapeChroniclesGameplayAbility>(Ability);

	// Store the input action value in the ability instance before it gets overriden by the next input action
	// TODO: This is bullshit. InputActionValue changes every tick.
	if (ensureAlways(IsValid(EscapeChroniclesAbilityInstance)))
	{
		EscapeChroniclesAbilityInstance->SetInputActionValue(LastInputActionValue);
	}
}