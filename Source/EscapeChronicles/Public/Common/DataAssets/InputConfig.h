// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

// Non-mutable data asset that contains input configuration properties
UCLASS()
class ESCAPECHRONICLES_API UInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	const auto& GetInputMappingContexts() const { return InputMappingContexts; }

	const auto& GetNativeInputActions() const { return NativeInputActions; }
	const auto& GetAbilityInputActions() const { return AbilityInputActions; }

private:
	/**
	 * List of input mapping contexts used by the owner. These input mapping contexts should contain all input actions
	 * from NativeInputActions and AbilityInputActions.
	 */
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<const UInputMappingContext>> InputMappingContexts;

	/**
	 * List of input actions used by the owner. These input actions are mapped to a gameplay tag and must be manually
	 * bound.
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<TObjectPtr<const UInputAction>, FGameplayTag> NativeInputActions;

	/**
	 * List of input actions used by the owner. These input actions are mapped to a gameplay tag and are automatically
	 * bound to abilities with matching input tags.
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<TObjectPtr<const UInputAction>, FGameplayTag> AbilityInputActions;
};