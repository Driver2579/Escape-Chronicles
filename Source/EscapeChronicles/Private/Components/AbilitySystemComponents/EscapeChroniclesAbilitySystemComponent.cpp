// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"

#include "EscapeChroniclesGameplayTags.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "Common/Structs/SaveData/AttributeSetSaveData.h"

int32 UEscapeChroniclesAbilitySystemComponent::GenerateInputId()
{
	// Initially it's set to -1, so the first one will be 0, the second one will be 1, etc.
	++LastGeneratedInputID;

	return LastGeneratedInputID;
}

void UEscapeChroniclesAbilitySystemComponent::RegisterInputTag(const FGameplayTag& InputTag)
{
	// Iterate all activatable abilities to find those that have the specified input tag
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		/**
		 * Input tags are added to DynamicSpecSourceTags in AbilitySystemSet when the ability is granted. We should also
		 * check if ability has an input ID, because otherwise there is nothing to associate.
		 */
		if (Spec.InputID != INDEX_NONE && Spec.GetDynamicSpecSourceTags().HasTag(InputTag))
		{
			// Remember the ability's input ID for the input tag
			InputIDsAssociatedWithInputTags.FindOrAdd(InputTag, Spec.InputID);
		}
	}
}

void UEscapeChroniclesAbilitySystemComponent::PressInputTag(const FGameplayTag& InputTag)
{
	const int32* InputID = InputIDsAssociatedWithInputTags.Find(InputTag);

	if (InputID)
	{
		PressInputID(*InputID);
	}
}

void UEscapeChroniclesAbilitySystemComponent::ReleaseInputTag(const FGameplayTag& InputTag)
{
	const int32* InputID = InputIDsAssociatedWithInputTags.Find(InputTag);

	if (InputID)
	{
		ReleaseInputID(*InputID);
	}
}

void UEscapeChroniclesAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	// Try to find other abilities with the same input ID
	TArray<FGameplayAbilitySpecHandle> AbilitiesWithSameInputID;
	FindAllAbilitiesWithInputID(AbilitiesWithSameInputID, AbilitySpec.InputID);

	// If this is the last ability with this input ID, unregister the input tag associated with that ID
	if (AbilitiesWithSameInputID.Num() <= 1)
	{
		const FGameplayTag* InputTag = InputIDsAssociatedWithInputTags.FindKey(AbilitySpec.InputID);

		// Could be null in case it wasn't registered at all
		if (InputTag)
		{
			InputIDsAssociatedWithInputTags.Remove(*InputTag);
		}
	}

	Super::OnRemoveAbility(AbilitySpec);
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