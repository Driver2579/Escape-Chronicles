// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"

#include "EscapeChroniclesGameplayTags.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "Common/Structs/SaveData/AttributeSetSaveData.h"

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

void UEscapeChroniclesAbilitySystemComponent::TryActivateAbilitiesByInputTag(const FGameplayTag& InputTag)
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

bool UEscapeChroniclesAbilitySystemComponent::CanModifyAttribute(const FGameplayAttribute& Attribute) const
{
	const FGameplayTag* AssociatedTag = BlockedAttributesByTags.FindKey(Attribute);

	if (AssociatedTag && HasMatchingGameplayTag(*AssociatedTag))
	{
		return false;
	}

	return true;
}

void UEscapeChroniclesAbilitySystemComponent::OnPreSaveObject()
{
	// Clear the map to avoid conflicts with the previous saved/loaded data
	SavedAttributeSets.Empty();

	// Iterate all attribute sets to save their attributes
	for (const UAttributeSet* AttributeSet : GetSpawnedAttributes())
	{
#if DO_CHECK
		check(IsValid(AttributeSet));
#endif

		FAttributeSetSaveData AttributeSetSaveData;

		TArray<FGameplayAttribute> Attributes;
		UAttributeSet::GetAttributesFromSetClass(AttributeSet->GetClass(), Attributes);

		// Iterate all attributes to save their base values
		for (const FGameplayAttribute& Attribute : Attributes)
		{
			// Save information about what attribute is being saved and save its base value
			AttributeSetSaveData.AttributesBaseValues.Add(Attribute,
				Attribute.GetGameplayAttributeDataChecked(AttributeSet)->GetBaseValue());
		}

		// Save information about what attribute set is being saved and save its attributes' base values
		SavedAttributeSets.Add(AttributeSet->GetClass(), AttributeSetSaveData);
	}
}

void UEscapeChroniclesAbilitySystemComponent::OnPostLoadObject()
{
	// Iterate all attribute sets to update their attributes with loaded data
	for (UAttributeSet* AttributeSet : GetSpawnedAttributes())
	{
#if DO_CHECK
		check(IsValid(AttributeSet));
#endif

		// Try to find the save data for the attribute set
		FAttributeSetSaveData* AttributeSetSaveData = SavedAttributeSets.Find(AttributeSet->GetClass());

		// Skip the attribute set if it doesn't have any save data
		if (!AttributeSetSaveData)
		{
			continue;
		}

		TArray<FGameplayAttribute> Attributes;
		UAttributeSet::GetAttributesFromSetClass(AttributeSet->GetClass(), Attributes);

		for (FGameplayAttribute& Attribute : Attributes)
		{
			const float* LoadedBaseValue = AttributeSetSaveData->AttributesBaseValues.Find(Attribute);

			// Set the loaded base value to the attribute if it was found
			if (LoadedBaseValue)
			{
				Attribute.GetGameplayAttributeDataChecked(AttributeSet)->SetBaseValue(*LoadedBaseValue);
			}
		}
	}
}