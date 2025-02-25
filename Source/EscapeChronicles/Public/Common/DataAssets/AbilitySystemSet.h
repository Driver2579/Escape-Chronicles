// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemSet.generated.h"

class UEscapeChroniclesAbilitySystemComponent;
class UEscapeChroniclesGameplayAbility;
class UGameplayEffect;
class UAttributeSet;

struct FGameplayAbilitySpecHandle;
struct FActiveGameplayEffectHandle;

// Data used by the ability set to grant gameplay abilities
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

// Data used by the ability set to grant gameplay effects
USTRUCT(BlueprintType)
struct FAbilitySystemSet_GameplayEffect
{
	GENERATED_BODY()

	// Gameplay effect to grant
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UEscapeChroniclesGameplayAbility> GameplayEffect;

	// Level of gameplay effect to grant
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

// Data used by the ability set to grant attribute sets
USTRUCT(BlueprintType)
struct FAbilitySystemSet_AttributeSet
{
	GENERATED_BODY()

	// Gameplay effect to grant
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;
};

// Non-mutable data asset used to grant gameplay abilities and gameplay effects
UCLASS(BlueprintType, Const)
class ESCAPECHRONICLES_API UAbilitySystemSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Grants the ability set to the specified ability system component.
	 * @param AbilitySystemComponent Ability system component to grant the abilities to.
	 * @param SourceObject Optional source object to add to the abilities.
	 */
	void GiveToAbilitySystem(UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent,
		UObject* SourceObject = nullptr) const;

	// Gameplay abilities to grant when this ability set is granted
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Abilities", meta=(TitleProperty="Ability"))
	TArray<FAbilitySystemSet_GameplayAbility> GrantedGameplayAbilities;

	// Gameplay effects to grant when this ability set is granted
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects", meta=(TitleProperty="GameplayEffect"))
	TArray<FAbilitySystemSet_GameplayEffect> GrantedGameplayEffects;

	// Attribute sets to grant when this ability set is granted
	UPROPERTY(EditDefaultsOnly, Category="Attribute Sets", meta=(TitleProperty="AttributeSet"))
	TArray<FAbilitySystemSet_AttributeSet> GrantedAttributes;
};