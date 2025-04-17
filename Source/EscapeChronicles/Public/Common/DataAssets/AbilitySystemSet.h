// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemSet.generated.h"

class UEscapeChroniclesAbilitySystemComponent;
class UEscapeChroniclesGameplayAbility;

// Data used by the ability system set to grant gameplay abilities
USTRUCT(BlueprintType)
struct FAbilitySystemSet_GameplayAbility
{
	GENERATED_BODY()

	// Gameplay ability to grant
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UEscapeChroniclesGameplayAbility> Ability;

	// Level of ability to grant
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability
	UPROPERTY(EditDefaultsOnly, meta=(Categories="InputTag"))
	FGameplayTag InputTag;
};

// Data used by the ability system set to grant gameplay effects
USTRUCT(BlueprintType)
struct FAbilitySystemSet_GameplayEffect
{
	GENERATED_BODY()

	// Gameplay effect to grant
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect;

	// Level of gameplay effect to grant
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

// Data used by the ability system set to grant attribute sets
USTRUCT(BlueprintType)
struct FAbilitySystemSet_AttributeSet
{
	GENERATED_BODY()

	// Attribute set to grant
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;
};

// Data used by the ability system set to block attributes from being modified
USTRUCT(BlueprintType)
struct FAbilitySystemSet_BlockedAttributes
{
	GENERATED_BODY()

	// Attributes to block
	UPROPERTY(EditDefaultsOnly)
	TArray<FGameplayAttribute> Attributes;
};

// Non-mutable data asset used to grant gameplay abilities and gameplay effects
UCLASS(BlueprintType, Const)
class ESCAPECHRONICLES_API UAbilitySystemSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Grants the ability system set to the specified ability system component.
	 * @param AbilitySystemComponent Ability system component to grant the set to.
	 * @param SourceObject Optional source object to add to the abilities.
	 */
	void GiveToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent,
		UObject* SourceObject = nullptr);

	/**
	 * Takes the ability system set from the specified ability system component.
	 * @param AbilitySystemComponent Ability system component to take the set from.
	 */
	void TakeFromAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Grants only attributes of this ability system set to the specified ability system component.
	 * @param AbilitySystemComponent Ability system component to grant the attributes to.
	 */
	void GiveAttributesToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Takes only attributes of this ability system set from the specified ability system component.
	 * @param AbilitySystemComponent Ability system component to take the attributes from.
	 */
	void TakeAttributesFromAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Grants only abilities of this ability system set to the specified ability system component.
	 * @param AbilitySystemComponent Ability system component to grant the abilities to.
	 * @param SourceObject Optional source object to add to the abilities.
	 */
	void GiveAbilitiesToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent,
		UObject* SourceObject = nullptr);

	/**
	 * Takes only abilities of this ability system set from the specified ability system component.
	 * @param AbilitySystemComponent Ability system component to take the attributes from.
	 */
	void TakeAbilitiesFromAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Grants only effects of this ability system set to the specified ability system component.
	 * @param AbilitySystemComponent Ability system component to apply the effects to.
	 */
	void GiveEffectsToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Takes only effects of this ability system set from the specified ability system component.
	 * @param AbilitySystemComponent Ability system component to remove the effects from.
	 */
	void TakeEffectsFromAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Only blocks attributes from being modified by gameplay effects if the owner has a tag that is associated with an
	 * attribute that wants to be modified of this ability system set.
	 * @param AbilitySystemComponent Ability system component to apply the blocking attributes to.
	 */
	void ApplyBlockingAttributesByTags(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Only stops blocking attributes from being modified by gameplay effects if the owner has a tag that is associated
	 * with an attribute that wants to be modified of this ability system set.
	 * @param AbilitySystemComponent Ability system component to remove the blocking attributes from.
	 */
	void RemoveBlockingAttributesByTags(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

private:
	// Attribute sets to grant when this ability system set is granted
	UPROPERTY(EditDefaultsOnly, Category="Attribute Sets", meta=(TitleProperty="Attribute Set"))
	TArray<FAbilitySystemSet_AttributeSet> AttributeSetsToGrant;

	// Gameplay abilities to grant when this ability system set is granted
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Abilities", meta=(TitleProperty="Gameplay Ability"))
	TArray<FAbilitySystemSet_GameplayAbility> GameplayAbilitiesToGrant;

	// Gameplay effects to grant when this ability system set is granted
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects", meta=(TitleProperty="Gameplay Effect"))
	TArray<FAbilitySystemSet_GameplayEffect> GameplayEffectsToGrant;

	/**
	 * Attributes in this map will not be allowed to be modified by gameplay effects when this ability system set is
	 * granted if an owner has an associated tag with attributes in this map.
	 * @remark Gameplay effects from this ability system set can modify any attributes even if they are in this map.
	 * Other ability system sets also can modify these attributes if they are applied at the same time with this one.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Attribute Sets", meta=(TitleProperty="Blocked Attributes"))
	TMap<FGameplayTag, FAbilitySystemSet_BlockedAttributes> AttributesToBlockByTags;

	/**
	 * Actual implementation of the GiveAttributesToAbilitySystem function. Doesn't check if an owner actor is
	 * authoritative.
	 */
	void GiveAttributesToAbilitySystem_Internal(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Actual implementation of the TakeAttributesFromAbilitySystem function. Doesn't check if an owner actor is
	 * authoritative.
	 */
	void TakeAttributesFromAbilitySystem_Internal(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Actual implementation of the GiveAbilitiesToAbilitySystem function. Doesn't check if an owner actor is
	 * authoritative.
	 */
	void GiveAbilitiesToAbilitySystem_Internal(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent,
		UObject* SourceObject);

	/**
	 * Actual implementation of the TakeAbilitiesFromAbilitySystem function. Doesn't check if an owner actor is
	 * authoritative.
	 */
	void TakeAbilitiesFromAbilitySystem_Internal(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Actual implementation of the GiveEffectsToAbilitySystem function. Doesn't check if an owner actor is
	 * authoritative.
	 */
	void GiveEffectsToAbilitySystem_Internal(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Actual implementation of the TakeEffectsFromAbilitySystem function. Doesn't check if an owner actor is
	 * authoritative.
	 */
	void TakeEffectsFromAbilitySystem_Internal(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Actual implementation of the ApplyBlockingAttributesByTags function. Doesn't check if an owner actor is
	 * authoritative.
	 */
	void ApplyBlockingAttributesByTags_Internal(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Actual implementation of the RemoveBlockingAttributesByTags function. Doesn't check if an owner actor is
	 * authoritative.
	 */
	void RemoveBlockingAttributesByTags_Internal(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent);

	TArray<TWeakObjectPtr<UAttributeSet>> GrantedAttributeSets;
	TArray<FGameplayAbilitySpecHandle> GrantedGameplayAbilities;
	TArray<FActiveGameplayEffectHandle> GrantedGameplayEffects;
};