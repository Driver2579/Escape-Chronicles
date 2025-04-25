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
	virtual void OnPreSaveObject() override;
	virtual void OnPostLoadObject() override;

private:
	/**
	 * Cached associations between input tags and abilities.
	 * @tparam FGameplayTag Input tag.
	 * @tparam FGameplayAbilitySpecHandle Abilities associated with the input tag.
	 */
	TMultiMap<FGameplayTag, FGameplayAbilitySpecHandle> GameplayAbilitiesAssociatedWithInputTags;

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