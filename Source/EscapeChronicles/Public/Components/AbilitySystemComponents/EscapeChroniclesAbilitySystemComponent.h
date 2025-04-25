// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Interfaces/Saveable.h"
#include "Common/Structs/SaveData/AttributeSetSaveData.h"
#include "EscapeChroniclesAbilitySystemComponent.generated.h"

struct FInputActionValue;

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesAbilitySystemComponent : public UAbilitySystemComponent, public ISaveable
{
	GENERATED_BODY()

public:
	// Generates a unique input ID for new ability
	int32 GenerateInputId();

	/**
	 * Registers an input tag to be associated with the specified ability in AbilitySystemSet. An associated ability
	 * will be found automatically if it was set in AbilitySystemSet.
	 * 
	 * Warning: This MUST be called for each input tag that should trigger an ability when the input action is bound.
	 * Otherwise, any functions related to abilities pressing/releasing by input tags are not going to work.
	 */
	void RegisterInputTag(const FGameplayTag& InputTag);

	/**
	 * Same as PressInputID, but uses input tag instead of input ID.
	 *
	 * Warning: To make this function work, you MUST call RegisterInputTag first.
	 */
	void PressInputTag(const FGameplayTag& InputTag);

	/**
	 * Same as ReleaseInputID, but uses input tag instead of input ID.
	 * 
	 * Warning: To make this function work, you MUST call RegisterInputTag first.
	 */
	void ReleaseInputTag(const FGameplayTag& InputTag);

	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;

	/**
	 * Doesn't allow certain attributes to be modified by certain roles. Should be used by attribute sets before
	 * modifying an attribute.
	 */
	bool CanModifyAttribute(const FGameplayAttribute& Attribute) const;

	// Adds the specified tag and attribute to the BlockedAttributesByTags map
	void ApplyBlockingAttributeWhenHasTag(const FGameplayTag& RequiredTagToBlockAttribute,
		const FGameplayAttribute& AttributeToBlock)
	{
		BlockedAttributesByTags.AddUnique(RequiredTagToBlockAttribute, AttributeToBlock);
	}

	// Removes the specified tag and attribute from the BlockedAttributesByTags map
	void RemoveBlockingAttributeWhenHasTag(const FGameplayTag& RequiredTagToBlockAttribute,
		const FGameplayAttribute& AttributeToBlock)
	{
		BlockedAttributesByTags.Remove(RequiredTagToBlockAttribute, AttributeToBlock);
	}

protected:
	// Unregisters an input tag associated with an input ID
	void UnregisterInputTag(const FGameplayTag& InputTag)
	{
		InputIDsAssociatedWithInputTags.Remove(InputTag);
	}

	virtual void OnPreSaveObject() override;
	virtual void OnPostLoadObject() override;

private:
	int32 LastGeneratedInputID = -1;

	/**
	 * Associations between input tags and input IDs of abilities.
	 * @tparam FGameplayTag Input tag.
	 * @tparam int32 Input ID.
	 */
	TMap<FGameplayTag, int32> InputIDsAssociatedWithInputTags;

	/**
	 * When this ability system component has a tag from this map, attributes associated with this tag will not be
	 * allowed to be modified.
	 * @tparam FGameplayTag Tag that is associated with attributes.
	 * @tparam FGameplayAttribute Attributes that should be blocked from modification when the owner has an associated
	 * tag.
	 */
	TMultiMap<FGameplayTag, FGameplayAttribute> BlockedAttributesByTags;

	// Map of attribute sets that are saved in the save game object
	UPROPERTY(SaveGame)
	TMap<TSoftClassPtr<UAttributeSet>, FAttributeSetSaveData> SavedAttributeSets;
};